#include "myno-mqtt.h"
#include "project-conf.h"

static MQTTClient client;
static Network network;

static int topic_cnt = 0;
static char _topic_to_subscribe[MAX_TOPICS][MAX_LEN_TOPIC];

static unsigned char buf[BUF_SIZE];
static unsigned char readbuf[BUF_SIZE];

void myno_mqtt_init(void)
{
    NetworkInit(&network);
    MQTTClientInit(&client, &network, COMMAND_TIMEOUT_MS, buf, BUF_SIZE,
                   readbuf, BUF_SIZE);
    MQTTStartTask(&client);
}

int myno_mqtt_disconnect(void)
{
    topic_cnt = 0;
    int res = MQTTDisconnect(&client);
    if (res < 0) {
        printf("myno_device: Unable to disconnect\n");
    }
    else {
        printf("myno_device: Disconnect successful\n");
    }
    NetworkDisconnect(&network);
    return res;
}

int myno_mqtt_subscribe(const char* topic, enum QoS qos, void(*on_message)(MessageData*))
{
    if (topic_cnt > MAX_TOPICS) {
        printf("myno_device: Already subscribed to max %d topics,",
               topic_cnt);
        return -1;
    }

    if (strlen(topic) > MAX_LEN_TOPIC) {
        printf("myno_device: Not subscribing, topic too long %s\n", topic);
        return -1;
    }

    strncpy(_topic_to_subscribe[topic_cnt], topic, strlen(topic));

    printf("myno_device: Subscribing to %s\n", _topic_to_subscribe[topic_cnt]);
    int ret = MQTTSubscribe(&client,
              _topic_to_subscribe[topic_cnt], qos, on_message);
    if (ret < 0) {
        printf("myno_device: Unable to subscribe to %s (%d)\n",
               _topic_to_subscribe[topic_cnt], ret);
        myno_mqtt_disconnect();
    }
    else {
        printf("myno_device: Now subscribed to %s, QOS %d\n",
               topic, (int) qos);
        topic_cnt++;
    }
    return ret;
}

int myno_mqtt_publish(const char* payload, unsigned int payload_len,
                             const char* payload_prefix, unsigned int prefix_len,
                             char* topic, enum QoS qos,
                             unsigned int block_size)
{
    MQTTMessage message;
    message.qos = qos;
    message.retained = 0;

    int rc = 0;
    unsigned int sent_bytes = 0;
    uint8_t send_buf[block_size];

    if(payload_prefix == NULL) prefix_len = 0;

    while (sent_bytes < payload_len) {
        memcpy(send_buf, payload_prefix, prefix_len);
        unsigned int bytes;
        if (payload_len - sent_bytes + prefix_len > block_size) {
            bytes = block_size;
        }
        else {
            bytes = payload_len - sent_bytes + prefix_len;
        }
        message.payloadlen = bytes;
        bytes -= prefix_len;
        memcpy(send_buf+prefix_len, payload+sent_bytes, bytes);
        message.payload = send_buf;

        if ((rc = MQTTPublish(&client, topic, &message)) < 0) {
            printf("myno_device: Unable to publish (%d)\n", rc);
            return rc;
        }
        else {
            sent_bytes += bytes;
            printf("myno_device: Message has been published to topic %s"
                " with QOS %d (%d/%d bytes)\n", topic, (int)message.qos,
                sent_bytes, payload_len);
        }
    }

    return rc;
}

int myno_mqtt_connect(char *remote_ip, int port)
{
    int ret = -1;

    /* ensure client isn't connected in case of a new connection */
    if (client.isconnected) {
        printf("myno_device: client already connected, disconnecting it\n");
        MQTTDisconnect(&client);
        NetworkDisconnect(&network);
    }

    MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
    data.MQTTVersion = MQTT_VERSION_v5;
    data.clientID.cstring = DEFAULT_MQTT_CLIENT_ID;
    data.username.cstring = DEFAULT_MQTT_USER;
    data.keepAliveInterval = DEFAULT_KEEPALIVE_SEC;
    data.cleansession = 1;
    data.willFlag = 0;

    printf("myno_device: Connecting to MQTT Broker from %s %d\n",
            remote_ip, port);
    printf("myno_device: Trying to connect to %s , port: %d\n",
            remote_ip, port);
    ret = NetworkConnect(&network, remote_ip, port);
    if (ret < 0) {
        printf("myno_device: Unable to connect\n");
        return ret;
    }

    printf("user:%s clientId:%s password:%s\n", data.username.cstring,
             data.clientID.cstring, data.password.cstring);
    ret = MQTTConnect(&client, &data);
    if (ret < 0) {
        printf("myno_device: Unable to connect client %d\n", ret);
        myno_mqtt_disconnect();
        return ret;
    }
    else {
        printf("myno_device: Connect successful\n");
        return 0;
    }
}