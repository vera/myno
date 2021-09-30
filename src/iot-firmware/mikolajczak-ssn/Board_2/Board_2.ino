#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <BH1750.h>
#include <string.h>

// DEFINITIONS_
// OVERALL
#define UUID "BOARD2-8A12-4F4F-8F69-6B8F3C2E78ED"
#define BOARD "Board_2"

// SUBSCRIBED TOPICS
#define LED1_TOPIC_S "actuator/led/led_1/"UUID
#define LED2_TOPIC_S "actuator/led/led_2/"UUID
#define PING_TOPIC_S "ping/yang/config/"UUID
#define CRUD_TOPICS_S CONFIG_TOPIC_P"/+/response/"UUID
#define CREATE_TOPIC_S CONFIG_TOPIC_P"/create/response/"UUID
#define RETRIEVE_TOPIC_S CONFIG_TOPIC_P"/retrieve/response/"UUID
#define UPDATE_TOPIC_S CONFIG_TOPIC_P"/update/response/"UUID
#define DELETE_TOPIC_S CONFIG_TOPIC_P"/delete/response/"UUID

// PUBLISHING TOPICS
#define RESPONSE_TOPIC_P "response/"UUID
#define MOTION1_TOPIC_P "sensor/motion/motion_1/"UUID
#define BRIGHT1_TOPIC_P "sensor/brightness/brightness_1/"UUID

#define CONFIG_TOPIC_P "yang/config"
#define PING_TOPIC_P CONFIG_TOPIC_P"/ping/response"
#define CREATE_TOPIC_P CONFIG_TOPIC_P"/create"
#define RETRIEVE_TOPIC_P CONFIG_TOPIC_P"/retrieve"
#define UPDATE_TOPIC_P CONFIG_TOPIC_P"/update"
#define DELETE_TOPIC_P CONFIG_TOPIC_P"/delete"

#define BRIGHT1_SSN_P "sensor/brightness/brightness_1/ssn/"UUID
#define MOTION1_SSN_P "sensor/motion/motion_1/ssn/"UUID

#define LED1_SSN_P "actuator/led/led_1/ssn/"UUID
#define SENSOR_ERROR_TOPIC_P "sensor/error"

// CRUD RESPONSES
#define CRUD_OK "OK"
#define CRUD_NOTOK "NOTFOUND" // not used

// SLEEP
#define MINUTE 60000

//Wifi & MQTT Uni
const char* ssid = "IoTSens";
const char* password =  "heterogenIstAnstrengend!";
const char* mqtt_server = "192.168.43.141";
WiFiClient Board2;
PubSubClient client(Board2);


