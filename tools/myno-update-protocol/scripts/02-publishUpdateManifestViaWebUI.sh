#!/bin/bash
DEVICE_UUID="TEST-DEVICE-MUP"

curl \
-F input_01_AppId=APP \
-F input_02_LinkOffset=0 \
-F input_03_Hash=cc6cf68547fe9a90070da75bdc953f34af1c708d1baf8c2912070d2f05845f73 \
-F input_04_Size=1 \
-F input_05_Version=2 \
-F input_06_OldVersion=1 \
-F input_07_InnerKeyInfo=fecafecaf9ffffff0200000001000000 \
-F input_08_InnerSignature=3c952d570be27aeda9f3ff0fd6c96c64f8b91b2751fdf720de0936a6687803d6491db967c0541750ad6bef103481b044834697832d7eb7153da4c6548385d4b0 \
-F input_09_DeviceNonce=1234567890 \
-F input_10_OuterKeyInfo=efbeaddef9ffffff0200000001000000 \
-F input_11_OuterSignature=5a5ed8526d573cbfd72196cba07a7e5456b1a16f1e65478738de8af0736f5da747cbc95a9da0b1664d4a1aa088a1d5e42f09f27cae91fb2539cec84d1a1d0e66 \
http://localhost:5000/function_call/$DEVICE_UUID/funcPubUpdateManifest