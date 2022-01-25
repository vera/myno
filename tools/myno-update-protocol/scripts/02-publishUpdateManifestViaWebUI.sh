#!/bin/bash
DEVICE_UUID="TEST-DEVICE-MUP"

curl \
-F input_1_AppId=APP \
-F input_2_LinkOffset=0 \
-F input_3_Hash=cc6cf68547fe9a90070da75bdc953f34af1c708d1baf8c2912070d2f05845f73 \
-F input_4_Size=555 \
-F input_5_Version=2 \
-F input_6_OldVersion=1 \
-F input_7_InnerSignature=99fe5c11de573700bbf10d56052d77e3faafc0239dd5381ae1723945970a31436094181e03b38e84ca5338f58354977f06d144ac65fc1e58e8dd6a73b9130b99 \
-F input_8_DeviceNonce=123456 \
http://localhost:5000/function_call/$DEVICE_UUID/funcPubUpdateManifest