// Ontology & Device UUID
// Device UUID and ontology with placeholder "%s" for UUID -> like Michael Nowak
String device_uuid = UUID;
String ontology = "[{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#Command\",\"@type\":[\"http://www.w3.org/2002/07/owl#Class\"]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#ControllingFunctionality\",\"@type\":[\"http://www.w3.org/2002/07/owl#Class\"]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#Device\",\"@type\":[\"http://www.w3.org/2002/07/owl#Class\"]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#MeasuringFunctionality\",\"@type\":[\"http://www.w3.org/2002/07/owl#Class\"]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#Operation\",\"@type\":[\"http://www.w3.org/2002/07/owl#Class\"]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#OperationInput\",\"@type\":[\"http://www.w3.org/2002/07/owl#Class\"]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#OperationOutput\",\"@type\":[\"http://www.w3.org/2002/07/owl#Class\"]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#OperationState\",\"@type\":[\"http://www.w3.org/2002/07/owl#Class\"]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#OutputDataPoint\",\"@type\":[\"http://www.w3.org/2002/07/owl#Class\"]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#Service\",\"@type\":[\"http://www.w3.org/2002/07/owl#Class\"]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#ThingProperty\",\"@type\":[\"http://www.w3.org/2002/07/owl#Class\"]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#exposesCommand\",\"@type\":[\"http://www.w3.org/2002/07/owl#ObjectProperty\"]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#exposesFunctionality\",\"@type\":[\"http://www.w3.org/2002/07/owl#AnnotationProperty\"]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasCommand\",\"@type\":[\"http://www.w3.org/2002/07/owl#ObjectProperty\"]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasDataRestriction_pattern\",\"@type\":[\"http://www.w3.org/2002/07/owl#DatatypeProperty\"]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasFunctionality\",\"@type\":[\"http://www.w3.org/2002/07/owl#AnnotationProperty\"]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasInput\",\"@type\":[\"http://www.w3.org/2002/07/owl#ObjectProperty\"],\"http://www.w3.org/2000/01/rdf-schema#range\":[{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#ThingProperty\"}]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasOperation\",\"@type\":[\"http://www.w3.org/2002/07/owl#ObjectProperty\"]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasOperationState\",\"@type\":[\"http://www.w3.org/2002/07/owl#ObjectProperty\"]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasOutput\",\"@type\":[\"http://www.w3.org/2002/07/owl#ObjectProperty\"]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasOutputDataPoint\",\"@type\":[\"http://www.w3.org/2002/07/owl#ObjectProperty\"]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasService\",\"@type\":[\"http://www.w3.org/2002/07/owl#ObjectProperty\"]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasSubService\",\"@type\":[\"http://www.w3.org/2002/07/owl#ObjectProperty\"]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasThingProperty\",\"@type\":[\"http://www.w3.org/2002/07/owl#ObjectProperty\"]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasValue\",\"@type\":[\"http://www.w3.org/2002/07/owl#DatatypeProperty\"]},{\"@id\":\"http://yang-netconf-mqtt\",\"@type\":[\"http://www.w3.org/2002/07/owl#Ontology\"],\"http://www.w3.org/2002/07/owl#imports\":[{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology-v0_9_0\"}]},{\"@id\":\"http://yang-netconf-mqtt#YangDescription\",\"@type\":[\"http://www.w3.org/2002/07/owl#Class\"],\"http://www.w3.org/2000/01/rdf-schema#subClassOf\":[{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#ThingProperty\"}]},{\"@id\":\"http://yang-netconf-mqtt#cmdLed_1Color\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#Command\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasInput\":[{\"@id\":\"http://yang-netconf-mqtt#colorInput\"},{\"@id\":\"http://yang-netconf-mqtt#uuidInput\"}]},{\"@id\":\"http://yang-netconf-mqtt#cmdLed_1Off\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#Command\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasInput\":[{\"@id\":\"http://yang-netconf-mqtt#uuidInput\"}]},{\"@id\":\"http://yang-netconf-mqtt#cmdLed_1On\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#Command\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasInput\":[{\"@id\":\"http://yang-netconf-mqtt#uuidInput\"}]},{\"@id\":\"http://yang-netconf-mqtt#colorInput\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#OperationInput\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasDataRestriction_pattern\":[{\"@value\":\"all\"},{\"@value\":\"green\"},{\"@value\":\"red\"}],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasThingProperty\":[{\"@id\":\"http://yang-netconf-mqtt#colorYangDesc\"}]},{\"@id\":\"http://yang-netconf-mqtt#colorYangDesc\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://yang-netconf-mqtt#YangDescription\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasValue\":[{\"@value\":\"colorparamterforled_1\"}]},{\"@id\":\"http://yang-netconf-mqtt#deviceCategory\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#ThingProperty\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasValue\":[{\"@value\":\"Board_2\"}]},{\"@id\":\"http://yang-netconf-mqtt#deviceDesc\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://yang-netconf-mqtt#YangDescription\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasValue\":[{\"@value\":\"MQTT-DeviceidentifiedbyUUID\"}]},{\"@id\":\"http://yang-netconf-mqtt#deviceUuid\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#ThingProperty\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasValue\":[{\"@value\":\"%s\"}]},{\"@id\":\"http://yang-netconf-mqtt#funcDescBrightness\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://yang-netconf-mqtt#YangDescription\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasValue\":[{\"@value\":\"Getbrightnessfromsensor\"}]},{\"@id\":\"http://yang-netconf-mqtt#funcDescLed_1Color\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://yang-netconf-mqtt#YangDescription\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasValue\":[{\"@value\":\"SetColorofled_1\"}]},{\"@id\":\"http://yang-netconf-mqtt#funcDescLed_1Off\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://yang-netconf-mqtt#YangDescription\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasValue\":[{\"@value\":\"Turnled_1off,i.e.allsub-leds\"}]},{\"@id\":\"http://yang-netconf-mqtt#funcDescLed_1On\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://yang-netconf-mqtt#YangDescription\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasValue\":[{\"@value\":\"Turnled_1on,i.e.allsub-leds\"}]},{\"@id\":\"http://yang-netconf-mqtt#funcDescMotion\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://yang-netconf-mqtt#YangDescription\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasValue\":[{\"@value\":\"Getmotiondetectionfromsensor\"}]},{\"@id\":\"http://yang-netconf-mqtt#funcDetectMotion\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#MeasuringFunctionality\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasThingProperty\":[{\"@id\":\"http://yang-netconf-mqtt#funcDescMotion\"}]},{\"@id\":\"http://yang-netconf-mqtt#funcGetBrightness\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#MeasuringFunctionality\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasThingProperty\":[{\"@id\":\"http://yang-netconf-mqtt#funcDescBrightness\"}]},{\"@id\":\"http://yang-netconf-mqtt#funcLed_1Color\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#ControllingFunctionality\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasCommand\":[{\"@id\":\"http://yang-netconf-mqtt#cmdLed_1Color\"}],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasThingProperty\":[{\"@id\":\"http://yang-netconf-mqtt#funcDescLed_1Color\"}]},{\"@id\":\"http://yang-netconf-mqtt#funcLed_1Off\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#ControllingFunctionality\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasCommand\":[{\"@id\":\"http://yang-netconf-mqtt#cmdLed_1Off\"}],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasThingProperty\":[{\"@id\":\"http://yang-netconf-mqtt#funcDescLed_1Off\"}]},{\"@id\":\"http://yang-netconf-mqtt#funcLed_1On\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#ControllingFunctionality\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasCommand\":[{\"@id\":\"http://yang-netconf-mqtt#cmdLed_1On\"}],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasThingProperty\":[{\"@id\":\"http://yang-netconf-mqtt#funcDescLed_1On\"}]},{\"@id\":\"http://yang-netconf-mqtt#mqttMethod\",\"@type\":[\"http://www.w3.org/2002/07/owl#DatatypeProperty\"]},{\"@id\":\"http://yang-netconf-mqtt#mqttTopic\",\"@type\":[\"http://www.w3.org/2002/07/owl#DatatypeProperty\"]},{\"@id\":\"http://yang-netconf-mqtt#myDevice\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#Device\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasFunctionality\":[{\"@id\":\"http://yang-netconf-mqtt#funcDetectMotion\"},{\"@id\":\"http://yang-netconf-mqtt#funcGetBrightness\"},{\"@id\":\"http://yang-netconf-mqtt#funcLed_1Color\"},{\"@id\":\"http://yang-netconf-mqtt#funcLed_1Off\"},{\"@id\":\"http://yang-netconf-mqtt#funcLed_1On\"}],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasService\":[{\"@id\":\"http://yang-netconf-mqtt#servNetconf\"}],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasThingProperty\":[{\"@id\":\"http://yang-netconf-mqtt#deviceCategory\"},{\"@id\":\"http://yang-netconf-mqtt#deviceDesc\"},{\"@id\":\"http://yang-netconf-mqtt#deviceUuid\"}]},{\"@id\":\"http://yang-netconf-mqtt#opDescState\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://yang-netconf-mqtt#YangDescription\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasDataRestriction_pattern\":[{\"@value\":\"error\"},{\"@value\":\"nothingtodo\"},{\"@value\":\"successful\"}]},{\"@id\":\"http://yang-netconf-mqtt#opMqttLed_1Color\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#Operation\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#exposesCommand\":[{\"@id\":\"http://yang-netconf-mqtt#cmdLed_1Color\"}],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasInput\":[{\"@id\":\"http://yang-netconf-mqtt#colorInput\"},{\"@id\":\"http://yang-netconf-mqtt#uuidInput\"}],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasOperationState\":[{\"@id\":\"http://yang-netconf-mqtt#opState\"}],\"http://yang-netconf-mqtt#mqttMethod\":[{\"@value\":\"color\"}],\"http://yang-netconf-mqtt#mqttTopic\":[{\"@value\":\"actuator/led/led_1\"}]},{\"@id\":\"http://yang-netconf-mqtt#opMqttLed_1Off\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#Operation\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#exposesCommand\":[{\"@id\":\"http://yang-netconf-mqtt#cmdLed_1Off\"}],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasInput\":[{\"@id\":\"http://yang-netconf-mqtt#uuidInput\"}],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasOperationState\":[{\"@id\":\"http://yang-netconf-mqtt#opState\"}],\"http://yang-netconf-mqtt#mqttMethod\":[{\"@value\":\"OFF\"}],\"http://yang-netconf-mqtt#mqttTopic\":[{\"@value\":\"actuator/led/led_1\"}]},{\"@id\":\"http://yang-netconf-mqtt#opMqttLed_1On\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#Operation\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#exposesCommand\":[{\"@id\":\"http://yang-netconf-mqtt#cmdLed_1On\"}],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasInput\":[{\"@id\":\"http://yang-netconf-mqtt#uuidInput\"}],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasOperationState\":[{\"@id\":\"http://yang-netconf-mqtt#opState\"}],\"http://yang-netconf-mqtt#mqttMethod\":[{\"@value\":\"ON\"}],\"http://yang-netconf-mqtt#mqttTopic\":[{\"@value\":\"actuator/led/led_1\"}]},{\"@id\":\"http://yang-netconf-mqtt#opState\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#OperationState\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasDataRestriction_pattern\":[{\"@value\":\"ERROR\"},{\"@value\":\"NOOP\"},{\"@value\":\"OK\"}],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasThingProperty\":[{\"@id\":\"http://yang-netconf-mqtt#opDescState\"}]},{\"@id\":\"http://yang-netconf-mqtt#outDpBrightness\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#OutputDataPoint\"],\"http://yang-netconf-mqtt#mqttTopic\":[{\"@value\":\"sensor/brightness/brightness_1/%s\"}]},{\"@id\":\"http://yang-netconf-mqtt#outDpMotion\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#OutputDataPoint\"],\"http://yang-netconf-mqtt#mqttTopic\":[{\"@value\":\"sensor/motion/motion_1/%s\"}]},{\"@id\":\"http://yang-netconf-mqtt#servBrightness\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#Service\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#exposesFunctionality\":[{\"@id\":\"http://yang-netconf-mqtt#funcGetBrightness\"}],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasOutputDataPoint\":[{\"@id\":\"http://yang-netconf-mqtt#outDpBrightness\"}]},{\"@id\":\"http://yang-netconf-mqtt#servMotion\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#Service\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#exposesFunctionality\":[{\"@id\":\"http://yang-netconf-mqtt#funcDetectMotion\"}],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasOutputDataPoint\":[{\"@id\":\"http://yang-netconf-mqtt#outDpMotion\"}]},{\"@id\":\"http://yang-netconf-mqtt#servNetconf\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#Service\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#exposesFunctionality\":[{\"@id\":\"http://yang-netconf-mqtt#funcDetectMotion\"},{\"@id\":\"http://yang-netconf-mqtt#funcGetBrightness\"},{\"@id\":\"http://yang-netconf-mqtt#funcLed_1Color\"},{\"@id\":\"http://yang-netconf-mqtt#funcLed_1Off\"},{\"@id\":\"http://yang-netconf-mqtt#funcLed_1On\"}],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasOperation\":[{\"@id\":\"http://yang-netconf-mqtt#opMqttLed_1Color\"},{\"@id\":\"http://yang-netconf-mqtt#opMqttLed_1Off\"},{\"@id\":\"http://yang-netconf-mqtt#opMqttLed_1On\"}],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasSubService\":[{\"@id\":\"http://yang-netconf-mqtt#servBrightness\"},{\"@id\":\"http://yang-netconf-mqtt#servMotion\"}]},{\"@id\":\"http://yang-netconf-mqtt#uuidInput\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#OperationInput\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasInput\":[{\"@id\":\"http://yang-netconf-mqtt#deviceUuid\"}],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasThingProperty\":[{\"@id\":\"http://yang-netconf-mqtt#uuidYangDesc\"}]},{\"@id\":\"http://yang-netconf-mqtt#uuidYangDesc\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://yang-netconf-mqtt#YangDescription\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasValue\":[{\"@value\":\"TargetUUIDforrequest\"}]}]";
String ssn_brightness = "[{\"@id\":\"http://www.ontology-of-units-of-measure.org/resource/om-2/Illuminance\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.w3.org/ns/sosa/ObservableProperty\"],\"http://www.w3.org/ns/sosa/isObservedBy\":[{\"@id\":\"https://www.cs.uni-potsdam.de/bs/research/myno/sensordata#mylightsensor\"}]},{\"@id\":\"http://www.ontology-of-units-of-measure.org/resource/om-2/Measure\",\"@type\":[\"http://www.w3.org/2002/07/owl#Class\"]},{\"@id\":\"http://www.ontology-of-units-of-measure.org/resource/om-2/hasUnit\",\"@type\":[\"http://www.w3.org/2002/07/owl#AnnotationProperty\"]},{\"@id\":\"https://www.cs.uni-potsdam.de/bs/research/myno/sensordata\",\"@type\":[\"http://www.w3.org/2002/07/owl#Ontology\"],\"http://www.w3.org/2002/07/owl#imports\":[{\"@id\":\"http://www.ontology-of-units-of-measure.org/resource/om-2/\"},{\"@id\":\"http://www.w3.org/ns/sosa/\"},{\"@id\":\"http://www.w3.org/ns/ssn/\"}]},{\"@id\":\"https://www.cs.uni-potsdam.de/bs/research/myno/sensordata#%name\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.w3.org/ns/sosa/Observation\"],\"http://www.w3.org/ns/sosa/hasFeatureOfInterest\":[{\"@id\":\"https://www.cs.uni-potsdam.de/bs/research/myno/sensordata#room21\"}],\"http://www.w3.org/ns/sosa/hasResult\":[{\"@id\":\"https://www.cs.uni-potsdam.de/bs/research/myno/sensordata#measureLightResult\"}],\"http://www.w3.org/ns/sosa/hasSimpleResult\":[{\"@type\":\"http://www.w3.org/2001/XMLSchema#float\",\"@value\":\"-0.1\"}],\"http://www.w3.org/ns/sosa/isHostedBy\":[{\"@id\":\"https://www.cs.uni-potsdam.de/bs/research/myno/sensordata#ESP32_%p\"}],\"http://www.w3.org/ns/sosa/madeBySensor\":[{\"@id\":\"https://www.cs.uni-potsdam.de/bs/research/myno/sensordata#mylightsensor\"}],\"http://www.w3.org/ns/sosa/observedProperty\":[{\"@id\":\"http://www.ontology-of-units-of-measure.org/resource/om-2/Illuminance\"}],\"http://www.w3.org/ns/sosa/resultTime\":[{\"@type\":\"http://www.w3.org/2001/XMLSchema#dateTime\",\"@value\":\"0000-01-01T00:00:00+00:00\"}],\"http://www.w3.org/ns/sosa/usedProcedure\":[{\"@id\":\"https://www.cs.uni-potsdam.de/bs/research/myno/sensordata#mylightprocedure\"}]},{\"@id\":\"https://www.cs.uni-potsdam.de/bs/research/myno/sensordata#ESP32_%p\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.w3.org/ns/sosa/Platform\"],\"http://www.w3.org/ns/sosa/hosts\":[{\"@id\":\"https://www.cs.uni-potsdam.de/bs/research/myno/sensordata#mylightsensor\"}]},{\"@id\":\"https://www.cs.uni-potsdam.de/bs/research/myno/sensordata#ProcedureOutput\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.w3.org/ns/ssn/Output\"],\"http://www.w3.org/2000/01/rdf-schema#comment\":[{\"@value\":\"%s\"}]},{\"@id\":\"https://www.cs.uni-potsdam.de/bs/research/myno/sensordata#measureLightResult\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.ontology-of-units-of-measure.org/resource/om-2/Measure\",\"http://www.w3.org/ns/sosa/Result\"],\"http://www.ontology-of-units-of-measure.org/resource/om-2/hasUnit\":[{\"@id\":\"http://www.ontology-of-units-of-measure.org/resource/om-2/lux\"}],\"http://www.w3.org/ns/sosa/isResultOf\":[{\"@id\":\"https://www.cs.uni-potsdam.de/bs/research/myno/sensordata#%name\"}]},{\"@id\":\"https://www.cs.uni-potsdam.de/bs/research/myno/sensordata#mylightprocedure\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.w3.org/ns/sosa/Procedure\"]},{\"@id\":\"https://www.cs.uni-potsdam.de/bs/research/myno/sensordata#mylightsensor\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.w3.org/ns/sosa/Sensor\"],\"http://www.w3.org/ns/sosa/isHostedBy\":[{\"@id\":\"https://www.cs.uni-potsdam.de/bs/research/myno/sensordata#ESP32_%p\"}],\"http://www.w3.org/ns/sosa/madeObservation\":[{\"@id\":\"https://www.cs.uni-potsdam.de/bs/research/myno/sensordata#%name\"}],\"http://www.w3.org/ns/sosa/observes\":[{\"@id\":\"http://www.ontology-of-units-of-measure.org/resource/om-2/Illuminance\"}]},{\"@id\":\"https://www.cs.uni-potsdam.de/bs/research/myno/sensordata#room21\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.w3.org/ns/sosa/FeatureOfInterest\"],\"http://www.w3.org/ns/sosa/isFeatureOfInterestOf\":[{\"@id\":\"https://www.cs.uni-potsdam.de/bs/research/myno/sensordata#%name\"}]}]";

