#ifndef _MYNO_MQTT_H
#define _MYNO_MQTT_H

#include "paho_mqtt.h"
#include "MQTTClient.h"

void myno_mqtt_init(void);
int myno_mqtt_subscribe(const char* topic, enum QoS qos, void(*on_message)(MessageData*));
int myno_mqtt_publish(const char* payload, unsigned int payload_len,
                      const char* payload_prefix, unsigned int prefix_len,
                      char* topic, enum QoS qos,
                      unsigned int block_size);
int myno_mqtt_connect(char *remote_ip, int port);
int myno_mqtt_disconnect(void);

#endif /* _MYNO_MQTT_H */