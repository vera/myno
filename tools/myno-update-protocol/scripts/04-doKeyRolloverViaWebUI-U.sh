#!/bin/bash
DEVICE_UUID="TEST-DEVICE-MUP"

curl \
-F input_01_AppId=0 \
-F input_02_Version=2 \
-F input_03_NewKeyInfo=efbeedfef9ffffff0200000001000000 \
-F input_04_NewKey=c3b993f64651add0f13cdd98824f04aca351b8349a25194f8b2e1b59ad9a163d2b8732c822c533c7c2ca19d6192062ef9598cbd6759bd58f77ac2fae8970fc6f \
-F input_05_DeviceNonce=1234567890 \
-F input_06_KeyInfo=efbeaddef9ffffff0200000001000000 \
-F input_07_Signature=50c80cc699c782977b067f4825533fd3d6a7b6e00fa1728147945ac64b050b1141eb5f2e153926599fd78e3514b7b8b34662804ed096d14ad5e3b84e9af1319e \
http://localhost:5000/function_call/$DEVICE_UUID/funcPubUpdateKeyU