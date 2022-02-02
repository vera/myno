#include <stdio.h>
#include <string.h>

#include "paho_mqtt.h"
#include "MQTTClient.h"
#include "uECC.h"

#include "project-conf.h"
#include "myno-mqtt.h"
#include "mup.h"
#include "mup-keys.h"

static mup_manifest_t manifest;

static sha256_context_t sha256_ctx;

static int expected_slice = 0;

static kernel_pid_t myno_device_pid;


inline int _send_rpc_error_response(char* reqid_prefix) {
    return myno_mqtt_publish(CMD_RESPONSE_ERROR, strlen(CMD_RESPONSE_ERROR),
                             reqid_prefix, strlen(reqid_prefix),
                             TOPIC_RESPONSE, QOS0, BLOCK_SIZE);
}

static int _check_signature(uint8_t* key, mup_keyinfo_t keyinfo, void* buf, size_t buf_len, uint8_t* signature, size_t signature_len) {
    if(keyinfo.alg == IANA_COSE_ALG_ES256 && keyinfo.kty == IANA_COSE_KTY_EC2) {
        sha256_context_t ctx;
        uint8_t digest[SHA256_DIGEST_LENGTH];

        sha256_init(&ctx);
        sha256_update(&ctx, buf, buf_len);
        sha256_final(&ctx, digest);

        uECC_Curve curve;
        switch(keyinfo.crv) {
            case IANA_COSE_CRV_P256:
                curve = uECC_secp256r1();
                break;
            case IANA_COSE_CRV_SECP256K1:
                curve = uECC_secp256k1();
                break;
            default:
                printf("mup: Failed to check signature: Unknown curve.\n");
                return -2;
        }

        if(!uECC_verify(key, digest, sizeof(digest), signature, curve)) {
            return -1;
        }
        else {
            return 0;
        }
    }
    else {
        printf("mup: Failed to check signature: Unknown key type or algorithm.\n");
        return -2;
    }
}

