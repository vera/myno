/*
 * Copyright (C) 2021 Vera Clemens <clemens@uni-potsdam.de>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "xtimer.h"
#include "shell.h"
#include "thread.h"
#include "net/gnrc/netif.h"
#include "paho_mqtt.h"
#include "MQTTClient.h"
#include "msg.h"
#include "ontology.h"
#include "mq135.h"

#define MAIN_QUEUE_SIZE     (8)
static msg_t _main_msg_queue[MAIN_QUEUE_SIZE];

#define BUF_SIZE                        1024
#define MQTT_VERSION_v311               4       /* MQTT v3.1.1 */
#define COMMAND_TIMEOUT_MS              4000

#ifndef MQTT_CLIENT_ID
#define MQTT_CLIENT_ID          ""
#endif

#ifndef MQTT_USER
#define MQTT_USER               ""
#endif

#ifndef MQTT_PWD
#define MQTT_PWD                ""
#endif

#define MQTT_BLOCK_SIZE         64

/**
 * @brief Default MQTT port
 */
#define MQTT_PORT               1883

/**
 * @brief Keepalive timeout in seconds
 */
#define MQTT_KEEPALIVE_SEC      0

#define IS_CLEAN_SESSION        1
/**
 * @brief Interval in seconds at which sensor readings are published.
 */
#define SENSOR_PUB_INTERVAL     5

static MQTTClient client;
static Network network;

static char _sensor_publish_thread_stack[THREAD_STACKSIZE_DEFAULT + THREAD_EXTRA_STACKSIZE_PRINTF];

static unsigned char buf[BUF_SIZE];
static unsigned char readbuf[BUF_SIZE];

static int _con(char *remote_ip)
{
    int ret = -1;

    /* ensure client isn't connected in case of a new connection */
    if (client.isconnected) {
        printf("myno_co2: client already connected, disconnecting it\n");
        MQTTDisconnect(&client);
        NetworkDisconnect(&network);
    }

    int port = MQTT_PORT;

    MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
    data.MQTTVersion = MQTT_VERSION_v311;
    data.clientID.cstring = MQTT_CLIENT_ID;
    data.username.cstring = MQTT_USER;
    data.password.cstring = MQTT_PWD;
    data.keepAliveInterval = MQTT_KEEPALIVE_SEC;
    data.cleansession = IS_CLEAN_SESSION;
    data.willFlag = 0;

    printf("myno_co2: connecting to MQTT broker at %s:%d\n",
            remote_ip, port);
    ret = NetworkConnect(&network, remote_ip, port);
    if (ret < 0) {
        printf("myno_co2: unable to connect (%d)\n", ret);
        return ret;
    }

    printf("myno_co2: user:%s client ID:%s password:%s\n", data.username.cstring,
             data.clientID.cstring, data.password.cstring);
    ret = MQTTConnect(&client, &data);
    if (ret < 0) {
        printf("myno_co2: unable to connect client %d\n", ret);
        return ret;
    }
    else {
        printf("myno_co2: connection successful\n");
        return 0;
    }
}

static int _pub_block(char *topic, char *msg)
{
    MQTTMessage message;
    message.qos = QOS1;
    message.retained = 0;
    message.payload = msg;
    message.payloadlen = strlen(message.payload);

    int rc;
    if ((rc = MQTTPublish(&client, topic, &message)) < 0) {
        printf("myno_co2: unable to publish (%d)\n", rc);
    }
    else {
        printf("myno_co2: published message to topic %s\n", topic);
    }

    return rc;
}

static int _pub(char *topic, char *msg, int block_size, bool with_device_id)
{
    if (!client.isconnected) {
        printf("myno_co2: broker connection lost\n");
        return -1;
    }

    unsigned int bytes_sent = 0, block_start = 0;
    char block[strlen(MYNO_DEVICE_ID) + 1 + block_size + 1];

    if (with_device_id) {
        /* Prepend "DEVICE_ID;" to each block */
        strncpy(block, MYNO_DEVICE_ID, strlen(MYNO_DEVICE_ID));
        block[strlen(MYNO_DEVICE_ID)] = ';';
        block_start = strlen(MYNO_DEVICE_ID) + 1;
    }

    while (bytes_sent < strlen(msg)) {
        strncpy(block + block_start, msg+bytes_sent, block_size);
        block[block_start + block_size] = '\0';
        int rc = _pub_block(topic, block);
        if (rc < 0) return rc;
        bytes_sent += block_size;
    }
    return 0;
}

