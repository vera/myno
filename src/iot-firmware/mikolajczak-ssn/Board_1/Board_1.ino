#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <BH1750.h>
#include <string.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

// DEFINITIONS_
// OVERALL
#define UUID "BOARD4-8A12-4F4F-8F69-6B8F3C2E78ED"
#define BOARD "PrecAgr_pb"
//timing powerbank
#define HELLO_TOPIC_P "hello/"UUID

// SUBSCRIBED TOPICS
#define LED1_TOPIC_S "actuator/led/led_1/"UUID
#define PUMP1_TOPIC_S "actuator/pump/pump_1/"UUID
#define PING_TOPIC_S "ping/yang/config/"UUID
#define CRUD_TOPICS_S CONFIG_TOPIC_P"/+/response/"UUID
#define CREATE_TOPIC_S CONFIG_TOPIC_P"/create/response/"UUID
#define RETRIEVE_TOPIC_S CONFIG_TOPIC_P"/retrieve/response/"UUID
#define UPDATE_TOPIC_S CONFIG_TOPIC_P"/update/response/"UUID
#define DELETE_TOPIC_S CONFIG_TOPIC_P"/delete/response/"UUID

// PUBLISHING TOPICS
#define RESPONSE_TOPIC_P "response/"UUID
#define BRIGHT1_TOPIC_P "sensor/brightness/brightness_1/"UUID
#define TEMP1_TOPIC_P "sensor/temperature/temperature_1/"UUID
#define HUM1_TOPIC_P "sensor/humidity/humidity_1/"UUID
#define PRES1_TOPIC_P "sensor/pressure/pressure_1/"UUID
#define RAIN1_TOPIC_P "sensor/rain/rain_1/"UUID
#define MOIST1_TOPIC_P "sensor/moisture/moisture_1/"UUID

#define CONFIG_TOPIC_P "yang/config"
#define PING_TOPIC_P CONFIG_TOPIC_P"/ping/response"
#define CREATE_TOPIC_P CONFIG_TOPIC_P"/create"
#define RETRIEVE_TOPIC_P CONFIG_TOPIC_P"/retrieve"
#define UPDATE_TOPIC_P CONFIG_TOPIC_P"/update"
#define DELETE_TOPIC_P CONFIG_TOPIC_P"/delete"

// CRUD RESPONSES
#define CRUD_OK "OK"
#define CRUD_NOTOK "NOTFOUND" // not used

//General ERROR TOPIC
#define ERROR_P "error"

// SLEEP
#define MINUTE 6000
 
// Thresholds for Analog Sensors
#define RAIN_THRESH 2000
#define RAIN_SIGNAL_THRESH 1000
#define MOISTURE_SIGNAL_THRESH 500
#define MOISTURE_MOIST_THRESH 3200
#define MOISTURE_WET_THRESH 2600

// Timing 
unsigned long raintime;
unsigned long pingtime

//Wifi & MQTT Uni
//const char* ssid = "IoTSens";
//const char* password =  "heterogenIstAnstrengend!";
//const char* mqtt_server = "192.168.43.141";
// für bugfixing
const char* ssid = "FRITZ!Box 7590 LL";
const char* password =  "36122480101678560091";
const char* mqtt_server = "192.168.178.41";


WiFiClient Board1;
PubSubClient client(Board1);

// Ontology & Device UUID
// Device UUID and ontology with placeholder "%s" for UUID -> like Michael Nowak
String device_uuid = UUID;