static void _on_slice_received(MessageData *data)
{
    msg_t m;

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

int handle_cmd_gettoken(char* reqid_prefix)
{
    printf("mup: Sending device token\n");
    return myno_mqtt_publish(CMD_GETTOKEN_RESPONSE, strlen(CMD_GETTOKEN_RESPONSE),
                             reqid_prefix, strlen(reqid_prefix),
                             TOPIC_RESPONSE, QOS0, BLOCK_SIZE);
}

int handle_cmd_manifest(char* reqid_prefix, rpc_message_t* rpc)
{
    if(rpc->num_params != 11) {
        printf("mup: Received manifest with wrong number of fields (%d)\n", rpc->num_params);
        return _send_rpc_error_response(reqid_prefix);
    }

    strncpy(manifest.app_id, (char*)rpc->params[0], APP_ID_LEN);
    manifest.link_offset = atoi((char*)rpc->params[1]);
    memcpy(manifest.hash, rpc->params[2], HASH_LEN);
    manifest.size = atoi((char*)rpc->params[3]);
    manifest.version = atoi((char*)rpc->params[4]);
    manifest.old_version = atoi((char*)rpc->params[5]);
    memcpy(&manifest.inner_keyinfo, rpc->params[6], sizeof(mup_keyinfo_t));
    memcpy(manifest.inner_signature, rpc->params[7], SIGNATURE_LEN);
    manifest.nonce = atoi((char*)rpc->params[8]);
    memcpy(&manifest.outer_keyinfo, rpc->params[9], sizeof(mup_keyinfo_t));
    memcpy(manifest.outer_signature, rpc->params[10], SIGNATURE_LEN);

    printf("mup: Received manifest\n");
    printf("mup: - App ID: %s\n", manifest.app_id);
    printf("mup: - Link offset: %d\n", manifest.link_offset);
    printf("mup: - Hash: ");
    for (size_t i = 0; i < HASH_LEN; i++) {
        printf("%02x", manifest.hash[i]);
    }
    printf("\n");
    printf("mup: - Size: %d\n", manifest.size);
    printf("mup: - Version: %d\n", manifest.version);
    printf("mup: - Old version: %d\n", manifest.old_version);
    printf("mup: - Inner key info: Key ID: 0x%04X, Algorithm: %d, Key Type: %d, Curve: %d\n", manifest.inner_keyinfo.kid, manifest.inner_keyinfo.alg, manifest.inner_keyinfo.kty, manifest.inner_keyinfo.crv);
    printf("mup: - Inner signature: ");
    for (size_t i = 0; i < SIGNATURE_LEN; i++) {
        printf("%02x", manifest.inner_signature[i]);
    }
    printf("\n");
    printf("mup: - Nonce: %d\n", manifest.nonce);
    printf("mup: - Outer key info: Key ID: 0x%04X, Algorithm: %d, Key Type: %d, Curve: %d\n", manifest.outer_keyinfo.kid, manifest.outer_keyinfo.alg, manifest.outer_keyinfo.kty, manifest.outer_keyinfo.crv);
    printf("mup: - Outer signature: ");
    for (size_t i = 0; i < SIGNATURE_LEN; i++) {
        printf("%02x", manifest.outer_signature[i]);
    }
    printf("\n");

    if(_check_signature(public_manufacturer, manifest.inner_keyinfo, (mup_inner_manifest_t*)&manifest, sizeof(mup_inner_manifest_t), manifest.inner_signature, SIGNATURE_LEN) < 0) {
        printf("mup: Inner signature invalid\n");
        return _send_rpc_error_response(reqid_prefix);
    }

    if(_check_signature(public_updateserver, manifest.outer_keyinfo, (mup_outer_manifest_t*)&manifest, sizeof(mup_outer_manifest_t), manifest.outer_signature, SIGNATURE_LEN) < 0) {
        printf("mup: Outer signature invalid\n");
        return _send_rpc_error_response(reqid_prefix);
    }

    printf("mup: Signature checks successful\n");

    expected_slice = 0;
    /* Subscribe to update slice topic */
    myno_device_pid = thread_getpid();
    int res;
    if ((res = myno_mqtt_subscribe(TOPIC_UPDATE_IMAGE, QOS0, _on_slice_received)) != 0) {
        printf("mup: Unable to subscribe to slice topic (%d)\n", res);
    }

    return myno_mqtt_publish(CMD_RESPONSE_OK, strlen(CMD_RESPONSE_OK),
                             reqid_prefix, strlen(reqid_prefix),
                             TOPIC_RESPONSE, QOS0, BLOCK_SIZE);
}

int handle_cmd_update(char* reqid_prefix, rpc_message_t* rpc)
{
    if(rpc->num_params != 1
       && strncmp((char*)rpc->params[0], CMD_IMAGE_START_KEYWORD, strlen((char*)rpc->params[0])) != 0) {
        printf("mup: Received unknown update command\n");
        return _send_rpc_error_response(reqid_prefix);
    }

    printf("mup: Starting update\n");

    uint8_t digest[SHA256_DIGEST_LENGTH];
    sha256_final(&sha256_ctx, digest);
    if(memcmp(digest, manifest.hash, HASH_LEN) != 0) {
        printf("mup: Hash doesn't match manifest hash\n");
        return _send_rpc_error_response(reqid_prefix);
    }

    printf("mup: Update checks successful\n");

    // TODO Install + reboot

    return myno_mqtt_publish(CMD_RESPONSE_OK, strlen(CMD_RESPONSE_OK),
                             reqid_prefix, strlen(reqid_prefix),
                             TOPIC_RESPONSE, QOS0, BLOCK_SIZE);
}

int handle_cmd_rollover_u(char* reqid_prefix, rpc_message_t* rpc)
{
    if(rpc->num_params != 7) {
        printf("mup: Received key manifest with wrong number of fields (%d)\n", rpc->num_params);
        return _send_rpc_error_response(reqid_prefix);
    }

    mup_key_manifest_u_t key_manifest;
    strncpy(key_manifest.app_id, (char*)rpc->params[0], APP_ID_LEN);
    key_manifest.version = atoi((char*)rpc->params[1]);
    memcpy(&key_manifest.new_keyinfo, rpc->params[2], sizeof(mup_keyinfo_t));
    memcpy(&key_manifest.new_key, rpc->params[3], KEY_LEN);
    key_manifest.nonce = atoi((char*)rpc->params[4]);
    memcpy(&key_manifest.keyinfo, rpc->params[5], sizeof(mup_keyinfo_t));
    memcpy(key_manifest.signature, rpc->params[6], SIGNATURE_LEN);

    printf("mup: Received key manifest\n");
    printf("mup: - App ID: %s\n", key_manifest.app_id);
    printf("mup: - Version: %d\n", key_manifest.version);
    printf("mup: - New key info: Key ID: 0x%04X, Algorithm: %d, Key Type: %d, Curve: %d\n", key_manifest.new_keyinfo.kid, key_manifest.new_keyinfo.alg, key_manifest.new_keyinfo.kty, key_manifest.new_keyinfo.crv);
    printf("mup: - New key: ");
    for (size_t i = 0; i < KEY_LEN; i++) {
        printf("%02x", key_manifest.new_key[i]);
    }
    printf("\n");
    printf("mup: - Nonce: %d\n", key_manifest.nonce);
    printf("mup: - Key info: Key ID: 0x%04X, Algorithm: %d, Key Type: %d, Curve: %d\n", key_manifest.keyinfo.kid, key_manifest.keyinfo.alg, key_manifest.keyinfo.kty, key_manifest.keyinfo.crv);
    printf("mup: - Signature: ");
    for (size_t i = 0; i < SIGNATURE_LEN; i++) {
        printf("%02x", key_manifest.signature[i]);
    }
    printf("\n");

    if(_check_signature(public_updateserver, key_manifest.keyinfo, &key_manifest, sizeof(mup_key_manifest_u_t)-SIGNATURE_LEN, key_manifest.signature, SIGNATURE_LEN) < 0) {
        printf("mup: Update server key rollover signature invalid\n");
        return _send_rpc_error_response(reqid_prefix);
    }

    memcpy(public_updateserver, key_manifest.new_key, KEY_LEN);
    printf("mup: Successfully stored new update server key\n");

    return myno_mqtt_publish(CMD_RESPONSE_OK, strlen(CMD_RESPONSE_OK),
                             reqid_prefix, strlen(reqid_prefix),
                             TOPIC_RESPONSE, QOS0, BLOCK_SIZE);
}

int handle_cmd_rollover_v(char* reqid_prefix, rpc_message_t* rpc)
{
    if(rpc->num_params != 9) {
        printf("mup: Received key manifest with wrong number of fields (%d)\n", rpc->num_params);
        return _send_rpc_error_response(reqid_prefix);
    }

    mup_key_manifest_v_t key_manifest;
    strncpy(key_manifest.app_id, (char*)rpc->params[0], APP_ID_LEN);
    key_manifest.version = atoi((char*)rpc->params[1]);
    memcpy(&key_manifest.new_keyinfo, rpc->params[2], sizeof(mup_keyinfo_t));
    memcpy(&key_manifest.new_key, rpc->params[3], KEY_LEN);
    memcpy(&key_manifest.inner_keyinfo, rpc->params[4], sizeof(mup_keyinfo_t));
    memcpy(key_manifest.inner_signature, rpc->params[5], SIGNATURE_LEN);
    key_manifest.nonce = atoi((char*)rpc->params[6]);
    memcpy(&key_manifest.outer_keyinfo, rpc->params[7], sizeof(mup_keyinfo_t));
    memcpy(key_manifest.outer_signature, rpc->params[8], SIGNATURE_LEN);

    printf("mup: Received key manifest\n");
    printf("mup: - App ID: %s\n", key_manifest.app_id);
    printf("mup: - Version: %d\n", key_manifest.version);
    printf("mup: - New key info: Key ID: 0x%04X, Algorithm: %d, Key Type: %d, Curve: %d\n", key_manifest.new_keyinfo.kid, key_manifest.new_keyinfo.alg, key_manifest.new_keyinfo.kty, key_manifest.new_keyinfo.crv);
    printf("mup: - New key: ");
    for (size_t i = 0; i < KEY_LEN; i++) {
        printf("%02x", key_manifest.new_key[i]);
    }
    printf("\n");
    printf("mup: - Inner key info: Key ID: 0x%04X, Algorithm: %d, Key Type: %d, Curve: %d\n", key_manifest.inner_keyinfo.kid, key_manifest.inner_keyinfo.alg, key_manifest.inner_keyinfo.kty, key_manifest.inner_keyinfo.crv);
    printf("mup: - Inner signature: ");
    for (size_t i = 0; i < SIGNATURE_LEN; i++) {
        printf("%02x", key_manifest.inner_signature[i]);
    }
    printf("\n");
    printf("mup: - Nonce: %d\n", key_manifest.nonce);
    printf("mup: - Outer key info: Key ID: 0x%04X, Algorithm: %d, Key Type: %d, Curve: %d\n", key_manifest.outer_keyinfo.kid, key_manifest.outer_keyinfo.alg, key_manifest.outer_keyinfo.kty, key_manifest.outer_keyinfo.crv);
    printf("mup: - Outer signature: ");
    for (size_t i = 0; i < SIGNATURE_LEN; i++) {
        printf("%02x", key_manifest.outer_signature[i]);
    }
    printf("\n");

    if(_check_signature(public_manufacturer, key_manifest.inner_keyinfo, (mup_inner_key_manifest_v_t*)&key_manifest, sizeof(mup_inner_key_manifest_v_t), key_manifest.inner_signature, SIGNATURE_LEN) < 0) {
        printf("mup: Inner vendor key rollover signature invalid\n");
        return _send_rpc_error_response(reqid_prefix);
    }

    if(_check_signature(public_updateserver, key_manifest.outer_keyinfo, &key_manifest, sizeof(mup_key_manifest_v_t)-SIGNATURE_LEN, key_manifest.outer_signature, SIGNATURE_LEN) < 0) {
        printf("mup: Outer vendor key rollover signature invalid\n");
        return _send_rpc_error_response(reqid_prefix);
    }

    memcpy(public_manufacturer, key_manifest.new_key, KEY_LEN);
    printf("mup: Successfully stored new vendor key\n");

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