static void *_sensor_publish_thread(void *arg)
{
    (void)arg;

    mq135_t dev;

    int res = mq135_init(&dev, MQ135_ADC_LINE);
    if (res != 0) {
        printf("myno_co2: CO2 sensor init failed");
        return NULL;
    }

    while (1) {
        int co2_read = mq135_read_raw(&dev);
        printf("myno_co2: read %d ppm\n", co2_read);
        int len = snprintf(NULL, 0, "%d", co2_read);
        // sanity check
        if (len > 128) {
            printf("myno_co2: CO2 reading is longer than 128 chars, aborting\n");
        }
        char str[len];
        snprintf(str, len + 1, "%d", co2_read);
        _pub(MYNO_CO2_SENSOR_TOPIC, str, len, false);
        xtimer_sleep(SENSOR_PUB_INTERVAL);
    }

    return NULL;
}

/* Adapted from _netif_add */
static int _ip_a_a(char *addr_str, char *prefix_len_str) {
    ipv6_addr_t addr;
    uint16_t flags = GNRC_NETIF_IPV6_ADDRS_FLAGS_STATE_VALID;

    if (ipv6_addr_from_str(&addr, addr_str) == NULL) {
        printf("error: unable to parse IPv6 address.\n");
        return 1;
    }

    netif_t* iface = netif_iter(NULL);
    if (iface == NULL) {
        printf("error: unable to find interface.\n");
        return 1;
    }

    uint8_t prefix_len = atoi(prefix_len_str);
    if (prefix_len == 0) {
        printf("error: unable to parse prefix length.\n");
        return 1;
    }

    flags |= (prefix_len << 8U);
    if (netif_set_opt(iface, NETOPT_IPV6_ADDR, flags, &addr,
                        sizeof(addr)) < 0) {
        printf("error: unable to add IPv6 address\n");
        return 1;
    }

    printf("myno_co2: added %s/%d to interface\n", addr_str, prefix_len);

    return 0;
}

/* Adapted from _nib_route */
static int _ip_r_a_default(char *addr_str) {
    ipv6_addr_t next_hop;

    netif_t* iface = netif_iter(NULL);
    if (iface == NULL) {
        printf("error: unable to find interface.\n");
        return 1;
    }

    if (ipv6_addr_from_str(&next_hop, addr_str) == NULL) {
        printf("error: unable to parse IPv6 address.");
        return 1;
    }

    gnrc_ipv6_nib_ft_add(NULL, 0, &next_hop, netif_get_id(iface), 0);

    printf("myno_co2: added default route\n");

    return 0;
}

int main(void)
{
    xtimer_sleep(3);
    msg_init_queue(_main_msg_queue, MAIN_QUEUE_SIZE);

    NetworkInit(&network);

    MQTTClientInit(&client, &network, COMMAND_TIMEOUT_MS, buf, BUF_SIZE,
                   readbuf,
                   BUF_SIZE);

    /* Assign IP address */
    _ip_a_a(WIRELESS_IP, WIRELESS_PREFIX_LENGTH);

    /* Set default route via border router */
    _ip_r_a_default(BORDER_ROUTER_LL_IP);

    /* Connect to broker */
    do {
        int res = _con(MQTT_BROKER_IP);
        if (res == 0) break;
        printf("myno_co2: retrying broker connection in 15 s...\n");
        xtimer_sleep(15);
    } while(1);

    /* Publish ontology */
    _pub(MYNO_ONTOLOGY_TOPIC, (char *)ontology, MQTT_BLOCK_SIZE, true);
    _pub(MYNO_ONTOLOGY_TOPIC, "END", MQTT_BLOCK_SIZE, true);

    /* Publish CO2 readings in loop (thread) */
    thread_create(_sensor_publish_thread_stack, sizeof(_sensor_publish_thread_stack),
                  THREAD_PRIORITY_MAIN - 1,
                  THREAD_CREATE_STACKTEST,
                  _sensor_publish_thread, NULL, "sensor publish");

    printf("Running myno_co2. Type help for commands info\n");

    MQTTStartTask(&client);

    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(NULL, line_buf, SHELL_DEFAULT_BUFSIZE);
    return 0;
}