String ssn_motion = "[{\"@id\":\"http://www.ontology-of-units-of-measure.org/resource/om-2/Measure\",\"@type\":[\"http://www.w3.org/2002/07/owl#Class\"]},{\"@id\":\"http://www.ontology-of-units-of-measure.org/resource/om-2/hasUnit\",\"@type\":[\"http://www.w3.org/2002/07/owl#AnnotationProperty\"]},{\"@id\":\"https://www.cs.uni-potsdam.de/bs/research/myno/sensordata\",\"@type\":[\"http://www.w3.org/2002/07/owl#Ontology\"],\"http://www.w3.org/2002/07/owl#imports\":[{\"@id\":\"http://www.w3.org/ns/sosa/\"},{\"@id\":\"http://www.w3.org/ns/ssn/\"}]},{\"@id\":\"https://www.cs.uni-potsdam.de/bs/research/myno/sensordata#%name\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.w3.org/ns/sosa/Observation\"],\"http://www.w3.org/ns/sosa/hasFeatureOfInterest\":[{\"@id\":\"https://www.cs.uni-potsdam.de/bs/research/myno/sensordata#room21\"}],\"http://www.w3.org/ns/sosa/hasResult\":[{\"@id\":\"https://www.cs.uni-potsdam.de/bs/research/myno/sensordata#measureMovementResult\"}],\"http://www.w3.org/ns/sosa/hasSimpleResult\":[{\"@value\":\"Movement\"}],\"http://www.w3.org/ns/sosa/madeBySensor\":[{\"@id\":\"https://www.cs.uni-potsdam.de/bs/research/myno/sensordata#mymovementsensor\"}],\"http://www.w3.org/ns/sosa/observedProperty\":[{\"@id\":\"https://www.cs.uni-potsdam.de/bs/research/myno/sensordata#Movement\"}],\"http://www.w3.org/ns/sosa/resultTime\":[{\"@type\":\"http://www.w3.org/2001/XMLSchema#dateTime\",\"@value\":\"0000-01-01T00:00:00+00:00\"}],\"http://www.w3.org/ns/sosa/usedProcedure\":[{\"@id\":\"https://www.cs.uni-potsdam.de/bs/research/myno/sensordata#mymovementprocedure\"}]},{\"@id\":\"https://www.cs.uni-potsdam.de/bs/research/myno/sensordata#ESP32_%p\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.w3.org/ns/sosa/Platform\"],\"http://www.w3.org/ns/sosa/hosts\":[{\"@id\":\"https://www.cs.uni-potsdam.de/bs/research/myno/sensordata#mymovementsensor\"}]},{\"@id\":\"https://www.cs.uni-potsdam.de/bs/research/myno/sensordata#Movement\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.w3.org/ns/sosa/ObservableProperty\"],\"http://www.w3.org/ns/sosa/isObservedBy\":[{\"@id\":\"https://www.cs.uni-potsdam.de/bs/research/myno/sensordata#mymovementsensor\"}]},{\"@id\":\"https://www.cs.uni-potsdam.de/bs/research/myno/sensordata#ProcedureOutput\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.w3.org/ns/ssn/Output\"],\"http://www.w3.org/2000/01/rdf-schema#comment\":[{\"@value\":\"%s\"}]},{\"@id\":\"https://www.cs.uni-potsdam.de/bs/research/myno/sensordata#measureMovementResult\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.ontology-of-units-of-measure.org/resource/om-2/Measure\",\"http://www.w3.org/ns/sosa/Result\"],\"http://www.ontology-of-units-of-measure.org/resource/om-2/hasUnit\":[{\"@value\":\"http://www.ontology-of-units-of-measure.org/resource/om-2/one\"}],\"http://www.w3.org/ns/sosa/isResultOf\":[{\"@id\":\"https://www.cs.uni-potsdam.de/bs/research/myno/sensordata#%name\"}]},{\"@id\":\"https://www.cs.uni-potsdam.de/bs/research/myno/sensordata#mymovementprocedure\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.w3.org/ns/sosa/Procedure\"]},{\"@id\":\"https://www.cs.uni-potsdam.de/bs/research/myno/sensordata#mymovementsensor\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.w3.org/ns/sosa/Sensor\"],\"http://www.w3.org/ns/sosa/isHostedBy\":[{\"@id\":\"https://www.cs.uni-potsdam.de/bs/research/myno/sensordata#ESP32_%p\"}],\"http://www.w3.org/ns/sosa/madeObservation\":[{\"@id\":\"https://www.cs.uni-potsdam.de/bs/research/myno/sensordata#%name\"}],\"http://www.w3.org/ns/sosa/observes\":[{\"@id\":\"https://www.cs.uni-potsdam.de/bs/research/myno/sensordata#Movement\"}]},{\"@id\":\"https://www.cs.uni-potsdam.de/bs/research/myno/sensordata#room21\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.w3.org/ns/sosa/FeatureOfInterest\"],\"http://www.w3.org/ns/sosa/isFeatureOfInterestOf\":[{\"@id\":\"https://www.cs.uni-potsdam.de/bs/research/myno/sensordata#%name\"}]}]";

