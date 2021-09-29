This repository should be regarded as an experimental research.

# MYNO: MQTT-YANG-NETCONF-Ontology

We developed an architecture for network management of IoT devices in the Internet of Things (IoT) based on the standards MQTT, YANG and NETCONF. The IoT device descriptions are based on the oneM2M Base Ontology.

## MQTT-NETCONF Bridge

The bridge maps between NETCONF RPCs and MQTT PUBLISH messages. It parses the ontology-based device descriptions to YANG, the data modelling language for the NETCONF. The implementation of the bridge is in the folder `src/mqtt-netconf-bridge`. 

## MYNO Update Protocol

The device implementation of the MUP is in the folder `src/iot-firmware/myno-update-protocol`. It is required in addition to the update server, which is included in the NETCONF client in the folder `src/netconf-client-update-server`.

The device application is in subfolder *CC2538-app*. The device sends a device token, validates the manifest and receives the update image.
For details of the CC2538EM hardware, see also: https://github.com/contiki-os/contiki/tree/master/platform/cc2538dk

The MUP ontology is in the folder `ontologies/myno-update-protocol`.

### Tools

The folder `tools/myno-update-protocol` contains different tools: 

1. <u>micro-ecc-signature</u> is used for creation of signatures and keys. 
2. <u>compress-image</u> script is used to compress the update image file. 
3. <u>rdflib-jsonld</u> is the python library with a bugfix for namespaces containing #.

## Virtual Devices

We provide the current state of the ontology for Virtual Device in the folder `ontologies/virtual-device`.

# License

This project is licensed under the terms of the GNU GPLv3 License.
