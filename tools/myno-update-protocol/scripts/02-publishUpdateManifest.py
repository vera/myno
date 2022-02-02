#!/bin/env python3
import sys

import paho.mqtt.client as paho

from assets.keys import DEFAULT_KEY_ID_UPDATE_SERVER, DEFAULT_KEY_ID_VENDOR
from assets.manifests import basic_manifest, manifest_fields
from sign_manifest import sign_manifest

DEVICE_UUID="TEST-DEVICE-MUP"
BROKER_ADDR="127.0.0.1"
BROKER_PORT=1883


def setup():
    client = paho.Client("manifest", protocol=5)
    client.connect(BROKER_ADDR, BROKER_PORT, keepalive=60)
    return client

def main():
    if len(sys.argv) == 1:
        key_id_vendor = DEFAULT_KEY_ID_VENDOR
        key_id_update_server = DEFAULT_KEY_ID_UPDATE_SERVER
    elif len(sys.argv) == 3:
        key_id_vendor = int(sys.argv[1], base=16)
        key_id_update_server = int(sys.argv[2], base=16)
    else:
        print("Usage: ./02-publishUpdateManifest.py [VENDOR-KEY-ID UPDATE-SERVER-KEY-ID]")
        exit(1)

    client = setup()

    signed_manifest = sign_manifest(basic_manifest, key_id_vendor, key_id_update_server)
    msg = b"0;PUB-UPDATE-MANIFEST;"
    for item in manifest_fields:
        if isinstance(signed_manifest[item], bytes):
            msg += signed_manifest[item] + b","
        else:
            msg += bytes(str(signed_manifest[item]), "utf-8") + b","
    msg = msg[:-1]
    print("Publishing: ", msg.hex())
    client.publish("yang/update/manifest/" + DEVICE_UUID, msg)

if __name__ == "__main__":
    main()