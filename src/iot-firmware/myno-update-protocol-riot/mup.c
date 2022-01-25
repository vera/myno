#include <stdio.h>
#include <string.h>

#include "MQTTClient.h"

#include "project-conf.h"
#include "myno-mqtt.h"
#include "mup.h"

static int expected_slice = 0;

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
        return myno_mqtt_publish(CMD_RESPONSE_ERROR, strlen(CMD_RESPONSE_ERROR),
                                 reqid_prefix, strlen(reqid_prefix),
                                 TOPIC_RESPONSE, QOS0, BLOCK_SIZE);
    }

    mup_manifest_t manifest;
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

    return myno_mqtt_publish(CMD_RESPONSE_OK, strlen(CMD_RESPONSE_OK),
                             reqid_prefix, strlen(reqid_prefix),
                             TOPIC_RESPONSE, QOS0, BLOCK_SIZE);
}

int handle_cmd_update(char* reqid_prefix, rpc_message_t* rpc)
{
    if(rpc->num_params != 1
       && strncmp(rpc->params[0], CMD_IMAGE_START_KEYWORD, strlen(rpc->params[0])) != 0) {
        printf("mup: Received unknwon update command\n");
        return myno_mqtt_publish(CMD_RESPONSE_ERROR, strlen(CMD_RESPONSE_ERROR),
                          reqid_prefix, strlen(reqid_prefix),
                          TOPIC_RESPONSE, QOS0, BLOCK_SIZE);
    }

    printf("mup: Starting update\n");
    // TODO Check hash and signatures

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
    expected_slice += 1;

    char num_prefix[8];
    snprintf(num_prefix, sizeof(num_prefix)/sizeof(num_prefix[0]), "%d", slice->num);

    return myno_mqtt_publish(UPDATE_SLICE_ACK, strlen(UPDATE_SLICE_ACK),
                             num_prefix, strlen(num_prefix),
                             TOPIC_UPDATE_IMAGE_RES, QOS0, BLOCK_SIZE);
}