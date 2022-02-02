#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "timex.h"
#include "ztimer.h"
#include "shell.h"
#include "thread.h"
#include "net/gnrc/netif.h"
#include "msg.h"

#include "myno-mqtt.h"
#include "ontology.h"
#include "project-conf.h"
#include "mup.h"

#ifdef MODULE_GNRC
static msg_t _main_msg_queue[MAIN_QUEUE_SIZE];
#endif /* MODULE_GNRC */

static msg_t rpc_message_queue[RPC_MESSAGE_QUEUE_SIZE];

static char shell_thread_stack[THREAD_STACKSIZE_MAIN];

static kernel_pid_t myno_device_pid;


static char* search_buffer(char *haystack, size_t haystacklen, char *needle, size_t needlelen)
{
    int searchlen = haystacklen - needlelen + 1;
    for ( ; searchlen-- > 0; haystack++) {
        if(!memcmp(haystack, needle, needlelen)) {
            return haystack;
        }
    }
    return NULL;
}

static void _on_cmd_received(MessageData *data)
{
    msg_t m;

    if(strncmp(data->topicName->lenstring.data, TOPIC_UPDATE_IMAGE, strlen(TOPIC_UPDATE_IMAGE)) == 0) {
        m.type = MSG_TYPE_IMAGE;
        mup_image_slice_t* slice = malloc(sizeof(rpc_message_t));
        slice->num = atoi(data->message->payload);
        slice->data_len = (int)data->message->payloadlen-(strstr((char *)data->message->payload, ",")-(char*)data->message->payload)-1;
        memcpy(slice->data, strstr((char *)data->message->payload, ",")+1, slice->data_len);
        m.content.ptr = slice;
        int res;
        if((res = msg_send(&m, myno_device_pid)) != 1) {
            printf("paho_mqtt_riot: Sending message failed (%d)\n", res);
        }
    }
    else {
        printf("paho_mqtt_riot: message received on topic %.*s\n",
               (int)data->topicName->lenstring.len,
               data->topicName->lenstring.data);

        char* sep = strstr(data->message->payload, ";");
        if(sep != data->message->payload && sep != NULL) {
            rpc_message_t* rpc = malloc(sizeof(rpc_message_t));
            rpc->reqid = atoi(data->message->payload);
            rpc->num_params = 0;
            int parsed_bytes = 0;

            char* params_sep = strstr(sep+1, ";");

            int rpc_name_len;
            if(params_sep != sep+1 && params_sep != NULL) {
                rpc_name_len = params_sep-sep-1;
                parsed_bytes = (int)(params_sep-(char*)data->message->payload)+1;
            }
            else {
                rpc_name_len = (int)data->message->payloadlen-(sep-(char*)data->message->payload);
                parsed_bytes = (int)data->message->payloadlen;
            }
            if(rpc_name_len >= RPC_NAME_LENGTH) {
                printf("paho_mqtt_riot: RPC name too long\n");
                return;
            }
            else {
                memcpy(rpc->rpc_name, sep+1, rpc_name_len);
                rpc->rpc_name[rpc_name_len] = '\0';
            }

            while(parsed_bytes < (int)data->message->payloadlen) {
                char* temp = params_sep+1;
                params_sep = search_buffer(params_sep+1, (int)data->message->payloadlen-parsed_bytes, ",", 1);
                int param_len;
                if(params_sep != NULL) {
                    param_len = params_sep-temp;
                } else {
                    /* final param */
                    param_len = (int)data->message->payloadlen-parsed_bytes;
                }
                parsed_bytes += param_len+1;
                if (param_len >= MAX_PARAM_LENGTH) {
                    printf("paho_mqtt_riot: Param too long\n");
                    continue;
                }
                memcpy(rpc->params[rpc->num_params], temp, param_len);
                rpc->params[rpc->num_params][param_len] = '\0';
                rpc->num_params += 1;
            }

            m.type = MSG_TYPE_RPC;
            m.content.ptr = rpc;
            int res;
            if((res = msg_send(&m, myno_device_pid)) != 1) {
                printf("paho_mqtt_riot: Sending message failed (%d)\n", res);
            }
        }
    }
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

static int _cmd_start(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    printf("Starting MYNO device\n");

    /* Assign IP address */
    _ip_a_a(DEVICE_IP, DEVICE_IP_PREFIX_LENGTH);

    int res = 0;
    /* Connect to broker */
    if ((res = myno_mqtt_connect(DEFAULT_MQTT_BROKER, DEFAULT_MQTT_PORT)) != 0) {
        return res;
    }
    /* Publish device description */
    if ((res = myno_mqtt_publish(ontology, strlen(ontology),
                                 DEVICE_UUID ";", strlen(DEVICE_UUID)+1,
                                 TOPIC_ONTOLOGY, QOS0, BLOCK_SIZE)) != 0) {
        return res;
    }
    if ((res = myno_mqtt_publish(ONTOLOGY_END_DELIMITER, strlen(ONTOLOGY_END_DELIMITER),
                                 DEVICE_UUID ";", strlen(DEVICE_UUID)+1,
                                 TOPIC_ONTOLOGY, QOS0, BLOCK_SIZE)) != 0) {
        return res;
    }
    /* Subscribe to update RPC topic */
    if ((res = myno_mqtt_subscribe(TOPIC_UPDATE, QOS0, _on_cmd_received)) != 0) {
        return res;
    }
    return res;
}

static const shell_command_t shell_commands[] =
{
    { "start",  "Start the MYNO device",              _cmd_start  },
    { NULL,     NULL,                                 NULL        }
};

void* _shell_thread(void* arg)
{
    (void)arg;
    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);
    return NULL; /* should not be reached */
}

