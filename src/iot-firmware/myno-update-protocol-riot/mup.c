#include <stdio.h>
#include <string.h>

#include "MQTTClient.h"
#include "hashes/sha256.h"
#include "uECC.h"

#include "project-conf.h"
#include "myno-mqtt.h"
#include "mup.h"
#include "mup-keys.h"

static mup_manifest_t manifest;

static sha256_context_t sha256_ctx;

static int expected_slice = 0;


inline int _send_rpc_error_response(char* reqid_prefix) {
    return myno_mqtt_publish(CMD_RESPONSE_ERROR, strlen(CMD_RESPONSE_ERROR),
                             reqid_prefix, strlen(reqid_prefix),
                             TOPIC_RESPONSE, QOS0, BLOCK_SIZE);
}

static int _check_signature(uint8_t* key, char* buf, char* signature, size_t signature_len) {
    sha256_context_t ctx;
    uint8_t digest[SHA256_DIGEST_LENGTH];

    sha256_init(&ctx);
    sha256_update(&ctx, buf, strlen(buf));
    sha256_final(&ctx, digest);

    uint8_t s[64];
    for(size_t i = 0; i < sizeof(s)/sizeof(s[0]); i++) {
        sscanf(signature+(i*2), "%02hhx", &s[i]);
    }

    if(!uECC_verify(key, digest, sizeof(digest), s, uECC_secp256r1())) {
        return -1;
    }
    else {
        return 0;
    }
}

int handle_cmd_gettoken(char* reqid_prefix)
{
    printf("mup: Sending device token\n");
    return myno_mqtt_publish(CMD_GETTOKEN_RESPONSE, strlen(CMD_GETTOKEN_RESPONSE),
                             reqid_prefix, strlen(reqid_prefix),
                             TOPIC_RESPONSE, QOS0, BLOCK_SIZE);
}

int handle_cmd_manifest(char* reqid_prefix, rpc_message_t* rpc)
{
    if(rpc->num_params != 9) {
        printf("mup: Received manifest with wrong number of fields\n");
        return _send_rpc_error_response(reqid_prefix);
    }

    strncpy(manifest.app_id, rpc->params[0], APP_ID_LEN);
    manifest.link_offset = atoi(rpc->params[1]);
    strncpy(manifest.hash, rpc->params[2], HASH_LEN);
    manifest.size = atoi(rpc->params[3]);
    manifest.version = atoi(rpc->params[4]);
    manifest.old_version = atoi(rpc->params[5]);
    strncpy(manifest.inner_signature, rpc->params[6], SIGNATURE_LEN);
    manifest.nonce = atoi(rpc->params[7]);
    strncpy(manifest.outer_signature, rpc->params[8], SIGNATURE_LEN);

    printf("mup: Received manifest\n");
    printf("mup: - App ID: %s\n", manifest.app_id);
    printf("mup: - Link offset: %d\n", manifest.link_offset);
    printf("mup: - Hash: %s\n", manifest.hash);
    printf("mup: - Size: %d\n", manifest.size);
    printf("mup: - Version: %d\n", manifest.version);
    printf("mup: - Old version: %d\n", manifest.old_version);
    printf("mup: - Inner signature: %s\n", manifest.inner_signature);
    printf("mup: - Nonce: %d\n", manifest.nonce);
    printf("mup: - Outer signature: %s\n", manifest.outer_signature);

    expected_slice = 0;

    return myno_mqtt_publish(CMD_RESPONSE_OK, strlen(CMD_RESPONSE_OK),
                             reqid_prefix, strlen(reqid_prefix),
                             TOPIC_RESPONSE, QOS0, BLOCK_SIZE);
}

int handle_cmd_update(char* reqid_prefix, rpc_message_t* rpc)
{
    if(rpc->num_params != 1
       && strncmp(rpc->params[0], CMD_IMAGE_START_KEYWORD, strlen(rpc->params[0])) != 0) {
        printf("mup: Received unknown update command\n");
        return _send_rpc_error_response(reqid_prefix);
    }

    printf("mup: Starting update\n");

    uint8_t digest[SHA256_DIGEST_LENGTH];
    sha256_final(&sha256_ctx, digest);
    char actual_hash[HASH_LEN];
    for(int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        snprintf(actual_hash+(i*2), 3, "%02x", digest[i]);
    }
    if(strncmp(actual_hash, manifest.hash, HASH_LEN) != 0) {
        printf("mup: Hash (%s) doesn't match manifest hash (%s)\n", actual_hash, manifest.hash);
        return _send_rpc_error_response(reqid_prefix);
    }

    char buf[sizeof(mup_manifest_t)];
    snprintf(buf, sizeof(buf), "%s;%d;%.64s;%d;%d;%d",
             manifest.app_id, manifest.link_offset, manifest.hash,
             manifest.size, manifest.version, manifest.old_version);
    if(_check_signature(public_manufacturer, buf, manifest.inner_signature, SIGNATURE_LEN) < 0) {
        printf("mup: Inner signature invalid\n");
        return _send_rpc_error_response(reqid_prefix);
    }

    snprintf(buf, sizeof(buf), "%s;%d;%.64s;%d;%d;%d;%s;%d",
             manifest.app_id, manifest.link_offset, manifest.hash,
             manifest.size, manifest.version, manifest.old_version,
             manifest.inner_signature, manifest.nonce);
    if(_check_signature(public_updateserver, buf, manifest.outer_signature, SIGNATURE_LEN) < 0) {
        printf("mup: Outer signature invalid\n");
        return _send_rpc_error_response(reqid_prefix);
    }

    printf("mup: Update checks successful\n");

    // TODO Install + reboot

    return myno_mqtt_publish(CMD_RESPONSE_OK, strlen(CMD_RESPONSE_OK),
                             reqid_prefix, strlen(reqid_prefix),
                             TOPIC_RESPONSE, QOS0, BLOCK_SIZE);
}

int handle_image_slice(mup_image_slice_t* slice)
{
    if(slice->num != expected_slice) {
        printf("mup: Received unexpected slice\n");
        return -1;
    }
    if(expected_slice == 0) {
        sha256_init(&sha256_ctx);
    }
    expected_slice += 1;

    sha256_update(&sha256_ctx, slice->data, slice->data_len);

    char num_prefix[8];
    snprintf(num_prefix, sizeof(num_prefix)/sizeof(num_prefix[0]), "%d", slice->num);

    return myno_mqtt_publish(UPDATE_SLICE_ACK, strlen(UPDATE_SLICE_ACK),
                             num_prefix, strlen(num_prefix),
                             TOPIC_UPDATE_IMAGE_RES, QOS0, BLOCK_SIZE);
}