String ssn_actuation = "[{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#%output\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.w3.org/ns/ssn/Output\"],\"http://www.w3.org/2000/01/rdf-schema#comment\":[{\"@value\":\"%outresult\"},{\"@value\":\"%s\"}]},{\"@id\":\"http://www.ontology-of-units-of-measure.org/resource/om-2/Measure\",\"@type\":[\"http://www.w3.org/2002/07/owl#Class\"]},{\"@id\":\"http://www.ontology-of-units-of-measure.org/resource/om-2/hasUnit\",\"@type\":[\"http://www.w3.org/2002/07/owl#AnnotationProperty\"]},{\"@id\":\"https://www.cs.uni-potsdam.de/bs/research/myno/sensordata\",\"@type\":[\"http://www.w3.org/2002/07/owl#Ontology\"],\"http://www.w3.org/2002/07/owl#imports\":[{\"@id\":\"http://www.w3.org/ns/sosa/\"},{\"@id\":\"http://www.w3.org/ns/ssn/\"}]},{\"@id\":\"https://www.cs.uni-potsdam.de/bs/research/myno/sensordata#%actuation\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.w3.org/ns/sosa/Actuation\"],\"http://www.w3.org/ns/sosa/actsOnProperty\":[{\"@id\":\"https://www.cs.uni-potsdam.de/bs/research/myno/sensordata#ledLightEmission\"}],\"http://www.w3.org/ns/sosa/hasFeatureOfInterest\":[{\"@id\":\"https://www.cs.uni-potsdam.de/bs/research/myno/sensordata#room21\"}],\"http://www.w3.org/ns/sosa/hasResult\":[{\"@id\":\"https://www.cs.uni-potsdam.de/bs/research/myno/sensordata#%result\"}],\"http://www.w3.org/ns/sosa/hasSimpleResult\":[{\"@value\":\"%simple_result\"}],\"http://www.w3.org/ns/sosa/madeByActuator\":[{\"@id\":\"https://www.cs.uni-potsdam.de/bs/research/myno/sensordata#%actuator\"}],\"http://www.w3.org/ns/sosa/resultTime\":[{\"@type\":\"http://www.w3.org/2001/XMLSchema#dateTime\",\"@value\":\"0000-01-01T00:00:00+00:00\"}],\"http://www.w3.org/ns/sosa/usedProcedure\":[{\"@id\":\"https://www.cs.uni-potsdam.de/bs/research/myno/sensordata#myprocedure\"}]},{\"@id\":\"https://www.cs.uni-potsdam.de/bs/research/myno/sensordata#%actuator\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.w3.org/ns/sosa/Actuator\"],\"http://www.w3.org/ns/sosa/isHostedBy\":[{\"@id\":\"https://www.cs.uni-potsdam.de/bs/research/myno/sensordata#ESP32_%p\"}],\"http://www.w3.org/ns/sosa/madeActuation\":[{\"@id\":\"https://www.cs.uni-potsdam.de/bs/research/myno/sensordata#%actuation\"}]},{\"@id\":\"https://www.cs.uni-potsdam.de/bs/research/myno/sensordata#ESP32_%p\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.w3.org/ns/sosa/Platform\"],\"http://www.w3.org/ns/sosa/hosts\":[{\"@id\":\"https://www.cs.uni-potsdam.de/bs/research/myno/sensordata#%actuator\"}]},{\"@id\":\"https://www.cs.uni-potsdam.de/bs/research/myno/sensordata#%result\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.w3.org/ns/sosa/Result\"],\"http://www.w3.org/ns/sosa/isResultOf\":[{\"@id\":\"https://www.cs.uni-potsdam.de/bs/research/myno/sensordata#%actuation\"}]},{\"@id\":\"https://www.cs.uni-potsdam.de/bs/research/myno/sensordata#ledLightEmission\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.w3.org/ns/sosa/ActuatableProperty\"],\"http://www.w3.org/ns/sosa/isActedOnBy\":[{\"@id\":\"https://www.cs.uni-potsdam.de/bs/research/myno/sensordata#%actuation\"}]},{\"@id\":\"https://www.cs.uni-potsdam.de/bs/research/myno/sensordata#myprocedure\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.w3.org/ns/sosa/Procedure\"]},{\"@id\":\"https://www.cs.uni-potsdam.de/bs/research/myno/sensordata#room21\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.w3.org/ns/sosa/FeatureOfInterest\"],\"http://www.w3.org/ns/sosa/isFeatureOfInterestOf\":[{\"@id\":\"https://www.cs.uni-potsdam.de/bs/research/myno/sensordata#%actuation\"}]}]";
//Sensor/actuator ssn data:
long measure_id = 0;
long act_id = 0;
char act_output[30];
char act_result[30];
char act_outresult[7];
char act_actuation[30];
char observation_name[35];
char result_name[35];