//String ontology = "[{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#Command\",\"@type\":[\"http://www.w3.org/2002/07/owl#Class\"]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#ControllingFunctionality\",\"@type\":[\"http://www.w3.org/2002/07/owl#Class\"]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#Device\",\"@type\":[\"http://www.w3.org/2002/07/owl#Class\"]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#MeasuringFunctionality\",\"@type\":[\"http://www.w3.org/2002/07/owl#Class\"]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#Operation\",\"@type\":[\"http://www.w3.org/2002/07/owl#Class\"]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#OperationInput\",\"@type\":[\"http://www.w3.org/2002/07/owl#Class\"]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#OperationOutput\",\"@type\":[\"http://www.w3.org/2002/07/owl#Class\"]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#OperationState\",\"@type\":[\"http://www.w3.org/2002/07/owl#Class\"]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#OutputDataPoint\",\"@type\":[\"http://www.w3.org/2002/07/owl#Class\"]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#Service\",\"@type\":[\"http://www.w3.org/2002/07/owl#Class\"]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#ThingProperty\",\"@type\":[\"http://www.w3.org/2002/07/owl#Class\"]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#Variable\",\"@type\":[\"http://www.w3.org/2002/07/owl#Class\"]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#exposesCommand\",\"@type\":[\"http://www.w3.org/2002/07/owl#ObjectProperty\"]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#exposesFunctionality\",\"@type\":[\"http://www.w3.org/2002/07/owl#AnnotationProperty\"]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#funcDescHumidity\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://yang-netconf-mqtt#YangDescription\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasValue\":[{\"@value\":\"Gethumidityfromsensor\"}]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#funcDescMoisture\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://yang-netconf-mqtt#YangDescription\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasValue\":[{\"@value\":\"Getmoisturefromsensor\"}]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#funcDescPressure\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://yang-netconf-mqtt#YangDescription\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasValue\":[{\"@value\":\"Getpressurefromsensor\"}]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#funcDescRainDetect\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://yang-netconf-mqtt#YangDescription\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasValue\":[{\"@value\":\"Getraindetectionsignalfromsensor\"}]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#funcDescTemperature\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://yang-netconf-mqtt#YangDescription\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasValue\":[{\"@value\":\"GetTemperaturefromsensor\"}]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#funcGetHumidity\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#MeasuringFunctionality\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasThingProperty\":[{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#funcDescHumidity\"}]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#funcGetMoisture\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#MeasuringFunctionality\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasThingProperty\":[{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#funcDescMoisture\"}]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#funcGetPressure\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#MeasuringFunctionality\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasThingProperty\":[{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#funcDescPressure\"}]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#funcGetRainDetect\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#MeasuringFunctionality\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasThingProperty\":[{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#funcDescRainDetect\"}]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#funcGetTemperature\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#MeasuringFunctionality\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasThingProperty\":[{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#funcDescTemperature\"}]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasCommand\",\"@type\":[\"http://www.w3.org/2002/07/owl#ObjectProperty\"]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasDataRestriction_maxInclusive\",\"@type\":[\"http://www.w3.org/2002/07/owl#AnnotationProperty\"]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasDataRestriction_minInclusive\",\"@type\":[\"http://www.w3.org/2002/07/owl#AnnotationProperty\"]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasDataRestriction_pattern\",\"@type\":[\"http://www.w3.org/2002/07/owl#DatatypeProperty\"]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasFunctionality\",\"@type\":[\"http://www.w3.org/2002/07/owl#AnnotationProperty\"]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasInput\",\"@type\":[\"http://www.w3.org/2002/07/owl#ObjectProperty\"],\"http://www.w3.org/2000/01/rdf-schema#range\":[{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#ThingProperty\"}]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasOperation\",\"@type\":[\"http://www.w3.org/2002/07/owl#ObjectProperty\"]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasOperationState\",\"@type\":[\"http://www.w3.org/2002/07/owl#ObjectProperty\"]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasOutput\",\"@type\":[\"http://www.w3.org/2002/07/owl#ObjectProperty\"]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasOutputDataPoint\",\"@type\":[\"http://www.w3.org/2002/07/owl#ObjectProperty\"]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasService\",\"@type\":[\"http://www.w3.org/2002/07/owl#ObjectProperty\"]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasSubService\",\"@type\":[\"http://www.w3.org/2002/07/owl#ObjectProperty\"]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasSubStructure\",\"@type\":[\"http://www.w3.org/2002/07/owl#AnnotationProperty\"]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasThingProperty\",\"@type\":[\"http://www.w3.org/2002/07/owl#ObjectProperty\"]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasValue\",\"@type\":[\"http://www.w3.org/2002/07/owl#DatatypeProperty\"]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#outDpHumidity\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#OutputDataPoint\"],\"http://yang-netconf-mqtt#mqttTopic\":[{\"@value\":\"sensor/humidity/humidity_1/%s\"}]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#outDpMoisture\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#OutputDataPoint\"],\"http://yang-netconf-mqtt#mqttTopic\":[{\"@value\":\"sensor/moisture/moisture_1/%s\"}]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#outDpPressure\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#OutputDataPoint\"],\"http://yang-netconf-mqtt#mqttTopic\":[{\"@value\":\"sensor/pressure/pressure_1/%s\"}]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#outDpRainDetect\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#OutputDataPoint\"],\"http://yang-netconf-mqtt#mqttTopic\":[{\"@value\":\"sensor/rain/rain_1/%s\"}]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#outDpTemperature\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#OutputDataPoint\"],\"http://yang-netconf-mqtt#mqttTopic\":[{\"@value\":\"sensor/temperature/temperature_1/%s\"}]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#servHumidity\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#Service\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#exposesFunctionality\":[{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#funcGetHumidity\"}],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasOutputDataPoint\":[{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#outDpHumidity\"}]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#servMoisture\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#Service\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#exposesFunctionality\":[{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#funcGetMoisture\"}],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasOutputDataPoint\":[{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#outDpMoisture\"}]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#servPressure\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#Service\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#exposesFunctionality\":[{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#funcGetPressure\"}],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasOutputDataPoint\":[{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#outDpPressure\"}]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#servRainDetect\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#Service\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#exposesFunctionality\":[{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#funcGetRainDetect\"}],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasOutputDataPoint\":[{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#outDpRainDetect\"}]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#servTemperature\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#Service\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#exposesFunctionality\":[{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#funcGetTemperature\"}],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasOutputDataPoint\":[{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#outDpTemperature\"}]},{\"@id\":\"http://yang-netconf-mqtt\",\"@type\":[\"http://www.w3.org/2002/07/owl#Ontology\"],\"http://www.w3.org/2002/07/owl#imports\":[{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology-v0_9_0\"}]},{\"@id\":\"http://yang-netconf-mqtt#YangDescription\",\"@type\":[\"http://www.w3.org/2002/07/owl#Class\"],\"http://www.w3.org/2000/01/rdf-schema#subClassOf\":[{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#ThingProperty\"}]},{\"@id\":\"http://yang-netconf-mqtt#cmdLed_1Off\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#Command\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasInput\":[{\"@id\":\"http://yang-netconf-mqtt#uuidInput\"}]},{\"@id\":\"http://yang-netconf-mqtt#cmdLed_1On\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#Command\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasInput\":[{\"@id\":\"http://yang-netconf-mqtt#uuidInput\"}]},{\"@id\":\"http://yang-netconf-mqtt#cmdLed_1Rgb\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#Command\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasInput\":[{\"@id\":\"http://yang-netconf-mqtt#rgbinput\"},{\"@id\":\"http://yang-netconf-mqtt#uuidInput\"}]},{\"@id\":\"http://yang-netconf-mqtt#deviceCategory\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#ThingProperty\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasValue\":[{\"@value\":\"%BoardName\"}]},{\"@id\":\"http://yang-netconf-mqtt#deviceDesc\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://yang-netconf-mqtt#YangDescription\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasValue\":[{\"@value\":\"MQTT-DeviceidentifiedbyUUID\"}]},{\"@id\":\"http://yang-netconf-mqtt#deviceUuid\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#ThingProperty\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasValue\":[{\"@value\":\"%s\"}]},{\"@id\":\"http://yang-netconf-mqtt#funcDescBrightness\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://yang-netconf-mqtt#YangDescription\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasValue\":[{\"@value\":\"Getbrightnessfromsensor\"}]},{\"@id\":\"http://yang-netconf-mqtt#funcDescLed_1Off\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://yang-netconf-mqtt#YangDescription\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasValue\":[{\"@value\":\"turnled1off\"}]},{\"@id\":\"http://yang-netconf-mqtt#funcDescLed_1On\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://yang-netconf-mqtt#YangDescription\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasValue\":[{\"@value\":\"turnled1on\"}]},{\"@id\":\"http://yang-netconf-mqtt#funcDescLed_1Rgb\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://yang-netconf-mqtt#YangDescription\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasValue\":[{\"@value\":\"setRGBvaluesforled1\"}]},{\"@id\":\"http://yang-netconf-mqtt#funcGetBrightness\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#MeasuringFunctionality\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasThingProperty\":[{\"@id\":\"http://yang-netconf-mqtt#funcDescBrightness\"}]},{\"@id\":\"http://yang-netconf-mqtt#funcLed_1Off\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#ControllingFunctionality\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasCommand\":[{\"@id\":\"http://yang-netconf-mqtt#cmdLed_1Off\"}],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasThingProperty\":[{\"@id\":\"http://yang-netconf-mqtt#funcDescLed_1Off\"}]},{\"@id\":\"http://yang-netconf-mqtt#funcLed_1On\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#ControllingFunctionality\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasCommand\":[{\"@id\":\"http://yang-netconf-mqtt#cmdLed_1On\"}],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasThingProperty\":[{\"@id\":\"http://yang-netconf-mqtt#funcDescLed_1On\"}]},{\"@id\":\"http://yang-netconf-mqtt#funcLed_1Rgb\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#ControllingFunctionality\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasCommand\":[{\"@id\":\"http://yang-netconf-mqtt#cmdLed_1Rgb\"}],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasThingProperty\":[{\"@id\":\"http://yang-netconf-mqtt#funcDescLed_1Rgb\"}]},{\"@id\":\"http://yang-netconf-mqtt#inputBlue\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#Variable\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasDataRestriction_maxInclusive\":[{\"@type\":\"http://www.w3.org/2001/XMLSchema#int\",\"@value\":\"255\"}],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasDataRestriction_minInclusive\":[{\"@type\":\"http://www.w3.org/2001/XMLSchema#int\",\"@value\":\"0\"}]},{\"@id\":\"http://yang-netconf-mqtt#inputGreen\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#Variable\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasDataRestriction_maxInclusive\":[{\"@type\":\"http://www.w3.org/2001/XMLSchema#int\",\"@value\":\"255\"}],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasDataRestriction_minInclusive\":[{\"@type\":\"http://www.w3.org/2001/XMLSchema#int\",\"@value\":\"0\"}]},{\"@id\":\"http://yang-netconf-mqtt#inputRed\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#Variable\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasDataRestriction_maxInclusive\":[{\"@type\":\"http://www.w3.org/2001/XMLSchema#int\",\"@value\":\"255\"}],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasDataRestriction_minInclusive\":[{\"@type\":\"http://www.w3.org/2001/XMLSchema#int\",\"@value\":\"0\"}]},{\"@id\":\"http://yang-netconf-mqtt#mqttMethod\",\"@type\":[\"http://www.w3.org/2002/07/owl#DatatypeProperty\"]},{\"@id\":\"http://yang-netconf-mqtt#mqttTopic\",\"@type\":[\"http://www.w3.org/2002/07/owl#DatatypeProperty\"]},{\"@id\":\"http://yang-netconf-mqtt#myDevice\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#Device\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasFunctionality\":[{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#funcGetHumidity\"},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#funcGetMoisture\"},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#funcGetPressure\"},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#funcGetRainDetect\"},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#funcGetTemperature\"},{\"@id\":\"http://yang-netconf-mqtt#funcGetBrightness\"},{\"@id\":\"http://yang-netconf-mqtt#funcLed_1Off\"},{\"@id\":\"http://yang-netconf-mqtt#funcLed_1On\"},{\"@id\":\"http://yang-netconf-mqtt#funcLed_1Rgb\"}],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasService\":[{\"@id\":\"http://yang-netconf-mqtt#servNetconf\"}],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasThingProperty\":[{\"@id\":\"http://yang-netconf-mqtt#deviceCategory\"},{\"@id\":\"http://yang-netconf-mqtt#deviceDesc\"},{\"@id\":\"http://yang-netconf-mqtt#deviceUuid\"}]},{\"@id\":\"http://yang-netconf-mqtt#opDescState\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://yang-netconf-mqtt#YangDescription\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasDataRestriction_pattern\":[{\"@value\":\"error\"},{\"@value\":\"nothingtodo\"},{\"@value\":\"successful\"}]},{\"@id\":\"http://yang-netconf-mqtt#opMqttLed_1Off\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#Operation\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#exposesCommand\":[{\"@id\":\"http://yang-netconf-mqtt#cmdLed_1Off\"}],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasInput\":[{\"@id\":\"http://yang-netconf-mqtt#uuidInput\"}],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasOperationState\":[{\"@id\":\"http://yang-netconf-mqtt#opState\"}],\"http://yang-netconf-mqtt#mqttMethod\":[{\"@value\":\"OFF\"}],\"http://yang-netconf-mqtt#mqttTopic\":[{\"@value\":\"actuator/led/led_1\"}]},{\"@id\":\"http://yang-netconf-mqtt#opMqttLed_1On\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#Operation\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#exposesCommand\":[{\"@id\":\"http://yang-netconf-mqtt#cmdLed_1On\"}],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasInput\":[{\"@id\":\"http://yang-netconf-mqtt#uuidInput\"}],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasOperationState\":[{\"@id\":\"http://yang-netconf-mqtt#opState\"}],\"http://yang-netconf-mqtt#mqttMethod\":[{\"@value\":\"ON\"}],\"http://yang-netconf-mqtt#mqttTopic\":[{\"@value\":\"actuator/led/led_1\"}]},{\"@id\":\"http://yang-netconf-mqtt#opMqttLed_1Rgb\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#Operation\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#exposesCommand\":[{\"@id\":\"http://yang-netconf-mqtt#cmdLed_1Rgb\"}],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasInput\":[{\"@id\":\"http://yang-netconf-mqtt#rgbinput\"},{\"@id\":\"http://yang-netconf-mqtt#uuidInput\"}],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasOperationState\":[{\"@id\":\"http://yang-netconf-mqtt#opState\"}],\"http://yang-netconf-mqtt#mqttMethod\":[{\"@value\":\"rgb\"}],\"http://yang-netconf-mqtt#mqttTopic\":[{\"@value\":\"actuator/led/led_1\"}]},{\"@id\":\"http://yang-netconf-mqtt#opState\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#OperationState\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasDataRestriction_pattern\":[{\"@value\":\"ERROR\"},{\"@value\":\"NOOP\"},{\"@value\":\"OK\"}],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasThingProperty\":[{\"@id\":\"http://yang-netconf-mqtt#opDescState\"}]},{\"@id\":\"http://yang-netconf-mqtt#outDpBrightness\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#OutputDataPoint\"],\"http://yang-netconf-mqtt#mqttTopic\":[{\"@value\":\"sensor/brightness/brightness_1/%s\"}]},{\"@id\":\"http://yang-netconf-mqtt#rgbYangDesc\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://yang-netconf-mqtt#YangDescription\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasValue\":[{\"@value\":\"RGBparameterforled_1\"}]},{\"@id\":\"http://yang-netconf-mqtt#rgbinput\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#OperationInput\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasSubStructure\":[{\"@id\":\"http://yang-netconf-mqtt#inputBlue\"},{\"@id\":\"http://yang-netconf-mqtt#inputGreen\"},{\"@id\":\"http://yang-netconf-mqtt#inputRed\"}],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasThingProperty\":[{\"@id\":\"http://yang-netconf-mqtt#rgbYangDesc\"}]},{\"@id\":\"http://yang-netconf-mqtt#servBrightness\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#Service\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#exposesFunctionality\":[{\"@id\":\"http://yang-netconf-mqtt#funcGetBrightness\"}],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasOutputDataPoint\":[{\"@id\":\"http://yang-netconf-mqtt#outDpBrightness\"}]},{\"@id\":\"http://yang-netconf-mqtt#servNetconf\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#Service\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#exposesFunctionality\":[{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#funcGetHumidity\"},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#funcGetMoisture\"},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#funcGetPressure\"},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#funcGetRainDetect\"},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#funcGetTemperature\"},{\"@id\":\"http://yang-netconf-mqtt#funcGetBrightness\"},{\"@id\":\"http://yang-netconf-mqtt#funcLed_1Off\"},{\"@id\":\"http://yang-netconf-mqtt#funcLed_1On\"},{\"@id\":\"http://yang-netconf-mqtt#funcLed_1Rgb\"}],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasOperation\":[{\"@id\":\"http://yang-netconf-mqtt#opMqttLed_1Off\"},{\"@id\":\"http://yang-netconf-mqtt#opMqttLed_1On\"},{\"@id\":\"http://yang-netconf-mqtt#opMqttLed_1Rgb\"}],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasSubService\":[{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#servHumidity\"},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#servMoisture\"},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#servPressure\"},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#servRainDetect\"},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#servTemperature\"},{\"@id\":\"http://yang-netconf-mqtt#servBrightness\"}]},{\"@id\":\"http://yang-netconf-mqtt#uuidInput\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#OperationInput\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasInput\":[{\"@id\":\"http://yang-netconf-mqtt#deviceUuid\"}],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasThingProperty\":[{\"@id\":\"http://yang-netconf-mqtt#uuidYangDesc\"}],\"http://yang-netconf-mqtt#mqttMethod\":[{\"@value\":\"OFF\"}]},{\"@id\":\"http://yang-netconf-mqtt#uuidYangDesc\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://yang-netconf-mqtt#YangDescription\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasValue\":[{\"@value\":\"TargetUUIDforrequest\"}]}]";
//String ontology ="[{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#Command\",\"@type\":[\"http://www.w3.org/2002/07/owl#Class\"]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#ControllingFunctionality\",\"@type\":[\"http://www.w3.org/2002/07/owl#Class\"]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#Device\",\"@type\":[\"http://www.w3.org/2002/07/owl#Class\"]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#MeasuringFunctionality\",\"@type\":[\"http://www.w3.org/2002/07/owl#Class\"]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#Operation\",\"@type\":[\"http://www.w3.org/2002/07/owl#Class\"]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#OperationInput\",\"@type\":[\"http://www.w3.org/2002/07/owl#Class\"]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#OperationOutput\",\"@type\":[\"http://www.w3.org/2002/07/owl#Class\"]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#OperationState\",\"@type\":[\"http://www.w3.org/2002/07/owl#Class\"]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#OutputDataPoint\",\"@type\":[\"http://www.w3.org/2002/07/owl#Class\"]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#Service\",\"@type\":[\"http://www.w3.org/2002/07/owl#Class\"]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#ThingProperty\",\"@type\":[\"http://www.w3.org/2002/07/owl#Class\"]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#Variable\",\"@type\":[\"http://www.w3.org/2002/07/owl#Class\"]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#cmdPump_1Off\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#Command\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasInput\":[{\"@id\":\"http://yang-netconf-mqtt#uuidInput\"}]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#cmdPump_1On\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#Command\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasInput\":[{\"@id\":\"http://yang-netconf-mqtt#uuidInput\"}]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#exposesCommand\",\"@type\":[\"http://www.w3.org/2002/07/owl#ObjectProperty\"]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#exposesFunctionality\",\"@type\":[\"http://www.w3.org/2002/07/owl#AnnotationProperty\"]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#funcDescHumidity\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://yang-netconf-mqtt#YangDescription\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasValue\":[{\"@value\":\"Gethumidityfromsensor\"}]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#funcDescMoisture\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://yang-netconf-mqtt#YangDescription\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasValue\":[{\"@value\":\"Getmoisturefromsensor\"}]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#funcDescPressure\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://yang-netconf-mqtt#YangDescription\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasValue\":[{\"@value\":\"Getpressurefromsensor\"}]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#funcDescPump_1Off\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://yang-netconf-mqtt#YangDescription\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasValue\":[{\"@value\":\"turnpump1off\"}]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#funcDescPump_1On\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://yang-netconf-mqtt#YangDescription\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasValue\":[{\"@value\":\"turnpump1on\"}]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#funcDescRainDetect\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://yang-netconf-mqtt#YangDescription\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasValue\":[{\"@value\":\"Getraindetectionsignalfromsensor\"}]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#funcDescTemperature\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://yang-netconf-mqtt#YangDescription\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasValue\":[{\"@value\":\"GetTemperaturefromsensor\"}]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#funcGetHumidity\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#MeasuringFunctionality\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasThingProperty\":[{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#funcDescHumidity\"}]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#funcGetMoisture\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#MeasuringFunctionality\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasThingProperty\":[{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#funcDescMoisture\"}]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#funcGetPressure\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#MeasuringFunctionality\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasThingProperty\":[{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#funcDescPressure\"}]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#funcGetRainDetect\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#MeasuringFunctionality\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasThingProperty\":[{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#funcDescRainDetect\"}]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#funcGetTemperature\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#MeasuringFunctionality\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasThingProperty\":[{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#funcDescTemperature\"}]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#funcPump_1Off\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#ControllingFunctionality\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasCommand\":[{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#cmdPump_1Off\"}],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasThingProperty\":[{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#funcDescPump_1Off\"}]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#funcPump_1On\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#ControllingFunctionality\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasCommand\":[{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#cmdPump_1On\"}],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasThingProperty\":[{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#funcDescPump_1On\"}]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasCommand\",\"@type\":[\"http://www.w3.org/2002/07/owl#ObjectProperty\"]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasDataRestriction_maxInclusive\",\"@type\":[\"http://www.w3.org/2002/07/owl#AnnotationProperty\"]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasDataRestriction_minInclusive\",\"@type\":[\"http://www.w3.org/2002/07/owl#AnnotationProperty\"]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasDataRestriction_pattern\",\"@type\":[\"http://www.w3.org/2002/07/owl#DatatypeProperty\"]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasFunctionality\",\"@type\":[\"http://www.w3.org/2002/07/owl#AnnotationProperty\"]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasInput\",\"@type\":[\"http://www.w3.org/2002/07/owl#ObjectProperty\"],\"http://www.w3.org/2000/01/rdf-schema#range\":[{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#ThingProperty\"}]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasOperation\",\"@type\":[\"http://www.w3.org/2002/07/owl#ObjectProperty\"]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasOperationState\",\"@type\":[\"http://www.w3.org/2002/07/owl#ObjectProperty\"]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasOutput\",\"@type\":[\"http://www.w3.org/2002/07/owl#ObjectProperty\"]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasOutputDataPoint\",\"@type\":[\"http://www.w3.org/2002/07/owl#ObjectProperty\"]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasService\",\"@type\":[\"http://www.w3.org/2002/07/owl#ObjectProperty\"]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasSubService\",\"@type\":[\"http://www.w3.org/2002/07/owl#ObjectProperty\"]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasSubStructure\",\"@type\":[\"http://www.w3.org/2002/07/owl#AnnotationProperty\"]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasThingProperty\",\"@type\":[\"http://www.w3.org/2002/07/owl#ObjectProperty\"]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasValue\",\"@type\":[\"http://www.w3.org/2002/07/owl#DatatypeProperty\"]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#opMqttPump_1Off\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#Operation\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#exposesCommand\":[{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#cmdPump_1Off\"}],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasInput\":[{\"@id\":\"http://yang-netconf-mqtt#uuidInput\"}],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasOperationState\":[{\"@id\":\"http://yang-netconf-mqtt#opState\"}],\"http://yang-netconf-mqtt#mqttMethod\":[{\"@value\":\"OFF\"}],\"http://yang-netconf-mqtt#mqttTopic\":[{\"@value\":\"actuator/pump/pump_1\"}]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#opMqttPump_1On\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#Operation\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#exposesCommand\":[{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#cmdPump_1On\"}],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasInput\":[{\"@id\":\"http://yang-netconf-mqtt#uuidInput\"}],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasOperationState\":[{\"@id\":\"http://yang-netconf-mqtt#opState\"}],\"http://yang-netconf-mqtt#mqttMethod\":[{\"@value\":\"ON\"}],\"http://yang-netconf-mqtt#mqttTopic\":[{\"@value\":\"actuator/pump/pump_1\"}]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#outDpHumidity\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#OutputDataPoint\"],\"http://yang-netconf-mqtt#mqttTopic\":[{\"@value\":\"sensor/humidity/humidity_1/%s\"}]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#outDpMoisture\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#OutputDataPoint\"],\"http://yang-netconf-mqtt#mqttTopic\":[{\"@value\":\"sensor/moisture/moisture_1/%s\"}]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#outDpPressure\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#OutputDataPoint\"],\"http://yang-netconf-mqtt#mqttTopic\":[{\"@value\":\"sensor/pressure/pressure_1/%s\"}]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#outDpRainDetect\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#OutputDataPoint\"],\"http://yang-netconf-mqtt#mqttTopic\":[{\"@value\":\"sensor/rain/rain_1/%s\"}]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#outDpTemperature\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#OutputDataPoint\"],\"http://yang-netconf-mqtt#mqttTopic\":[{\"@value\":\"sensor/temperature/temperature_1/%s\"}]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#servHumidity\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#Service\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#exposesFunctionality\":[{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#funcGetHumidity\"}],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasOutputDataPoint\":[{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#outDpHumidity\"}]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#servMoisture\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#Service\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#exposesFunctionality\":[{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#funcGetMoisture\"}],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasOutputDataPoint\":[{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#outDpMoisture\"}]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#servPressure\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#Service\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#exposesFunctionality\":[{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#funcGetPressure\"}],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasOutputDataPoint\":[{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#outDpPressure\"}]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#servRainDetect\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#Service\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#exposesFunctionality\":[{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#funcGetRainDetect\"}],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasOutputDataPoint\":[{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#outDpRainDetect\"}]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#servTemperature\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#Service\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#exposesFunctionality\":[{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#funcGetTemperature\"}],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasOutputDataPoint\":[{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#outDpTemperature\"}]},{\"@id\":\"http://yang-netconf-mqtt\",\"@type\":[\"http://www.w3.org/2002/07/owl#Ontology\"],\"http://www.w3.org/2002/07/owl#imports\":[{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology-v0_9_0\"}]},{\"@id\":\"http://yang-netconf-mqtt#YangDescription\",\"@type\":[\"http://www.w3.org/2002/07/owl#Class\"],\"http://www.w3.org/2000/01/rdf-schema#subClassOf\":[{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#ThingProperty\"}]},{\"@id\":\"http://yang-netconf-mqtt#cmdLed_1Off\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#Command\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasInput\":[{\"@id\":\"http://yang-netconf-mqtt#uuidInput\"}]},{\"@id\":\"http://yang-netconf-mqtt#cmdLed_1On\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#Command\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasInput\":[{\"@id\":\"http://yang-netconf-mqtt#uuidInput\"}]},{\"@id\":\"http://yang-netconf-mqtt#cmdLed_1Rgb\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#Command\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasInput\":[{\"@id\":\"http://yang-netconf-mqtt#rgbinput\"},{\"@id\":\"http://yang-netconf-mqtt#uuidInput\"}]},{\"@id\":\"http://yang-netconf-mqtt#deviceCategory\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#ThingProperty\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasValue\":[{\"@value\":\"%BoardName\"}]},{\"@id\":\"http://yang-netconf-mqtt#deviceDesc\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://yang-netconf-mqtt#YangDescription\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasValue\":[{\"@value\":\"MQTT-DeviceidentifiedbyUUID\"}]},{\"@id\":\"http://yang-netconf-mqtt#deviceUuid\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#ThingProperty\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasValue\":[{\"@value\":\"%s\"}]},{\"@id\":\"http://yang-netconf-mqtt#funcDescBrightness\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://yang-netconf-mqtt#YangDescription\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasValue\":[{\"@value\":\"Getbrightnessfromsensor\"}]},{\"@id\":\"http://yang-netconf-mqtt#funcDescLed_1Off\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://yang-netconf-mqtt#YangDescription\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasValue\":[{\"@value\":\"turnled1off\"}]},{\"@id\":\"http://yang-netconf-mqtt#funcDescLed_1On\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://yang-netconf-mqtt#YangDescription\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasValue\":[{\"@value\":\"turnled1on\"}]},{\"@id\":\"http://yang-netconf-mqtt#funcDescLed_1Rgb\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://yang-netconf-mqtt#YangDescription\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasValue\":[{\"@value\":\"setRGBvaluesforled1\"}]},{\"@id\":\"http://yang-netconf-mqtt#funcGetBrightness\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#MeasuringFunctionality\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasThingProperty\":[{\"@id\":\"http://yang-netconf-mqtt#funcDescBrightness\"}]},{\"@id\":\"http://yang-netconf-mqtt#funcLed_1Off\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#ControllingFunctionality\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasCommand\":[{\"@id\":\"http://yang-netconf-mqtt#cmdLed_1Off\"}],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasThingProperty\":[{\"@id\":\"http://yang-netconf-mqtt#funcDescLed_1Off\"}]},{\"@id\":\"http://yang-netconf-mqtt#funcLed_1On\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#ControllingFunctionality\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasCommand\":[{\"@id\":\"http://yang-netconf-mqtt#cmdLed_1On\"}],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasThingProperty\":[{\"@id\":\"http://yang-netconf-mqtt#funcDescLed_1On\"}]},{\"@id\":\"http://yang-netconf-mqtt#funcLed_1Rgb\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#ControllingFunctionality\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasCommand\":[{\"@id\":\"http://yang-netconf-mqtt#cmdLed_1Rgb\"}],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasThingProperty\":[{\"@id\":\"http://yang-netconf-mqtt#funcDescLed_1Rgb\"}]},{\"@id\":\"http://yang-netconf-mqtt#inputBlue\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#Variable\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasDataRestriction_maxInclusive\":[{\"@type\":\"http://www.w3.org/2001/XMLSchema#int\",\"@value\":\"255\"}],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasDataRestriction_minInclusive\":[{\"@type\":\"http://www.w3.org/2001/XMLSchema#int\",\"@value\":\"0\"}]},{\"@id\":\"http://yang-netconf-mqtt#inputGreen\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#Variable\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasDataRestriction_maxInclusive\":[{\"@type\":\"http://www.w3.org/2001/XMLSchema#int\",\"@value\":\"255\"}],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasDataRestriction_minInclusive\":[{\"@type\":\"http://www.w3.org/2001/XMLSchema#int\",\"@value\":\"0\"}]},{\"@id\":\"http://yang-netconf-mqtt#inputRed\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#Variable\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasDataRestriction_maxInclusive\":[{\"@type\":\"http://www.w3.org/2001/XMLSchema#int\",\"@value\":\"255\"}],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasDataRestriction_minInclusive\":[{\"@type\":\"http://www.w3.org/2001/XMLSchema#int\",\"@value\":\"0\"}]},{\"@id\":\"http://yang-netconf-mqtt#mqttMethod\",\"@type\":[\"http://www.w3.org/2002/07/owl#DatatypeProperty\"]},{\"@id\":\"http://yang-netconf-mqtt#mqttTopic\",\"@type\":[\"http://www.w3.org/2002/07/owl#DatatypeProperty\"]},{\"@id\":\"http://yang-netconf-mqtt#myDevice\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#Device\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasFunctionality\":[{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#funcGetHumidity\"},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#funcGetMoisture\"},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#funcGetPressure\"},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#funcGetRainDetect\"},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#funcGetTemperature\"},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#funcPump_1Off\"},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#funcPump_1On\"},{\"@id\":\"http://yang-netconf-mqtt#funcGetBrightness\"},{\"@id\":\"http://yang-netconf-mqtt#funcLed_1Off\"},{\"@id\":\"http://yang-netconf-mqtt#funcLed_1On\"},{\"@id\":\"http://yang-netconf-mqtt#funcLed_1Rgb\"}],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasService\":[{\"@id\":\"http://yang-netconf-mqtt#servNetconf\"}],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasThingProperty\":[{\"@id\":\"http://yang-netconf-mqtt#deviceCategory\"},{\"@id\":\"http://yang-netconf-mqtt#deviceDesc\"},{\"@id\":\"http://yang-netconf-mqtt#deviceUuid\"}]},{\"@id\":\"http://yang-netconf-mqtt#opDescState\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://yang-netconf-mqtt#YangDescription\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasDataRestriction_pattern\":[{\"@value\":\"error\"},{\"@value\":\"nothingtodo\"},{\"@value\":\"successful\"}]},{\"@id\":\"http://yang-netconf-mqtt#opMqttLed_1Off\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#Operation\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#exposesCommand\":[{\"@id\":\"http://yang-netconf-mqtt#cmdLed_1Off\"}],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasInput\":[{\"@id\":\"http://yang-netconf-mqtt#uuidInput\"}],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasOperationState\":[{\"@id\":\"http://yang-netconf-mqtt#opState\"}],\"http://yang-netconf-mqtt#mqttMethod\":[{\"@value\":\"OFF\"}],\"http://yang-netconf-mqtt#mqttTopic\":[{\"@value\":\"actuator/led/led_1\"}]},{\"@id\":\"http://yang-netconf-mqtt#opMqttLed_1On\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#Operation\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#exposesCommand\":[{\"@id\":\"http://yang-netconf-mqtt#cmdLed_1On\"}],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasInput\":[{\"@id\":\"http://yang-netconf-mqtt#uuidInput\"}],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasOperationState\":[{\"@id\":\"http://yang-netconf-mqtt#opState\"}],\"http://yang-netconf-mqtt#mqttMethod\":[{\"@value\":\"ON\"}],\"http://yang-netconf-mqtt#mqttTopic\":[{\"@value\":\"actuator/led/led_1\"}]},{\"@id\":\"http://yang-netconf-mqtt#opMqttLed_1Rgb\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#Operation\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#exposesCommand\":[{\"@id\":\"http://yang-netconf-mqtt#cmdLed_1Rgb\"}],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasInput\":[{\"@id\":\"http://yang-netconf-mqtt#rgbinput\"},{\"@id\":\"http://yang-netconf-mqtt#uuidInput\"}],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasOperationState\":[{\"@id\":\"http://yang-netconf-mqtt#opState\"}],\"http://yang-netconf-mqtt#mqttMethod\":[{\"@value\":\"rgb\"}],\"http://yang-netconf-mqtt#mqttTopic\":[{\"@value\":\"actuator/led/led_1\"}]},{\"@id\":\"http://yang-netconf-mqtt#opState\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#OperationState\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasDataRestriction_pattern\":[{\"@value\":\"ERROR\"},{\"@value\":\"NOOP\"},{\"@value\":\"OK\"}],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasThingProperty\":[{\"@id\":\"http://yang-netconf-mqtt#opDescState\"}]},{\"@id\":\"http://yang-netconf-mqtt#outDpBrightness\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#OutputDataPoint\"],\"http://yang-netconf-mqtt#mqttTopic\":[{\"@value\":\"sensor/brightness/brightness_1/%s\"}]},{\"@id\":\"http://yang-netconf-mqtt#rgbYangDesc\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://yang-netconf-mqtt#YangDescription\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasValue\":[{\"@value\":\"RGBparameterforled_1\"}]},{\"@id\":\"http://yang-netconf-mqtt#rgbinput\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#OperationInput\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasSubStructure\":[{\"@id\":\"http://yang-netconf-mqtt#inputBlue\"},{\"@id\":\"http://yang-netconf-mqtt#inputGreen\"},{\"@id\":\"http://yang-netconf-mqtt#inputRed\"}],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasThingProperty\":[{\"@id\":\"http://yang-netconf-mqtt#rgbYangDesc\"}]},{\"@id\":\"http://yang-netconf-mqtt#servBrightness\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#Service\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#exposesFunctionality\":[{\"@id\":\"http://yang-netconf-mqtt#funcGetBrightness\"}],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasOutputDataPoint\":[{\"@id\":\"http://yang-netconf-mqtt#outDpBrightness\"}]},{\"@id\":\"http://yang-netconf-mqtt#servNetconf\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#Service\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#exposesFunctionality\":[{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#funcGetHumidity\"},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#funcGetMoisture\"},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#funcGetPressure\"},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#funcGetRainDetect\"},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#funcGetTemperature\"},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#funcPump_1Off\"},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#funcPump_1On\"},{\"@id\":\"http://yang-netconf-mqtt#funcGetBrightness\"},{\"@id\":\"http://yang-netconf-mqtt#funcLed_1Off\"},{\"@id\":\"http://yang-netconf-mqtt#funcLed_1On\"},{\"@id\":\"http://yang-netconf-mqtt#funcLed_1Rgb\"}],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasOperation\":[{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#opMqttPump_1Off\"},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#opMqttPump_1On\"},{\"@id\":\"http://yang-netconf-mqtt#opMqttLed_1Off\"},{\"@id\":\"http://yang-netconf-mqtt#opMqttLed_1On\"},{\"@id\":\"http://yang-netconf-mqtt#opMqttLed_1Rgb\"}],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasSubService\":[{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#servHumidity\"},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#servMoisture\"},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#servPressure\"},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#servRainDetect\"},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#servTemperature\"},{\"@id\":\"http://yang-netconf-mqtt#servBrightness\"}]},{\"@id\":\"http://yang-netconf-mqtt#uuidInput\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#OperationInput\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasInput\":[{\"@id\":\"http://yang-netconf-mqtt#deviceUuid\"}],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasThingProperty\":[{\"@id\":\"http://yang-netconf-mqtt#uuidYangDesc\"}],\"http://yang-netconf-mqtt#mqttMethod\":[{\"@value\":\"OFF\"}]},{\"@id\":\"http://yang-netconf-mqtt#uuidYangDesc\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://yang-netconf-mqtt#YangDescription\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasValue\":[{\"@value\":\"TargetUUIDforrequest\"}]}]";
//Ontology with units
String ontology ="[{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#Command\",\"@type\":[\"http://www.w3.org/2002/07/owl#Class\"]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#ControllingFunctionality\",\"@type\":[\"http://www.w3.org/2002/07/owl#Class\"]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#Device\",\"@type\":[\"http://www.w3.org/2002/07/owl#Class\"]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#MeasuringFunctionality\",\"@type\":[\"http://www.w3.org/2002/07/owl#Class\"]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#Operation\",\"@type\":[\"http://www.w3.org/2002/07/owl#Class\"]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#OperationInput\",\"@type\":[\"http://www.w3.org/2002/07/owl#Class\"]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#OperationOutput\",\"@type\":[\"http://www.w3.org/2002/07/owl#Class\"]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#OperationState\",\"@type\":[\"http://www.w3.org/2002/07/owl#Class\"]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#OutputDataPoint\",\"@type\":[\"http://www.w3.org/2002/07/owl#Class\"]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#Service\",\"@type\":[\"http://www.w3.org/2002/07/owl#Class\"]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#ThingProperty\",\"@type\":[\"http://www.w3.org/2002/07/owl#Class\"]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#Variable\",\"@type\":[\"http://www.w3.org/2002/07/owl#Class\"]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#cmdPump_1Off\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#Command\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasInput\":[{\"@id\":\"http://yang-netconf-mqtt#uuidInput\"}]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#cmdPump_1On\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#Command\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasInput\":[{\"@id\":\"http://yang-netconf-mqtt#uuidInput\"}]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#exposesCommand\",\"@type\":[\"http://www.w3.org/2002/07/owl#ObjectProperty\"]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#exposesFunctionality\",\"@type\":[\"http://www.w3.org/2002/07/owl#AnnotationProperty\"]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#funcDescHumidity\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://yang-netconf-mqtt#YangDescription\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasValue\":[{\"@value\":\"Gethumidityfromsensor\"}]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#funcDescMoisture\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://yang-netconf-mqtt#YangDescription\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasValue\":[{\"@value\":\"Getmoisturefromsensor\"}]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#funcDescPressure\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://yang-netconf-mqtt#YangDescription\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasValue\":[{\"@value\":\"Getpressurefromsensor\"}]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#funcDescPump_1Off\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://yang-netconf-mqtt#YangDescription\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasValue\":[{\"@value\":\"turnpump1off\"}]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#funcDescPump_1On\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://yang-netconf-mqtt#YangDescription\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasValue\":[{\"@value\":\"turnpump1on\"}]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#funcDescRainDetect\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://yang-netconf-mqtt#YangDescription\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasValue\":[{\"@value\":\"Getraindetectionsignalfromsensor\"}]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#funcDescTemperature\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://yang-netconf-mqtt#YangDescription\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasValue\":[{\"@value\":\"GetTemperaturefromsensor\"}]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#funcGetHumidity\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#MeasuringFunctionality\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasThingProperty\":[{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#funcDescHumidity\"}]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#funcGetMoisture\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#MeasuringFunctionality\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasThingProperty\":[{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#funcDescMoisture\"}]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#funcGetPressure\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#MeasuringFunctionality\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasThingProperty\":[{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#funcDescPressure\"}]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#funcGetRainDetect\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#MeasuringFunctionality\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasThingProperty\":[{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#funcDescRainDetect\"}]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#funcGetTemperature\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#MeasuringFunctionality\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasThingProperty\":[{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#funcDescTemperature\"}]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#funcPump_1Off\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#ControllingFunctionality\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasCommand\":[{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#cmdPump_1Off\"}],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasThingProperty\":[{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#funcDescPump_1Off\"}]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#funcPump_1On\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#ControllingFunctionality\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasCommand\":[{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#cmdPump_1On\"}],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasThingProperty\":[{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#funcDescPump_1On\"}]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasCommand\",\"@type\":[\"http://www.w3.org/2002/07/owl#ObjectProperty\"]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasDataRestriction_maxInclusive\",\"@type\":[\"http://www.w3.org/2002/07/owl#AnnotationProperty\"]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasDataRestriction_minInclusive\",\"@type\":[\"http://www.w3.org/2002/07/owl#AnnotationProperty\"]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasDataRestriction_pattern\",\"@type\":[\"http://www.w3.org/2002/07/owl#DatatypeProperty\"]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasFunctionality\",\"@type\":[\"http://www.w3.org/2002/07/owl#AnnotationProperty\"]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasInput\",\"@type\":[\"http://www.w3.org/2002/07/owl#ObjectProperty\"],\"http://www.w3.org/2000/01/rdf-schema#range\":[{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#ThingProperty\"}]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasOperation\",\"@type\":[\"http://www.w3.org/2002/07/owl#ObjectProperty\"]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasOperationState\",\"@type\":[\"http://www.w3.org/2002/07/owl#ObjectProperty\"]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasOutput\",\"@type\":[\"http://www.w3.org/2002/07/owl#ObjectProperty\"]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasOutputDataPoint\",\"@type\":[\"http://www.w3.org/2002/07/owl#ObjectProperty\"]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasService\",\"@type\":[\"http://www.w3.org/2002/07/owl#ObjectProperty\"]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasSubService\",\"@type\":[\"http://www.w3.org/2002/07/owl#ObjectProperty\"]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasSubStructure\",\"@type\":[\"http://www.w3.org/2002/07/owl#AnnotationProperty\"]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasThingProperty\",\"@type\":[\"http://www.w3.org/2002/07/owl#ObjectProperty\"]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasValue\",\"@type\":[\"http://www.w3.org/2002/07/owl#DatatypeProperty\"]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#opMqttPump_1Off\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#Operation\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#exposesCommand\":[{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#cmdPump_1Off\"}],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasInput\":[{\"@id\":\"http://yang-netconf-mqtt#uuidInput\"}],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasOperationState\":[{\"@id\":\"http://yang-netconf-mqtt#opState\"}],\"http://yang-netconf-mqtt#mqttMethod\":[{\"@value\":\"OFF\"}],\"http://yang-netconf-mqtt#mqttTopic\":[{\"@value\":\"actuator/pump/pump_1\"}]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#opMqttPump_1On\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#Operation\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#exposesCommand\":[{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#cmdPump_1On\"}],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasInput\":[{\"@id\":\"http://yang-netconf-mqtt#uuidInput\"}],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasOperationState\":[{\"@id\":\"http://yang-netconf-mqtt#opState\"}],\"http://yang-netconf-mqtt#mqttMethod\":[{\"@value\":\"ON\"}],\"http://yang-netconf-mqtt#mqttTopic\":[{\"@value\":\"actuator/pump/pump_1\"}]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#outDpHumidity\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#OutputDataPoint\"],\"http://www.ontology-of-units-of-measure.org/resource/om-2/hasUnit\":[{\"@id\":\"http://www.ontology-of-units-of-measure.org/resource/om-2/percent\"}],\"http://yang-netconf-mqtt#mqttTopic\":[{\"@value\":\"sensor/humidity/humidity_1/%s\"}]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#outDpMoisture\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#OutputDataPoint\"],\"http://www.ontology-of-units-of-measure.org/resource/om-2/hasUnit\":[{\"@id\":\"http://www.ontology-of-units-of-measure.org/resource/om-2/percent\"}],\"http://yang-netconf-mqtt#mqttTopic\":[{\"@value\":\"sensor/moisture/moisture_1/%s\"}]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#outDpPressure\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#OutputDataPoint\"],\"http://www.ontology-of-units-of-measure.org/resource/om-2/hasUnit\":[{\"@id\":\"http://www.ontology-of-units-of-measure.org/resource/om-2/hectopascal\"}],\"http://yang-netconf-mqtt#mqttTopic\":[{\"@value\":\"sensor/pressure/pressure_1/%s\"}]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#outDpRainDetect\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#OutputDataPoint\"],\"http://www.ontology-of-units-of-measure.org/resource/om-2/hasUnit\":[{\"@id\":\"http://www.ontology-of-units-of-measure.org/resource/om-2/percent\"}],\"http://yang-netconf-mqtt#mqttTopic\":[{\"@value\":\"sensor/rain/rain_1/%s\"}]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#outDpTemperature\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#OutputDataPoint\"],\"http://www.ontology-of-units-of-measure.org/resource/om-2/hasUnit\":[{\"@id\":\"http://www.ontology-of-units-of-measure.org/resource/om-2/degreeCelsius\"}],\"http://yang-netconf-mqtt#mqttTopic\":[{\"@value\":\"sensor/temperature/temperature_1/%s\"}]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#servHumidity\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#Service\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#exposesFunctionality\":[{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#funcGetHumidity\"}],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasOutputDataPoint\":[{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#outDpHumidity\"}]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#servMoisture\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#Service\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#exposesFunctionality\":[{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#funcGetMoisture\"}],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasOutputDataPoint\":[{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#outDpMoisture\"}]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#servPressure\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#Service\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#exposesFunctionality\":[{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#funcGetPressure\"}],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasOutputDataPoint\":[{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#outDpPressure\"}]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#servRainDetect\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#Service\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#exposesFunctionality\":[{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#funcGetRainDetect\"}],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasOutputDataPoint\":[{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#outDpRainDetect\"}]},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#servTemperature\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#Service\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#exposesFunctionality\":[{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#funcGetTemperature\"}],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasOutputDataPoint\":[{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#outDpTemperature\"}]},{\"@id\":\"http://yang-netconf-mqtt\",\"@type\":[\"http://www.w3.org/2002/07/owl#Ontology\"],\"http://www.w3.org/2002/07/owl#imports\":[{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology-v0_9_0\"},{\"@id\":\"http://www.ontology-of-units-of-measure.org/resource/om-2/\"}]},{\"@id\":\"http://yang-netconf-mqtt#YangDescription\",\"@type\":[\"http://www.w3.org/2002/07/owl#Class\"],\"http://www.w3.org/2000/01/rdf-schema#subClassOf\":[{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#ThingProperty\"}]},{\"@id\":\"http://yang-netconf-mqtt#cmdLed_1Off\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#Command\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasInput\":[{\"@id\":\"http://yang-netconf-mqtt#uuidInput\"}]},{\"@id\":\"http://yang-netconf-mqtt#cmdLed_1On\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#Command\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasInput\":[{\"@id\":\"http://yang-netconf-mqtt#uuidInput\"}]},{\"@id\":\"http://yang-netconf-mqtt#cmdLed_1Rgb\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#Command\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasInput\":[{\"@id\":\"http://yang-netconf-mqtt#rgbinput\"},{\"@id\":\"http://yang-netconf-mqtt#uuidInput\"}]},{\"@id\":\"http://yang-netconf-mqtt#deviceCategory\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#ThingProperty\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasValue\":[{\"@value\":\"%BoardName\"}]},{\"@id\":\"http://yang-netconf-mqtt#deviceDesc\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://yang-netconf-mqtt#YangDescription\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasValue\":[{\"@value\":\"MQTT-DeviceidentifiedbyUUID\"}]},{\"@id\":\"http://yang-netconf-mqtt#deviceUuid\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#ThingProperty\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasValue\":[{\"@value\":\"%s\"}]},{\"@id\":\"http://yang-netconf-mqtt#funcDescBrightness\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://yang-netconf-mqtt#YangDescription\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasValue\":[{\"@value\":\"Getbrightnessfromsensor\"}]},{\"@id\":\"http://yang-netconf-mqtt#funcDescLed_1Off\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://yang-netconf-mqtt#YangDescription\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasValue\":[{\"@value\":\"turnled1off\"}]},{\"@id\":\"http://yang-netconf-mqtt#funcDescLed_1On\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://yang-netconf-mqtt#YangDescription\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasValue\":[{\"@value\":\"turnled1on\"}]},{\"@id\":\"http://yang-netconf-mqtt#funcDescLed_1Rgb\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://yang-netconf-mqtt#YangDescription\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasValue\":[{\"@value\":\"setRGBvaluesforled1\"}]},{\"@id\":\"http://yang-netconf-mqtt#funcGetBrightness\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#MeasuringFunctionality\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasThingProperty\":[{\"@id\":\"http://yang-netconf-mqtt#funcDescBrightness\"}]},{\"@id\":\"http://yang-netconf-mqtt#funcLed_1Off\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#ControllingFunctionality\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasCommand\":[{\"@id\":\"http://yang-netconf-mqtt#cmdLed_1Off\"}],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasThingProperty\":[{\"@id\":\"http://yang-netconf-mqtt#funcDescLed_1Off\"}]},{\"@id\":\"http://yang-netconf-mqtt#funcLed_1On\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#ControllingFunctionality\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasCommand\":[{\"@id\":\"http://yang-netconf-mqtt#cmdLed_1On\"}],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasThingProperty\":[{\"@id\":\"http://yang-netconf-mqtt#funcDescLed_1On\"}]},{\"@id\":\"http://yang-netconf-mqtt#funcLed_1Rgb\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#ControllingFunctionality\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasCommand\":[{\"@id\":\"http://yang-netconf-mqtt#cmdLed_1Rgb\"}],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasThingProperty\":[{\"@id\":\"http://yang-netconf-mqtt#funcDescLed_1Rgb\"}]},{\"@id\":\"http://yang-netconf-mqtt#inputBlue\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#Variable\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasDataRestriction_maxInclusive\":[{\"@type\":\"http://www.w3.org/2001/XMLSchema#int\",\"@value\":\"255\"}],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasDataRestriction_minInclusive\":[{\"@type\":\"http://www.w3.org/2001/XMLSchema#int\",\"@value\":\"0\"}]},{\"@id\":\"http://yang-netconf-mqtt#inputGreen\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#Variable\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasDataRestriction_maxInclusive\":[{\"@type\":\"http://www.w3.org/2001/XMLSchema#int\",\"@value\":\"255\"}],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasDataRestriction_minInclusive\":[{\"@type\":\"http://www.w3.org/2001/XMLSchema#int\",\"@value\":\"0\"}]},{\"@id\":\"http://yang-netconf-mqtt#inputRed\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#Variable\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasDataRestriction_maxInclusive\":[{\"@type\":\"http://www.w3.org/2001/XMLSchema#int\",\"@value\":\"255\"}],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasDataRestriction_minInclusive\":[{\"@type\":\"http://www.w3.org/2001/XMLSchema#int\",\"@value\":\"0\"}]},{\"@id\":\"http://yang-netconf-mqtt#mqttMethod\",\"@type\":[\"http://www.w3.org/2002/07/owl#DatatypeProperty\"]},{\"@id\":\"http://yang-netconf-mqtt#mqttTopic\",\"@type\":[\"http://www.w3.org/2002/07/owl#DatatypeProperty\"]},{\"@id\":\"http://yang-netconf-mqtt#myDevice\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#Device\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasFunctionality\":[{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#funcGetHumidity\"},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#funcGetMoisture\"},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#funcGetPressure\"},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#funcGetRainDetect\"},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#funcGetTemperature\"},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#funcPump_1Off\"},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#funcPump_1On\"},{\"@id\":\"http://yang-netconf-mqtt#funcGetBrightness\"},{\"@id\":\"http://yang-netconf-mqtt#funcLed_1Off\"},{\"@id\":\"http://yang-netconf-mqtt#funcLed_1On\"},{\"@id\":\"http://yang-netconf-mqtt#funcLed_1Rgb\"}],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasService\":[{\"@id\":\"http://yang-netconf-mqtt#servNetconf\"}],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasThingProperty\":[{\"@id\":\"http://yang-netconf-mqtt#deviceCategory\"},{\"@id\":\"http://yang-netconf-mqtt#deviceDesc\"},{\"@id\":\"http://yang-netconf-mqtt#deviceUuid\"}]},{\"@id\":\"http://yang-netconf-mqtt#opDescState\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://yang-netconf-mqtt#YangDescription\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasDataRestriction_pattern\":[{\"@value\":\"error\"},{\"@value\":\"nothingtodo\"},{\"@value\":\"successful\"}]},{\"@id\":\"http://yang-netconf-mqtt#opMqttLed_1Off\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#Operation\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#exposesCommand\":[{\"@id\":\"http://yang-netconf-mqtt#cmdLed_1Off\"}],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasInput\":[{\"@id\":\"http://yang-netconf-mqtt#uuidInput\"}],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasOperationState\":[{\"@id\":\"http://yang-netconf-mqtt#opState\"}],\"http://yang-netconf-mqtt#mqttMethod\":[{\"@value\":\"OFF\"}],\"http://yang-netconf-mqtt#mqttTopic\":[{\"@value\":\"actuator/led/led_1\"}]},{\"@id\":\"http://yang-netconf-mqtt#opMqttLed_1On\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#Operation\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#exposesCommand\":[{\"@id\":\"http://yang-netconf-mqtt#cmdLed_1On\"}],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasInput\":[{\"@id\":\"http://yang-netconf-mqtt#uuidInput\"}],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasOperationState\":[{\"@id\":\"http://yang-netconf-mqtt#opState\"}],\"http://yang-netconf-mqtt#mqttMethod\":[{\"@value\":\"ON\"}],\"http://yang-netconf-mqtt#mqttTopic\":[{\"@value\":\"actuator/led/led_1\"}]},{\"@id\":\"http://yang-netconf-mqtt#opMqttLed_1Rgb\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#Operation\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#exposesCommand\":[{\"@id\":\"http://yang-netconf-mqtt#cmdLed_1Rgb\"}],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasInput\":[{\"@id\":\"http://yang-netconf-mqtt#rgbinput\"},{\"@id\":\"http://yang-netconf-mqtt#uuidInput\"}],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasOperationState\":[{\"@id\":\"http://yang-netconf-mqtt#opState\"}],\"http://yang-netconf-mqtt#mqttMethod\":[{\"@value\":\"rgb\"}],\"http://yang-netconf-mqtt#mqttTopic\":[{\"@value\":\"actuator/led/led_1\"}]},{\"@id\":\"http://yang-netconf-mqtt#opState\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#OperationState\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasDataRestriction_pattern\":[{\"@value\":\"ERROR\"},{\"@value\":\"NOOP\"},{\"@value\":\"OK\"}],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasThingProperty\":[{\"@id\":\"http://yang-netconf-mqtt#opDescState\"}]},{\"@id\":\"http://yang-netconf-mqtt#outDpBrightness\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#OutputDataPoint\"],\"http://www.ontology-of-units-of-measure.org/resource/om-2/hasUnit\":[{\"@id\":\"http://www.ontology-of-units-of-measure.org/resource/om-2/lux\"}],\"http://yang-netconf-mqtt#mqttTopic\":[{\"@value\":\"sensor/brightness/brightness_1/%s\"}]},{\"@id\":\"http://yang-netconf-mqtt#rgbYangDesc\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://yang-netconf-mqtt#YangDescription\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasValue\":[{\"@value\":\"RGBparameterforled_1\"}]},{\"@id\":\"http://yang-netconf-mqtt#rgbinput\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#OperationInput\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasSubStructure\":[{\"@id\":\"http://yang-netconf-mqtt#inputBlue\"},{\"@id\":\"http://yang-netconf-mqtt#inputGreen\"},{\"@id\":\"http://yang-netconf-mqtt#inputRed\"}],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasThingProperty\":[{\"@id\":\"http://yang-netconf-mqtt#rgbYangDesc\"}]},{\"@id\":\"http://yang-netconf-mqtt#servBrightness\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#Service\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#exposesFunctionality\":[{\"@id\":\"http://yang-netconf-mqtt#funcGetBrightness\"}],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasOutputDataPoint\":[{\"@id\":\"http://yang-netconf-mqtt#outDpBrightness\"}]},{\"@id\":\"http://yang-netconf-mqtt#servNetconf\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#Service\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#exposesFunctionality\":[{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#funcGetHumidity\"},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#funcGetMoisture\"},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#funcGetPressure\"},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#funcGetRainDetect\"},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#funcGetTemperature\"},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#funcPump_1Off\"},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#funcPump_1On\"},{\"@id\":\"http://yang-netconf-mqtt#funcGetBrightness\"},{\"@id\":\"http://yang-netconf-mqtt#funcLed_1Off\"},{\"@id\":\"http://yang-netconf-mqtt#funcLed_1On\"},{\"@id\":\"http://yang-netconf-mqtt#funcLed_1Rgb\"}],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasOperation\":[{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#opMqttPump_1Off\"},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#opMqttPump_1On\"},{\"@id\":\"http://yang-netconf-mqtt#opMqttLed_1Off\"},{\"@id\":\"http://yang-netconf-mqtt#opMqttLed_1On\"},{\"@id\":\"http://yang-netconf-mqtt#opMqttLed_1Rgb\"}],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasSubService\":[{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#servHumidity\"},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#servMoisture\"},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#servPressure\"},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#servRainDetect\"},{\"@id\":\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#servTemperature\"},{\"@id\":\"http://yang-netconf-mqtt#servBrightness\"}]},{\"@id\":\"http://yang-netconf-mqtt#uuidInput\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#OperationInput\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasInput\":[{\"@id\":\"http://yang-netconf-mqtt#deviceUuid\"}],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasThingProperty\":[{\"@id\":\"http://yang-netconf-mqtt#uuidYangDesc\"}],\"http://yang-netconf-mqtt#mqttMethod\":[{\"@value\":\"OFF\"}]},{\"@id\":\"http://yang-netconf-mqtt#uuidYangDesc\",\"@type\":[\"http://www.w3.org/2002/07/owl#NamedIndividual\",\"http://yang-netconf-mqtt#YangDescription\"],\"http://www.onem2m.org/ontology/Base_Ontology/base_ontology#hasValue\":[{\"@value\":\"TargetUUIDforrequest\"}]}]";
// Sleep variables
int measurement = 0;

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
Adafruit_BME280 hum_pres_temp_1;
const int brightness_scl = 21;
const int brightness_sda = 22;
const int moisture_out = 33;
const int rain_out = 32;
// Actuators
const int led_1r = 25;
const int led_1g = 26;
const int led_1b = 27;
const int pump_1 = 18;
  // global to remember old values 
