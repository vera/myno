#!/bin/bash
DEVICE_UUID="TEST-DEVICE-MUP"

curl \
-F inputUpdateImage=@../myno-device.bin \
http://localhost:5000/function_call/$DEVICE_UUID/funcPubUpdateImage