// PubSubclient can only publish char-arrays, need this for conversion of ontology. Roughly the size of ontology file
bool device_isknown = false;
bool got_crud_response = false;
bool checkout_successful = false;
int tries = 0;
// DEEP SLEEP:
// saved in deep sleep
RTC_DATA_ATTR int bootCount = 0;

// Sensors
BH1750 brightness_1;
const int motion_1 = 13;
const int brightness_scl = 21;
const int brightness_sda = 22;

// Actuators
const int led_green = 18;
const int led_red = 17;


//Setup includes: Wifi and MQTT Conncection, Setting up Sensors, Publishing Device Description
void setup() {
  delay(200);
  Serial.begin(115200);
  if (bootCount == 0) {
    Serial.println("Starting Sketch");
  }
  ++bootCount;
  generate_config();
  setup_wifi();
  setup_mqtt();
  setup_sensors();
  setup_actuators();
  if (bootCount == 1) {
    //publish_config(CREATE_TOPIC_P);
    device_configuration("create");
  }
  else {
    Serial.println("Device woken up");
    Serial.print("Boot number: ");
    Serial.println(bootCount);
    //device_configuration("retrieve");
    //device_configuration("delete");
    device_configuration("update");
  }
}

//Connect to Wifi
void setup_wifi() {
  Serial.println();
  Serial.println("Attempting Wifi-Connection: ");
  delay(10);
  unsigned long starttime = millis();
  // We start by connecting to a WiFi network
  Serial.print("Connecting to: ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED && (millis() - starttime) < MINUTE) {
    delay(1000);
    Serial.print(".");
  }
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("");
    Serial.println("Unable to attain Wifi connection, going to sleep, press rst to start again.");
    esp_deep_sleep_start();
  }
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