int rval, gval, bval = 0;


//Setup includes: Wifi and MQTT Conncection, Setting up Sensors, Publishing Device Description
void setup(){
  delay(200);
  Serial.begin(115200);
  if (bootCount == 0){
    Serial.println("Starting Sketch");
  }
  ++bootCount;
  generate_config();
  setup_wifi(); 
  setup_mqtt();
  setup_sensors();
  setup_actuators();
  if (bootCount == 1){
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
   while (WiFi.status() != WL_CONNECTED && (millis()-starttime) < MINUTE) {
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
void setup_mqtt(){
    client.setServer(mqtt_server, 1883);
    client.setCallback(callback);
    unsigned long starttime = millis();
    while (!client.connected() && (millis()-starttime) < MINUTE) {
        Serial.println();   
        Serial.println("Attempting MQTT-Connection: ");
        Serial.print(".");
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
    if(!client.connected()){
       Serial.println("Unable to attain MQTT connection, going to sleep, press rst to start again.");
       esp_deep_sleep_start();
    }
    Serial.println("MQTT connected.");
    client.subscribe(LED1_TOPIC_S);
    client.subscribe(PUMP1_TOPIC_S);
    Serial.println("Subscribed to: "LED1_TOPIC_S);
    client.subscribe(CREATE_TOPIC_S);
    client.subscribe(RETRIEVE_TOPIC_S);
    client.subscribe(UPDATE_TOPIC_S);
    client.subscribe(DELETE_TOPIC_S);
    Serial.println("Subscribed to CRUD Topics: "CRUD_TOPICS_S);
    client.subscribe(PING_TOPIC_S);
    Serial.println("Subscribed to PING Topic: "PING_TOPIC_S);
}

void device_configuration(char* operation){
  // OPERATION: CREATE; RETRIEVE; UPDATE; DELETE
  // FIRST IS ALWAYS RETRIEVE
  device_isknown = false;
  client.publish(RETRIEVE_TOPIC_P,UUID);
  crud_response_wait("retrieve");
  if (strcmp(operation,"create") == 0){ 
    if(device_isknown){
      Serial.println("Aborting Configuration Publication. Device already known. Updating instead.");
      publish_config(UPDATE_TOPIC_P);
      crud_response_wait("update");
    }
    else{
      publish_config(CREATE_TOPIC_P);
      crud_response_wait("create");
    }
  }
  else if (strcmp(operation,"retrieve") == 0){
    // Read is performed first
  }
  else if (strcmp(operation,"update") == 0){
    if(device_isknown){
      publish_config(UPDATE_TOPIC_P);
      crud_response_wait("update");
    }
    else{
      Serial.println("Device unknown, publishing config instead");
      publish_config(CREATE_TOPIC_P);
      crud_response_wait("create");
    }
  }
  else if (strcmp(operation,"delete") == 0){
    if(device_isknown){
      client.publish(DELETE_TOPIC_P,UUID);
      crud_response_wait("delete");
    }
  else{
      Serial.println("Device unknown, deletion impossible");
    }
  }
}

// Wait for response to CRUD-Request before moving forward
void crud_response_wait(char* Operation){
   got_crud_response = false;
   unsigned long starttime = millis();
   while(tries < 5){
     while(!got_crud_response && (millis()-starttime) < MINUTE){
      if(!client.connected()){
        if(WiFi.status() != WL_CONNECTED){
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
   if(got_crud_response){
    return;
   }
   else{
    tries++;
    device_configuration(Operation);
   }
  }
  char *msg;
  sprintf(msg,UUID" %s",Operation);
  client.publish(ERROR_P,msg);
}
void generate_config(){
  // Replace %s with device uuid and concatenate device uuid and ontology
  ontology.replace("%s",UUID);
  ontology.replace("%BoardName",BOARD);
  //xString(device_uuid+";"+ontology).toCharArray(configuration,sizeof(configuration));
}

// Publication of Device Description called within device_configuration
void publish_config(char* topic){
  Serial.println();   
  Serial.println("Publishing Device Description: ");  
  client.publish(topic,String(device_uuid+";"+ontology).c_str());

  // Create END-Message for configuration and send
  //String(device_uuid+";END").toCharArray(configuration_end,sizeof(configuration_end));
  client.publish(topic,UUID";END");
  Serial.println("Ontology published.");
}

void setup_sensors(){
   // Sensors are defined globally
   // BH1750: brigthness_1
   // Wire.begin takes two parameters: SDA pin and SCL pin;
   Wire.begin(brightness_sda,brightness_scl);
   hum_pres_temp_1.begin(0x76, &Wire);
   Serial.println("BME280 Set up");
   brightness_1.begin(BH1750::CONTINUOUS_HIGH_RES_MODE_2);
   Serial.println("BH1750 High Resolution Mode");


}

// Measure brightness and ouput to UI and SSN-Topic
void out_brightness_1(){
  char lux_msg[15];
  float lux =  brightness_1.readLightLevel();
  //dtostrf(lux, 5, 2, lux_char);
 //dtostrf(lux, 5, 2, lux_char);
    sprintf(lux_msg, "%.2f", lux);
    sprintf(lux_msg, "%.1f lux", lux);
    client.publish(BRIGHT1_TOPIC_P, lux_msg);
}
void out_humidity_1(){
  char hum_msg[15];
  float hum = hum_pres_temp_1.readHumidity();
  sprintf(hum_msg, "%.1f %%", hum);
  client.publish(HUM1_TOPIC_P, hum_msg);
}

void out_pressure_1(){
  char pres_msg[15];
  float pres = hum_pres_temp_1.readPressure() / 100.00;
  sprintf(pres_msg, "%.1f hPa", pres);
  client.publish(PRES1_TOPIC_P, pres_msg);
}

void out_temperature_1(){
  char temp_msg[15];
  float temp = hum_pres_temp_1.readTemperature();
  sprintf(temp_msg, "%.1f °C", temp);
  client.publish(TEMP1_TOPIC_P, temp_msg);
}

void out_rain_1(){
char rain_msg[17];
  int rain = analogRead(rain_out);
  Serial.print("Rain ");
  Serial.println(rain);
  if (rain >= RAIN_THRESH){
    client.publish(RAIN1_TOPIC_P, "No rain detected");
    }
  else if( rain >= RAIN_SIGNAL_THRESH) {
    client.publish(RAIN1_TOPIC_P, "Rain detected");
    }
  else{
    client.publish(RAIN1_TOPIC_P, "Low signal, check sensor");
  }
  }

void out_moisture_1(){
char moisture_msg[25];
  int moisture = analogRead(moisture_out);
    Serial.print("Moisture ");
    Serial.println(moisture);

  if (moisture >= MOISTURE_MOIST_THRESH){
    client.publish(MOIST1_TOPIC_P, "Soil is dry");
    }
  else if  (moisture >= MOISTURE_WET_THRESH){
    client.publish(MOIST1_TOPIC_P, "Soil is moist");
    }
  else if (moisture >= MOISTURE_SIGNAL_THRESH){
    client.publish(MOIST1_TOPIC_P, "Soil is wet");
    }
  else{
    client.publish(MOIST1_TOPIC_P, "Low signal, check sensor");
  }
}

void setup_actuators(){
  ledcAttachPin(led_1r, 1); // assign RGB led pins to channels
  ledcAttachPin(led_1g, 2);
  ledcAttachPin(led_1b, 3);
  
  // Initialize channels 
  // channels 0-15, resolution 1-16 bits, freq limits depend on resolution
  // ledcSetup(uint8_t channel, uint32_t freq, uint8_t resolution_bits);
  ledcSetup(1, 12000, 8); // 12 kHz PWM, 8-bit resolution
  ledcSetup(2, 12000, 8);
  ledcSetup(3, 12000, 8);
  ledcWrite(1,0);
  ledcWrite(2,0);
  ledcWrite(3,0); 
  // setup Pump
  pinMode(pump_1,OUTPUT);
  digitalWrite(pump_1,LOW);
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
  seq_id = strtok((char*)message.c_str(),delim);
  Serial.print("Message id: ");
  Serial.println(seq_id);
  cmd = strtok(NULL,delim);
  if (cmd == NULL){
    cmd = seq_id;
    seq_id = "none";
  }
  Serial.print("Command: ");
  Serial.println(cmd);
  cmd_param = strtok(NULL,delim);
  if (cmd_param == NULL){
    cmd_param = "none";
  }
  Serial.print("Command Parameter: ");
  Serial.println(cmd_param);
  
  
  // DIFFERENT TOPICS
  if (strcmp(topic, CREATE_TOPIC_S) == 0){
     create_react(cmd);
  }
  else if (strcmp(topic, RETRIEVE_TOPIC_S) == 0){
     read_react(cmd);
  }
  else if (strcmp(topic, UPDATE_TOPIC_S) == 0){
     update_react(cmd);
  }
  else if (strcmp(topic, DELETE_TOPIC_S) == 0){
     delete_react(cmd);
  }
  else if (strcmp(topic,PING_TOPIC_S) == 0){
    Serial.println("Got ping request");
    ping_response();
  }
  
  else if (strstr(topic,"led_1") != NULL){
    Serial.println("Command for LED1");
    response = led_1_response(cmd,cmd_param, seq_id);
    publish_response(RESPONSE_TOPIC_P,response);
  }
  else if (strstr(topic,"pump_1") != NULL){
    Serial.println("Command for Pump1");
    response = pump_1_response(cmd,cmd_param, seq_id);
    publish_response(RESPONSE_TOPIC_P,response);
 
  }
  else {
    Serial.println("Topic unknown");
    sprintf(response,"%s;ERROR",seq_id);
    publish_response(RESPONSE_TOPIC_P,response);

  }
}
void ping_response(){
  // Not yet implemented
  client.publish(PING_TOPIC_P,CRUD_OK);
}

void create_react(char* cmd){
      if (strcmp(cmd,CRUD_OK) == 0 ){
        Serial.println("Device config successfully sent");
      }
      else {
        Serial.println("Error sending device config"); 
      }
      got_crud_response = true;      
}
void read_react(char* cmd){
     if (strcmp(cmd,CRUD_OK) == 0 ){
        Serial.println("Device config known");
        device_isknown = true;
      }
      else {
        Serial.println("Device config not known"); 
      }    
      got_crud_response = true;      
}
void delete_react(char* cmd){
      if (strcmp(cmd,CRUD_OK) == 0 ){
        Serial.println("Device config deletion successful");
        checkout_successful = true;
      }
      else {
        Serial.println("Device config deletion not successful"); 
      }    
      got_crud_response = true;      
}
void update_react(char* cmd){
      if (strcmp(cmd,CRUD_OK) == 0 ){
        Serial.println("Device config update successful");
      }
      else {
        Serial.println("Device config update not successful"); 
      }    
      got_crud_response = true;     
}


// Changes in LED1: Input command and command parameters and message id all from mqtt message, output: response message
char* led_1_response(char* command, char* command_param, char* seq_id){
  static char reply[20];
  if (strcmp(command,"ON") == 0) {
    Serial.println("Command ON");
    if (rval == 255 &&  gval == 255 && bval == 255){
        sprintf(reply,"%s;NOOP",seq_id);

    }
    else {
      ledcWrite(1,255);
      ledcWrite(2,255);
      ledcWrite(3,255);
      rval = 255;
      gval = 255;
      bval = 255;
      sprintf(reply,"%s;OK",seq_id);
    }
  }
  else if (strcmp(command,"OFF") == 0) {
    Serial.println("Command OFF");
    if (rval == 0 && gval == 0 && bval == 0){
        sprintf(reply,"%s;NOOP",seq_id);
    }
    else {
      ledcWrite(1,0);
      ledcWrite(2,0);
      ledcWrite(3,0);
      rval = 0;
      gval = 0;
      bval = 0;
      sprintf(reply,"%s;OK",seq_id);
    }
  }
  else if (strcmp(command,"rgb") == 0){
    Serial.println("Command RGB");
    Serial.println(command_param);
    char *rgb;
    int old_rval, old_gval, old_bval;
    old_rval = rval;
    old_gval = gval;
    old_bval = bval;
    rgb = strtok(command_param,"=");
    rgb = strtok(NULL,"=");
    rval = atoi(strtok(rgb,","));
    gval = atoi(strtok(NULL,","));
    bval = atoi(strtok(NULL,","));
    if(old_rval == rval && old_gval == gval && old_bval == bval){
      sprintf(reply,"%s;NOOP",seq_id);
    }
    else{
      ledcWrite(1,rval);
      ledcWrite(2,gval);
      ledcWrite(3,bval);
      sprintf(reply,"%s;OK",seq_id);
    }
  }
    else {
      Serial.println("Topic known, command uknown");
      sprintf(reply,"%s;ERROR",seq_id);
 }
 return reply;
}

char* pump_1_response(char* command, char* command_param, char* seq_id){
  static char reply[20];
  if (strcmp(command,"ON") == 0) {
    Serial.println("Command ON");
    if (digitalRead(pump_1) == HIGH){
        sprintf(reply,"%s;NOOP",seq_id);

    }
    else {
      digitalWrite(pump_1,HIGH);
      sprintf(reply,"%s;",seq_id);

    }
  }
  else if (strcmp(command,"OFF") == 0) {
    Serial.println("Command OFF");
    if (digitalRead(pump_1) == LOW ){
        sprintf(reply,"%s;NOOP",seq_id);
    }
    else {
      digitalWrite(pump_1,LOW);
      sprintf(reply,"%s;OK",seq_id);

    }
  }
  else {
      Serial.println("Topic known, command uknown");
      sprintf(reply,"%s;ERROR",seq_id);
 }
 return reply;
}
    
void publish_response(char* topic,char* message){
  Serial.println(message);
  client.publish(topic,message);
}

void to_sleep(){
  Serial.println("Attempting Deletion before deep sleep: Wakeup by button or rst");
  device_configuration("delete");
  if(checkout_successful == true) {
   checkout_successful = false;
   Serial.println("Checkout successful, going to sleep");
   esp_deep_sleep_start();
  }
  else{
   Serial.println("Checkout unsuccessful, sent error to mqtt, going to sleep anyways");
   client.publish(ERROR_P,UUID"; Error deleting configuration");
   esp_deep_sleep_start();
  }
}

void loop() {
  if (!client.connected()) {
    Serial.println("Lost MQTT-connection. Reconnecting");
    if( WiFi.status() != WL_CONNECTED){
      Serial.println("Lost Wifi-Connection as well. Reconnecting");
      setup_wifi();
    }
    setup_mqtt();
  }
  client.loop();
  out_brightness_1();
  out_pressure_1();
  out_humidity_1();
  out_temperature_1();
  if (millis() - raintime) < 5000){
    // Skip this
  }
  else {
    raintime = millis();
    out_rain_1();
  }
  out_moisture_1();
  if (millis() - pingtime) < 300000){
      //Skip the ping
  }
  else {
    pingtime = millis();
    client.publish(HELLO_TOPIC_P,UUID";hello");
  }
  delay(1000);


}
