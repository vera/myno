#ifndef _MUP_H
#define _MUP_H
#include "hashes/sha256.h"

#include "project-conf.h"

#define CMD_GETTOKEN            "GET-DEVICE-TOKEN"
#define CMD_MANIFEST            "PUB-UPDATE-MANIFEST"
#define CMD_IMAGE               "PUB-UPDATE-IMAGE"

#define CMD_IMAGE_START_KEYWORD "FIN"

#define CMD_GETTOKEN_RESPONSE   "123456,1"

#define UPDATE_SLICE_ACK        ",OK"

#define TOPIC_UPDATE            "yang/update/+/" DEVICE_UUID
#define TOPIC_UPDATE_IMAGE      "yang/update/image/" DEVICE_UUID "/slices"
#define TOPIC_UPDATE_IMAGE_RES  "yang/update/image/" DEVICE_UUID "/response"

#define MSG_TYPE_IMAGE          0x02

#define APP_ID_LEN              32
#define HASH_LEN                SHA256_DIGEST_LENGTH
#define SIGNATURE_LEN           64

#define SLICE_SIZE              256

typedef struct mup_keyinfo_t {
    uint32_t kid;
    int32_t alg;
    uint32_t kty;
    int32_t crv;
} mup_keyinfo_t;

typedef struct mup_inner_manifest_t {
    char app_id[APP_ID_LEN];
    int link_offset;
    uint8_t hash[HASH_LEN];
    int size;
    int version;
    int old_version;
    mup_keyinfo_t inner_keyinfo;
} mup_inner_manifest_t;

typedef struct mup_outer_manifest_t {
    char app_id[APP_ID_LEN];
    int link_offset;
    uint8_t hash[HASH_LEN];
    int size;
    int version;
    int old_version;
    mup_keyinfo_t inner_keyinfo;
    uint8_t inner_signature[SIGNATURE_LEN];
    int nonce;
    mup_keyinfo_t outer_keyinfo;
} mup_outer_manifest_t;

typedef struct mup_manifest_t {
    char app_id[APP_ID_LEN];
    int link_offset;
    uint8_t hash[HASH_LEN];
    int size;
    int version;
    int old_version;
    mup_keyinfo_t inner_keyinfo;
    uint8_t inner_signature[SIGNATURE_LEN];
    int nonce;
    mup_keyinfo_t outer_keyinfo;
    uint8_t outer_signature[SIGNATURE_LEN];
} mup_manifest_t;

typedef struct mup_image_slice_t {
    int num;
    int data_len;
    uint8_t data[SLICE_SIZE];
} mup_image_slice_t;


#ifndef IANA_COSE_MAPPINGS
#define IANA_COSE_MAPPINGS

/* https://www.iana.org/assignments/cose/cose.xhtml#algorithms */
#define IANA_COSE_ALG_ES256 -7

/* https://www.iana.org/assignments/cose/cose.xhtml#key-type */
#define IANA_COSE_KTY_EC2 2

/* https://www.iana.org/assignments/cose/cose.xhtml#elliptic-curves */
#define IANA_COSE_CRV_P256 1
#define IANA_COSE_CRV_SECP256K1 8

#endif /* IANA_COSE_MAPPINGS */


int handle_cmd_gettoken(char* reqid_prefix);

int handle_cmd_manifest(char* reqid_prefix, rpc_message_t* rpc);

int handle_cmd_update(char* reqid_prefix, rpc_message_t* rpc);

int handle_image_slice(mup_image_slice_t* slice);

#endif /* _MUP_H */