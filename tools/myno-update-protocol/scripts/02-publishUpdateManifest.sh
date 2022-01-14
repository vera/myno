#!/bin/bash
mosquitto_sub -t 'response/MYNO-UPDATE-8A12-4F4F-8F69-6B8F3C2E78DD' -v -C 9 &

# 825c8c39487fcb868c8f2d52c8c360a3d5fb4cbe2e7d90e1a98498a87d175c56a670ee374961e7ccdb4f0702c82f7c3cf5a2a63463a3f90bce5ae9a8fc3197

mosquitto_pub -t "yang/update/manifest/MYNO-UPDATE-8A12-4F4F-8F69-6B8F3C2E78DD" -m "36;PUB-UPDATE-MANIFEST;0,APP"
sleep 2
mosquitto_pub -t "yang/update/manifest/MYNO-UPDATE-8A12-4F4F-8F69-6B8F3C2E78DD" -m "37;PUB-UPDATE-MANIFEST;1,0"
sleep 2
mosquitto_pub -t "yang/update/manifest/MYNO-UPDATE-8A12-4F4F-8F69-6B8F3C2E78DD" -m "38;PUB-UPDATE-MANIFEST;2,0b7a2d44cd0dc6f00a7f17b383747c89c24a1d21f77652dbee61feb62ca65894"
sleep 2
mosquitto_pub -t "yang/update/manifest/MYNO-UPDATE-8A12-4F4F-8F69-6B8F3C2E78DD" -m "39;PUB-UPDATE-MANIFEST;3,555"
sleep 2
mosquitto_pub -t "yang/update/manifest/MYNO-UPDATE-8A12-4F4F-8F69-6B8F3C2E78DD" -m "40;PUB-UPDATE-MANIFEST;4,2"
sleep 2
mosquitto_pub -t "yang/update/manifest/MYNO-UPDATE-8A12-4F4F-8F69-6B8F3C2E78DD" -m "41;PUB-UPDATE-MANIFEST;5,1"
sleep 2
mosquitto_pub -t "yang/update/manifest/MYNO-UPDATE-8A12-4F4F-8F69-6B8F3C2E78DD" -m "42;PUB-UPDATE-MANIFEST;6,ee2c882f83ae522c598ef02d980b1e4607f2bdb9c5d1d98cf4b817f7ccbf6fb4bc6a3e23bed1aaf3709803044c80b73a0c6be3dfa718184bd0d375a45819e1fd"
sleep 2
mosquitto_pub -t "yang/update/manifest/MYNO-UPDATE-8A12-4F4F-8F69-6B8F3C2E78DD" -m "43;PUB-UPDATE-MANIFEST;7,123456"
sleep 2
mosquitto_pub -t "yang/update/manifest/MYNO-UPDATE-8A12-4F4F-8F69-6B8F3C2E78DD" -m "44;PUB-UPDATE-MANIFEST;8,fe1763e57d007f218cc7d7479deae74e7ad72b1a78adde4267b0267d91cc5c8bae8f603046a9279b7c302ab3452519e42b19b5d683658e69e65c0f5cff8c5e45"
sleep 2
mosquitto_pub -t "yang/update/manifest/MYNO-UPDATE-8A12-4F4F-8F69-6B8F3C2E78DD" -m "45;PUB-UPDATE-MANIFEST;9,START"
