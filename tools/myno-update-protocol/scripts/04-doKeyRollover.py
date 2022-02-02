#!/bin/env python3
import sys

import paho.mqtt.client as paho

from assets.keys import DEFAULT_KEY_ID_UPDATE_SERVER, DEFAULT_KEY_ID_VENDOR
from assets.key_manifests import key_manifest_vendor, key_manifest_vendor_fields, key_manifest_update_server, key_manifest_update_server_fields
from sign_manifest import sign_key_manifest_u, sign_key_manifest_v

DEVICE_UUID="TEST-DEVICE-MUP"
BROKER_ADDR="127.0.0.1"
BROKER_PORT=1883

def setup():
    client = paho.Client("manifest", protocol=5)
    client.connect(BROKER_ADDR, BROKER_PORT, keepalive=60)
    return client

def main():
    client = setup()

    if len(sys.argv) == 2 and sys.argv[1] in ["U", "V"]:
        if sys.argv[1] == "V":
            key_id = DEFAULT_KEY_ID_VENDOR
        else:
            key_id = DEFAULT_KEY_ID_UPDATE_SERVER
    elif len(sys.argv) == 3 and sys.argv[1] in ["U", "V"]:
        key_id = int(sys.argv[2], base=16)
    else:
        print("Usage: ./04-doKeyRollover.py U|V [KEY-ID]")
        print("U means 'do key rollover for update server key'.")
        print("V means 'do key rollover for vendor key'.")
        print("KEY-ID optionally specifies which (old) key to use for signing.")
        # TODO Also make new key ID variable
        exit(1)

    if sys.argv[1] == "U":
        msg = b"0;PUB-UPDATE-KEY-U;"
        signed_manifest = sign_key_manifest_u(key_manifest_update_server, key_id)
        manifest_fields = key_manifest_update_server_fields
    elif sys.argv[1] == "V":
        msg = b"0;PUB-UPDATE-KEY-V;"
        signed_manifest = sign_key_manifest_v(key_manifest_vendor, key_id)
        manifest_fields = key_manifest_vendor_fields

    for item in manifest_fields:
        if isinstance(signed_manifest[item], bytes):
            msg += signed_manifest[item] + b","
        else:
            msg += bytes(str(signed_manifest[item]), "utf-8") + b","
    msg = msg[:-1]
    client.publish("yang/update/rollover/" + DEVICE_UUID, msg)

if __name__ == "__main__":
    main()