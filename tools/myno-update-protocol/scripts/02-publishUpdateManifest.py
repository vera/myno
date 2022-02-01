#!/bin/env python3
import paho.mqtt.client as paho

from assets.manifests import basic_manifest

DEVICE_UUID="TEST-DEVICE-MUP"
BROKER_ADDR="127.0.0.1"
BROKER_PORT=1883

def setup():
    client = paho.Client("manifest", protocol=5)
    client.connect(BROKER_ADDR, BROKER_PORT, keepalive=60)
    return client

client = setup()
msg = b"0;PUB-UPDATE-MANIFEST;"
for item in basic_manifest:
    if isinstance(basic_manifest[item], bytes):
        msg += basic_manifest[item] + b","
    else:
        msg += bytes(str(basic_manifest[item]), "utf-8") + b","
msg = msg[:-1]
client.publish("yang/update/manifest/" + DEVICE_UUID, msg)