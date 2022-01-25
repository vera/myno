#ifndef _MUP_H
#define _MUP_H
#include "project-conf.h"

#define CMD_GETTOKEN            "GET-DEVICE-TOKEN"
#define CMD_MANIFEST            "PUB-UPDATE-MANIFEST"
#define CMD_IMAGE               "PUB-UPDATE-IMAGE"

#define CMD_IMAGE_START_KEYWORD "FIN"

#define CMD_GETTOKEN_RESPONSE   "123456,1"

#define UPDATE_SLICE_ACK        ",OK"

#define TOPIC_UPDATE            "yang/update/#"
#define TOPIC_UPDATE_IMAGE      "yang/update/image/" DEVICE_UUID
#define TOPIC_UPDATE_IMAGE_RES  "yang/update/image/" DEVICE_UUID "/response"

#define MSG_TYPE_IMAGE          0x02

#define APP_ID_LEN              64
#define HASH_LEN                65
#define SIGNATURE_LEN           129

#define SLICE_SIZE              256

typedef struct mup_manifest_t {
    char app_id[APP_ID_LEN];
    int link_offset;
    char hash[HASH_LEN];
    int size;
    int version;
    int old_version;
    char inner_signature[129];
    int nonce;
    char outer_signature[SIGNATURE_LEN];
} mup_manifest_t;

typedef struct mup_image_slice_t {
    int num;
    int data_len;
    uint8_t data[SLICE_SIZE];
} mup_image_slice_t;

int handle_cmd_gettoken(char* reqid_prefix);

int handle_cmd_manifest(char* reqid_prefix, rpc_message_t* rpc);

int handle_cmd_update(char* reqid_prefix, rpc_message_t* rpc);

int handle_image_slice(mup_image_slice_t* slice);

#endif /* _MUP_H */