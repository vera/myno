#!/bin/bash 
mosquitto_sub -t 'response/MYNO-UPDATE-8A12-4F4F-8F69-6B8F3C2E78DD' -v -C 1 & 

mosquitto_pub -t "yang/update/token/MYNO-UPDATE-8A12-4F4F-8F69-6B8F3C2E78DD" -m "1;GET-DEVICE-TOKEN"