//Connection to MQTT
void setup_mqtt() {
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  unsigned long starttime = millis();
  while (!client.connected() && (millis() - starttime) < MINUTE) {
    Serial.println();
    Serial.println("Attempting MQTT-Connection: ");
    // Attempt to connect
    if (client.connect(UUID)) {
      break;
    }
    else {
      Serial.print("failed, response code: ");
      Serial.print(client.state());
      Serial.println(" trying again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
  if (!client.connected()) {
    Serial.println("Unable to attain MQTT connection, going to sleep, press rst to start again.");
    esp_deep_sleep_start();
  }
  Serial.println("MQTT connected.");
  client.subscribe(LED1_TOPIC_S);
  Serial.println("Subscribed to: "LED1_TOPIC_S);
  client.subscribe(CREATE_TOPIC_S);
  client.subscribe(RETRIEVE_TOPIC_S);
  client.subscribe(UPDATE_TOPIC_S);
  client.subscribe(DELETE_TOPIC_S);
  Serial.println("Subscribed to CRUD Topics: "CRUD_TOPICS_S);
  client.subscribe(PING_TOPIC_S);
  Serial.println("Subscribed to PING Topic: "PING_TOPIC_S);
}
void device_configuration(char* operation) {
  // OPERATION: CREATE; RETRIEVE; UPDATE; DELETE
  // FIRST IS ALWAYS RETRIEVE
  device_isknown = false;
  client.publish(RETRIEVE_TOPIC_P, UUID);
  crud_response_wait("retrieve");
  if (strcmp(operation, "create") == 0) {
    if (device_isknown) {
      Serial.println("Aborting Configuration Publication. Device already known. Updating instead.");
      publish_config(UPDATE_TOPIC_P);
      crud_response_wait("update");
    }
    else {
      publish_config(CREATE_TOPIC_P);
      crud_response_wait("create");
    }
  }
  else if (strcmp(operation, "retrieve") == 0) {
    // Read is performed first
  }
  else if (strcmp(operation, "update") == 0) {
    if (device_isknown) {
      publish_config(UPDATE_TOPIC_P);
      crud_response_wait("update");
    }
    else {
      Serial.println("Device unknown, publishing config instead");
      publish_config(CREATE_TOPIC_P);
      crud_response_wait("create");
    }
  }
  else if (strcmp(operation, "delete") == 0) {
    if (device_isknown) {
      client.publish(DELETE_TOPIC_P, UUID);
      crud_response_wait("delete");
    }
    else {
      Serial.println("Device unknown, deletion impossible");
    }
  }
}
void crud_response_wait(char* Operation) {
  got_crud_response = false;
  unsigned long starttime = millis();
  while (tries < 5) {
    while (!got_crud_response && (millis() - starttime) < MINUTE) {
      if (!client.connected()) {
        if (WiFi.status() != WL_CONNECTED) {
          setup_wifi();
        }
        setup_mqtt();
        tries = 0;
        device_configuration(Operation);
      }
      Serial.print("Waiting for CRUD-Reponse for: ");
      Serial.println(Operation);
      delay(1000);
      client.loop();
    }
    if (got_crud_response) {
      return;
    }
    else {
      tries++;
      device_configuration(Operation);
    }
  }
  char *msg;
  sprintf(msg, UUID" %s", Operation);
  client.publish("error", msg);
}
void generate_config() {
  // Replace %s with device uuid and concatenate device uuid and ontology
  ontology.replace("%s", UUID);
  // String(device_uuid+";"+ontology).toCharArray(configuration,sizeof(configuration));
}

// Publication of Device Description
void publish_config(char* topic) {
  Serial.println();
  Serial.println("Publishing Device Description: ");
  client.publish(topic, String(device_uuid + ";" + ontology).c_str());

  // Create END-Message for configuration and send
  //String(device_uuid+";END").toCharArray(configuration_end,sizeof(configuration_end));
  client.publish(topic, UUID";END");
  Serial.println("Ontology published.");
}
void setup_sensors() {
  // BH1750: brigthness_1
  // Wire.begin takes two parameters: SDA pin and SCL pin;
  Wire.begin(brightness_sda, brightness_scl);
  brightness_1.begin(BH1750::CONTINUOUS_HIGH_RES_MODE_2);
  Serial.println(F("BH1750 High Resolution Mode"));
  // PIR: motion_1
  pinMode(motion_1, INPUT);
  Serial.println("Motion Sensor");

}
void out_brightness_1() {
  char lux_msg[15];
  float lux =  brightness_1.readLightLevel();
  //dtostrf(lux, 5, 2, lux_char);
  if (lux < 0.0) {
    sprintf(lux_msg, "Error");
    client.publish(SENSOR_ERROR_TOPIC_P, UUID);

  }
  else {
    //dtostrf(lux, 5, 2, lux_char);
    sprintf(lux_msg, "%.1f", lux);
    sprintf(observation_name, "myObservation-bright-%li", measure_id);
    sprintf(result_name, "myResult-bright-%li", measure_id);
    String ssn_brightness_copy = ssn_brightness;
    ssn_brightness_copy.replace("%name", observation_name);
    ssn_brightness_copy.replace("%result", result_name);
    ssn_brightness_copy.replace("-0.1", lux_msg);
    //ssn_brightness_copy.replace("0000-01-01T00:00:00+00:00","2019-08-12T09:15:02+02:00");
    ssn_brightness_copy.replace("%s", UUID);
    ssn_brightness_copy.replace("%p", BOARD);

    client.publish(BRIGHT1_SSN_P, ssn_brightness_copy.c_str());
    sprintf(lux_msg, "%.1f lux", lux);
    client.publish(BRIGHT1_TOPIC_P, lux_msg);
  }
}

void out_motion_1() {
  char motion_msg[12];
  if (millis() < 60000) {
    sprintf(motion_msg, "Calibrating");
  }
  else {
    bool isMotion = digitalRead(motion_1);
    if (isMotion) {
      sprintf(motion_msg, "Motion");
      String ssn_motion_copy = ssn_motion;
      sprintf(observation_name, "myObservation-motion-%li", measure_id);
      sprintf(result_name, "myResult-motion-%li", measure_id);
      ssn_motion_copy.replace("%name", observation_name);
      ssn_motion_copy.replace("%result", result_name);
      ssn_motion_copy.replace("-0.1", motion_msg);
      //ssn_motion_copy.replace("0000-01-01T00:00:00+00:00","2019-08-12T09:15:02+02:00");
      ssn_motion_copy.replace("%s", UUID);
      ssn_motion_copy.replace("%p", BOARD);

      client.publish(MOTION1_SSN_P, ssn_motion_copy.c_str());
    }
    else {
      sprintf(motion_msg, "No Motion");
    }
  }
  client.publish(MOTION1_TOPIC_P, motion_msg);
}


void setup_actuators() {
  pinMode(led_green, OUTPUT);
  pinMode(led_red, OUTPUT);
}
// Callback function used for subscribed topics
void callback(char* topic, byte* message_bytes, unsigned int length) {
  // Retrieve message and commands + parameters
  String message;
  for (int i = 0; i < length; i++) {
    message += (char)message_bytes[i];
  }
  //Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  //Serial.print(". Message: ");
  Serial.println(message);
  // Retrieve parts of message: For RPC seq_id is a message id, otherwise the first part of the message is the command.
  char *seq_id, *cmd, *cmd_param, *response;
  const char delim[] = ";";
  seq_id = strtok((char*)message.c_str(), delim);
  Serial.print("Message id: ");
  Serial.println(seq_id);
  cmd = strtok(NULL, delim);
  if (cmd == NULL) {
    cmd = seq_id;
    seq_id = "none";
  }
  Serial.print("Command: ");
  Serial.println(cmd);
  cmd_param = strtok(NULL, delim);
  if (cmd_param == NULL) {
    cmd_param = "none";
  }
  Serial.print("Command Parameter: ");
  Serial.println(cmd_param);

  // DIFFERENT TOPICS
  if (strcmp(topic, CREATE_TOPIC_S) == 0) {
    create_react(cmd);
  }
  else if (strcmp(topic, RETRIEVE_TOPIC_S) == 0) {
    read_react(cmd);
  }
  else if (strcmp(topic, UPDATE_TOPIC_S) == 0) {
    update_react(cmd);
  }
  else if (strcmp(topic, DELETE_TOPIC_S) == 0) {
    delete_react(cmd);
  }
  else if (strcmp(topic, PING_TOPIC_S) == 0) {
    Serial.println("Got ping request");
    ping_response();
  }

  else if (strstr(topic, "led_1") != NULL) {
    Serial.println("Command for LED1");
    response = led_1_response(cmd, cmd_param, seq_id);
    publish_response(RESPONSE_TOPIC_P, response);
  }
  else {
    Serial.println("Topic unknown");
    sprintf(response, "%s;ERROR", seq_id);
    publish_response(RESPONSE_TOPIC_P, response);

  }
}
void ping_response() {
  client.publish(PING_TOPIC_P, CRUD_OK);
}

void create_react(char* cmd) {
  if (strcmp(cmd, CRUD_OK) == 0 ) {
    Serial.println("Device config successfully sent");
  }
  else {
    Serial.println("Error sending device config");
  }
  got_crud_response = true;
}
void read_react(char* cmd) {
  if (strcmp(cmd, CRUD_OK) == 0 ) {
    Serial.println("Device config known");
    device_isknown = true;
  }
  else {
    Serial.println("Device config not known");
  }
  got_crud_response = true;
}
void delete_react(char* cmd) {
  if (strcmp(cmd, CRUD_OK) == 0 ) {
    Serial.println("Device config deletion successful");
    checkout_successful = true;
  }
  else {
    Serial.println("Device config deletion not successful");
  }
  got_crud_response = true;
}
void update_react(char* cmd) {
  if (strcmp(cmd, CRUD_OK) == 0 ) {
    Serial.println("Device config update successful known");
  }
  else {
    Serial.println("Device config update not successful");
  }
  got_crud_response = true;
}



// Changes in LED1: Input command and command parameters and message id all from mqtt message, output: response message
char* led_1_response(char* command, char* command_param, char* seq_id) {
  static char reply[20];
  if (strcmp(command, "ON") == 0) {
    Serial.println("Command ON");
    if (digitalRead(led_red) == HIGH && digitalRead(led_green) == HIGH) {
      sprintf(reply, "%s;NOOP", seq_id);
      sprintf(act_outresult, "NOOP");
    }
    else if ( (digitalRead(led_red) == LOW && digitalRead(led_green) == LOW) || (digitalRead(led_red) == HIGH && digitalRead(led_green) == LOW) || (digitalRead(led_red) == LOW && digitalRead(led_green) == HIGH)) {
      digitalWrite(led_red, HIGH);
      digitalWrite(led_green, HIGH);
      sprintf(reply, "%s;OK", seq_id);
      sprintf(act_outresult, "OK");
    }
    else {
      sprintf(reply, "%s;ERROR", seq_id);
      sprintf(act_outresult, "ERROR");
    }
  }
  else if (strcmp(command, "OFF") == 0) {
    Serial.println("Command OFF");
    if (digitalRead(led_red) == LOW && digitalRead(led_green) == LOW) {
      sprintf(reply, "%s;NOOP", seq_id);
      sprintf(act_outresult, "NOOP");
    }
    else if ( (digitalRead(led_red) == HIGH && digitalRead(led_green) == HIGH) || (digitalRead(led_red) == HIGH && digitalRead(led_green) == LOW) || (digitalRead(led_red) == LOW && digitalRead(led_green) == HIGH)) {
      digitalWrite(led_red, LOW);
      digitalWrite(led_green, LOW);
      sprintf(reply, "%s;OK", seq_id);
      sprintf(act_outresult, "OK");
    }
    else {
      sprintf(reply, "%s;ERROR", seq_id);
      sprintf(act_outresult, "ERROR");
    }
  }
  else if (strcmp(command, "color") == 0) {
    Serial.println("Topic correct, command color");
    if (strstr(command_param, "all") != NULL) {
      if (digitalRead(led_red) == HIGH && digitalRead(led_green) == HIGH) {
        sprintf(reply, "%s;NOOP", seq_id);
        sprintf(act_outresult, "NOOP");
      }
      else if ( (digitalRead(led_red) == LOW && digitalRead(led_green) == LOW) || (digitalRead(led_red) == HIGH && digitalRead(led_green) == LOW) || (digitalRead(led_red) == LOW && digitalRead(led_green) == HIGH)) {
        digitalWrite(led_red, HIGH);
        digitalWrite(led_green, HIGH);
        sprintf(reply, "%s;OK", seq_id);
        sprintf(act_outresult, "OK");
      }
      else {
        sprintf(reply, "%s;ERROR", seq_id);
        sprintf(act_outresult, "ERROR");
      }
    }
    else if (strstr(command_param, "green") != NULL) {
      if (digitalRead(led_red) == LOW && digitalRead(led_green) == HIGH) {
        sprintf(reply, "%s;NOOP", seq_id);
        sprintf(act_outresult, "NOOP");
      }
      else if ( (digitalRead(led_red) == HIGH && digitalRead(led_green) == LOW) || (digitalRead(led_red) == HIGH && digitalRead(led_green) == HIGH) || (digitalRead(led_red) == LOW && digitalRead(led_green) == LOW)) {
        digitalWrite(led_red, LOW);
        digitalWrite(led_green, HIGH);
        sprintf(reply, "%s;OK", seq_id);
        sprintf(act_outresult, "OK");
      }
      else {
        sprintf(reply, "%s;ERROR", seq_id);
        sprintf(act_outresult, "ERROR");
      }
    }
    else if (strstr(command_param, "red") != NULL) {
      if (digitalRead(led_red) == HIGH && digitalRead(led_green) == LOW) {
        sprintf(reply, "%s;NOOP", seq_id);
        sprintf(act_outresult, "NOOP");
      }
      else if ( (digitalRead(led_red) == LOW && digitalRead(led_green) == HIGH) || (digitalRead(led_red) == HIGH && digitalRead(led_green) == HIGH) || (digitalRead(led_red) == LOW && digitalRead(led_green) == LOW)) {
        digitalWrite(led_red, HIGH);
        digitalWrite(led_green, LOW);
        sprintf(reply, "%s;OK", seq_id);
        sprintf(act_outresult, "OK");
      }
      else {
        sprintf(reply, "%s;ERROR", seq_id);
        sprintf(act_outresult, "ERROR");
      }
    }
  }
  else {
    Serial.println("Topic known, command uknown");
    sprintf(reply, "%s;ERROR", seq_id);
  }
  String ssn_actuation_copy = ssn_actuation;
  sprintf(act_actuation, "%s-%s-%li", command, command_param, seq_id);
  sprintf(act_result, "result-%li", seq_id);
  sprintf(act_output, "output-%li", seq_id);
  ssn_actuation_copy.replace("%output", act_output);
  ssn_actuation_copy.replace("%result", act_result);
  ssn_actuation_copy.replace("%actuator", "led_2");
  //ssn_actuation_copy.replace("0000-01-01T00:00:00+00:00","2019-08-12T09:15:02+02:00");
  ssn_actuation_copy.replace("%s", UUID);
  ssn_actuation_copy.replace("%outresult", act_outresult);
  ssn_actuation_copy.replace("%p", BOARD);
  client.publish(LED1_SSN_P, ssn_actuation_copy.c_str());
  return reply;
}

void publish_response(char* topic, char* message) {
  Serial.println(message);
  client.publish(topic, message);
}

void to_sleep() {
  Serial.println("Attempting Deletion before deep sleep: Wakeup by rst");
  device_configuration("delete");
  if (checkout_successful == true) {
    checkout_successful = false;
    Serial.println("Checkout successful, going to sleep");
    esp_deep_sleep_start();
  }
  else {
    Serial.println("Checkout unsuccessful, sent error to mqtt, going to sleep anyways");
    client.publish("error", UUID"; Error deleting configuration");
    esp_deep_sleep_start();
  }
}

void loop() {
  if (!client.connected()) {
    Serial.println("Lost MQTT-connection. Reconnecting");
    if ( WiFi.status() != WL_CONNECTED) {
      Serial.println("Lost Wifi-Connection as well. Reconnecting");
      setup_wifi();
    }
    setup_mqtt();
  }
  client.loop();
  out_brightness_1();
  out_motion_1();
  measure_id++;
  Serial.println(measure_id);
  delay(500);
  if (measure_id > 20) {
    //to_sleep();
  }
}
