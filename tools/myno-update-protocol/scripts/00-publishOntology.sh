#!/bin/bash
mosquitto_pub -t "yang/config" -f 00-ontology-device.txt

mosquitto_pub -t "yang/config" -m "MYNO-UPDATE-8A12-4F4F-8F69-6B8F3C2E78DD;END"