int main(void)
{
#ifdef MODULE_LWIP
    /* let LWIP initialize */
    ztimer_sleep(ZTIMER_MSEC, 1 * MS_PER_SEC);
#endif

#ifdef MODULE_GNRC
    msg_init_queue(_main_msg_queue, MAIN_QUEUE_SIZE);
#endif

    myno_device_pid = thread_getpid();

    thread_create(shell_thread_stack, sizeof(shell_thread_stack),
                  THREAD_PRIORITY_MAIN + 1,
                  THREAD_CREATE_STACKTEST,
                  _shell_thread, NULL, "shell_thread");

    myno_mqtt_init();

    msg_init_queue(rpc_message_queue, RPC_MESSAGE_QUEUE_SIZE);

    msg_t m;
    while(1) {
        msg_receive(&m);

        if(m.content.ptr == NULL) continue;
        switch(m.type) {
        case MSG_TYPE_RPC:
            ;
            rpc_message_t* rpc = (rpc_message_t*)m.content.ptr;
            printf("myno_device: Got RPC %s (ID %d)\n", rpc->rpc_name, rpc->reqid);

            char reqid_prefix[8];
            snprintf(reqid_prefix, sizeof(reqid_prefix)/sizeof(reqid_prefix[0]), "%d;", rpc->reqid);

            if(strncmp(rpc->rpc_name, CMD_GETTOKEN, strlen(CMD_GETTOKEN)) == 0) {
                handle_cmd_gettoken(reqid_prefix);
            }
            else if(strncmp(rpc->rpc_name, CMD_MANIFEST, strlen(CMD_MANIFEST)) == 0) {
                handle_cmd_manifest(reqid_prefix, rpc);
            }
            else if(strncmp(rpc->rpc_name, CMD_IMAGE, strlen(CMD_IMAGE)) == 0) {
                handle_cmd_update(reqid_prefix, rpc);
            }
            else if(strncmp(rpc->rpc_name, CMD_ROLLOVER_U, strlen(CMD_ROLLOVER_U)) == 0) {
                handle_cmd_rollover_u(reqid_prefix, rpc);
            }
            else if(strncmp(rpc->rpc_name, CMD_ROLLOVER_V, strlen(CMD_ROLLOVER_V)) == 0) {
                handle_cmd_rollover_v(reqid_prefix, rpc);
            }
            else {
                printf("myno_device: Unrecognized command\n");
            }
            break;
        case MSG_TYPE_IMAGE:
            ;
            mup_image_slice_t* slice = (mup_image_slice_t*)m.content.ptr;
            printf("myno_device: Got slice %d (%d bytes)\n", slice->num, slice->data_len);
            handle_image_slice(slice);
            break;
        default:
            printf("myno_device: Unrecognized message type\n");
            break;
        }

        free(m.content.ptr);
    }

    return 0; /* should not be reached */
}