#!/bin/bash

curl \
-F input_1_AppId=APP \
-F input_2_LinkOffset=0 \
-F input_3_Hash=70b7516a55038fb2f755efc3f15f0979873452d2a833fb6c2b5e6147e72b3521 \
-F input_4_Size=100719 \
-F input_5_Version=2 \
-F input_6_OldVersion=1 \
-F input_7_InnerSignature=b2011db0fb6a51c39dc448904d8f9ded8fee989af26e59830d90a8506f608116ea8653a1ff00a0e46b35d5d2252f1f9f21659b452a565d50f95cfa782e9b0f85 \
-F input_8_DeviceNonce=123456 \
http://localhost:5000/function_call/MYNO-UPDATE-8A12-4F4F-8F69-6B8F3C2E78DD/funcPubUpdateManifest/