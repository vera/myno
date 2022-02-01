#ifndef _PROJECT_CONF_H_
#define _PROJECT_CONF_H_

#define MQTT_VERSION_v5                 4       /* MQTT v3.1.1 version is 4 */
#define COMMAND_TIMEOUT_MS              4000

/**
 * @brief Default MQTT port
 */
#define DEFAULT_MQTT_PORT               1883

/**
 * @brief Default MQTT IP address
 */
#define DEFAULT_MQTT_BROKER             "fd00::1"

/**
 * @brief Keepalive timeout in seconds
 */
#define DEFAULT_KEEPALIVE_SEC           10

#ifndef DEFAULT_MQTT_CLIENT_ID
#define DEFAULT_MQTT_CLIENT_ID          ""
#endif

#ifndef DEFAULT_MQTT_USER
#define DEFAULT_MQTT_USER               ""
#endif

#ifndef DEFAULT_MQTT_PWD
#define DEFAULT_MQTT_PWD                ""
#endif

#ifndef MAX_LEN_TOPIC
#define MAX_LEN_TOPIC                   64
#endif

#ifndef MAX_TOPICS
#define MAX_TOPICS                      4
#endif

#define DEVICE_UUID                     "TEST-DEVICE-MUP"

#define TOPIC_ONTOLOGY                  "yang/config"
#define TOPIC_RESPONSE                  "response/" DEVICE_UUID

#define CMD_RESPONSE_OK                 "10_OK"
#define CMD_RESPONSE_NOOP               "11_NOOP"
#define CMD_RESPONSE_ERROR              "12_ERROR"

#define ONTOLOGY_END_DELIMITER          "END"

/* Size of the MQTT read buffer
 * (should be larger than SLICE_SIZE when using MUP) */
#define BUF_SIZE                        512

#define BLOCK_SIZE                      256

#define DEVICE_IP                       "fd00::2"
#define DEVICE_IP_PREFIX_LENGTH         "64"

#define RPC_MESSAGE_QUEUE_SIZE          8

#define MSG_TYPE_RPC                    0x01

#define RPC_NAME_LENGTH                 32
#define MAX_PARAM_LENGTH                129
#define MAX_NUM_PARAMS                  16

#ifdef MODULE_GNRC
#define MAIN_QUEUE_SIZE                 8
#endif /* MODULE_GNRC */

typedef struct rpc_message_t {
    char rpc_name[RPC_NAME_LENGTH];
    unsigned int reqid;
    uint8_t params[MAX_NUM_PARAMS][MAX_PARAM_LENGTH];
    unsigned int num_params;
} rpc_message_t;

#endif /* _PROJECT_CONF_H */