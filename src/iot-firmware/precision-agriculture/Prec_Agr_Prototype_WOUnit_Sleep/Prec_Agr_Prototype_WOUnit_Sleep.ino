#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <BH1750.h>
#include <string.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

// DEFINITIONS_
// Debug
//#define DEBUG 1
// OVERALL
#define UUID BOARD"-8A12-4F4F-8F69-6B8F3C2E78ED"
#define BOARD "PrecAgr_longterm"
//timing powerbank
#define HELLO_TOPIC_P "hello/"UUID
#define BOOT_TOPIC_P "boot/"UUID

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

// ALARM TOPIC
#define BRIGHT1_TOPIC_ALARM "alarm/sensor/brightness/brightness_1/"UUID

// SENSOR CONFIG TOPICS
#define BRIGHT1_CONFIG "config/sensor/brightness/brightness_1/"UUID
#define TEMP1_CONFIG "config/sensor/temperature/temperature_1/"UUID
#define HUM1_CONFIG "config/sensor/humidity/humidity_1/"UUID
#define PRES1_CONFIG "config/sensor/pressure/pressure_1/"UUID
#define RAIN1_CONFIG "config/sensor/rain/rain_1/"UUID
#define MOIST1_CONFIG "config/sensor/moisture/moisture_1/"UUID

// CRUD RESPONSES
#define CRUD_OK "OK"
#define CRUD_NOTOK "NOTFOUND" // not used

//General ERROR TOPIC
#define ERROR_P "error"

// SLEEP
#define uS_TO_S_FACTOR 1000000UL  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  3600UL      /* Time ESP32 will go to sleep (in seconds) */

// TIMING
#define CONNECTION_TIMEOUT 30000 
#define MINUTE 60000
// Old Thresholds for Analog Sensors
/*#define RAIN_THRESH 2000
#define RAIN_SIGNAL_THRESH 1000
#define MOISTURE_SIGNAL_THRESH 500
#define MOISTURE_MOIST_THRESH 3200
#define MOISTURE_WET_THRESH 2600
*/
// ENDPOINTS for analog sensors
#define MOISTURE_AIR 4095
#define MOISTURE_WATER 2018
#define RAIN_DRY 4095
#define RAIN_WET 1040
// Timing 

//Alarm Sensor Thresholds;
int moisture_min = 0;
int moisture_max = 0;
bool moisture_min_alarm = false;
bool moisture_max_alarm = false;

int rain_min = 0;
int rain_max = 0;
bool rain_min_alarm = false;
bool rain_max_alarm = false;

int bright_min = 0;
int bright_max = 0;
bool bright_min_alarm = false;
bool bright_max_alarm = false;

int temp_min = 0 ;
int temp_max = 0;
bool temp_max_alarm = false;
bool temp_min_alarm = false;

int pressure_min = 0;
int pressure_max = 0;
bool pressure_min_alarm = false;
bool pressure_max_alarm = false;

int humidity_min = 0;
int humidity_max = 0;
bool humidity_min_alarm = false;
bool humidity_max_alarm = false;

//Sensor intervals (in seconds * 1000 to get milliseconds);

unsigned int moisture_interv = 5 * 1000;
unsigned long moisture_last = 0;
unsigned int rain_interv = 5 * 1000; // specification says minimum are 5 minutes (300sec)
unsigned long rain_last = 0;
unsigned int bright_interv = 5 * 1000;
unsigned long bright_last = 0;
unsigned int temp_interv = 5 * 1000;
unsigned long temp_last = 0;
unsigned int pressure_interv = 5 * 1000;
unsigned long pressure_last = 0;
unsigned int humidity_interv = 5 * 1000;
unsigned long humidity_last = 0;
 
// Sensor Measurements

float brightn = 0;
float moisture = 0;
float rain = 0;
float temp = 0;
float pressure = 0;
float humidity = 0;

// Actuator pump
unsigned int pumping_time = 5 * 1000;
unsigned int pumping_cooloff = 5 * 1000;
unsigned long pumping_last = 0;
int pump_min = 15;
int pump_max = 50;
bool needtopump = false;

//Wifi & MQTT Uni
//const char* ssid = "IoTSens";
//const char* password =  "heterogenIstAnstrengend!";
//const char* mqtt_server = "192.168.43.141";
// für bugfixing
const char* ssid = "FRITZ!Box 7590 LL";
const char* password =  "36122480101678560091";
//const char* mqtt_server = "192.168.178.52";
//pi as bridge
const char* mqtt_server = "192.168.178.3";

WiFiClient Board1;
PubSubClient client(Board1);

// Sleeping time
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
  #ifdef DEBUG
  //delay(500);
  #endif DEBUG
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  #ifdef DEBUG
  Serial.begin(115200);
  #endif
  if (bootCount == 0){
    #ifdef DEBUG
    Serial.println("Starting Sketch");
    #endif
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
    #ifdef DEBUG  
    Serial.println("Device woken up");
    Serial.print("Boot number: ");
    Serial.println(bootCount);
    #endif
    device_configuration("retrieve");
    //device_configuration("delete");
    //device_configuration("update");
    if(!device_isknown){
        publish_config(CREATE_TOPIC_P);
        crud_response_wait("create");
      }
    
  }
    // First Measurements
    char bootnum[5];
    sprintf(bootnum,"%d",bootCount);
    client.publish(HELLO_TOPIC_P,"ping");
    client.publish(BOOT_TOPIC_P,bootnum);
    out_brightness_1();
    out_temp_1();
    out_pressure_1();
    out_humidity_1();
    out_moisture_1();
    out_rain_1();
}

//Connect to Wifi
void setup_wifi() {
  #ifdef DEBUG
  Serial.println();   
  Serial.println("Attempting Wifi-Connection: ");
  #endif
  delay(10);
  unsigned long starttime = millis();
  // We start by connecting to a WiFi network
  #ifdef DEBUG
  Serial.print("Attempting WiFi connection to: ");
  Serial.println(ssid);
  #endif
  WiFi.begin(ssid, password);
   while (WiFi.status() != WL_CONNECTED && (millis()-starttime) < CONNECTION_TIMEOUT) {
      #ifdef DEBUG
      delay(100);
      Serial.print(".");
      #endif
    }
    if (WiFi.status() != WL_CONNECTED) {
    #ifdef DEBUG
    Serial.println("");
    Serial.println("Unable to attain Wifi connection, going to sleep, press rst to start again.");
    #endif
    esp_deep_sleep_start();
    }
  #ifdef DEBUG
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  #endif
}


//Connection to MQTT
void setup_mqtt(){
    client.setServer(mqtt_server, 1883);
    client.setCallback(callback);
    unsigned long starttime = millis();
    Serial.println();
    #ifdef DEBUG
    Serial.println("Attempting MQTT-Connection: ");
    #endif
    while (!client.connected() && (millis()-starttime) < CONNECTION_TIMEOUT) {
        #ifdef DEBUG
        Serial.print(".");
        #endif
        // Attempt to connect
        if (client.connect(UUID)) {
          break;
        } 
        else {
          #ifdef DEBUG
          Serial.print("failed, response code: ");
          Serial.print(client.state());
          Serial.println(" trying again in 5 seconds");
          #endif
          // Wait 5 seconds before retrying
          //delay(5000);
          #ifdef DEBUG
          Serial.println("Attempting MQTT-Connection: ");
          #endif
        }
    }
    if(!client.connected()){
      #ifdef DEBUG
       Serial.println("Unable to attain MQTT connection, going to sleep, press rst to start again.");
       #endif
       esp_deep_sleep_start();
    }
    #ifdef DEBUG
    Serial.println("MQTT connected.");
    #endif
    client.subscribe(LED1_TOPIC_S);
    client.subscribe(PUMP1_TOPIC_S);
    #ifdef DEBUG
    Serial.println("Subscribed to: "LED1_TOPIC_S);
    #endif
    client.subscribe(CREATE_TOPIC_S);
    client.subscribe(RETRIEVE_TOPIC_S);
    client.subscribe(UPDATE_TOPIC_S);
    client.subscribe(DELETE_TOPIC_S);
    #ifdef DEBUG
    Serial.println("Subscribed to CRUD Topics: "CRUD_TOPICS_S);
    #endif
    client.subscribe(PING_TOPIC_S);
    #ifdef DEBUG
    Serial.println("Subscribed to PING Topic: "PING_TOPIC_S);
    #endif
    client.subscribe(BRIGHT1_CONFIG);
    client.subscribe(TEMP1_CONFIG);
    client.subscribe(HUM1_CONFIG);
    client.subscribe(PRES1_CONFIG);
    client.subscribe(RAIN1_CONFIG);
    client.subscribe(MOIST1_CONFIG);
}

void device_configuration(char* operation){
  // OPERATION: CREATE; RETRIEVE; UPDATE; DELETE
  // FIRST IS ALWAYS RETRIEVE
  device_isknown = false;
  client.publish(RETRIEVE_TOPIC_P,UUID);
  crud_response_wait("retrieve");
  if (strcmp(operation,"create") == 0){ 
    if(device_isknown){
      #ifdef DEBUG
      Serial.println("Aborting Configuration Publication. Device already known. Updating instead.");
      #endif
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
      #ifdef DEBUG
      Serial.println("Device unknown, publishing config instead");
      #endif
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
      #ifdef DEBUG
      Serial.println("Device unknown, deletion impossible");
      #endif
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
      #ifdef DEBUG
      //Serial.print("Waiting for CRUD-Reponse for: ");
      //Serial.println(Operation);
      //delay(1000);
      #endif
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
  //String(device_uuid+";"+ontology).toCharArray(configuration,sizeof(configuration));
}

// Publication of Device Description called within device_configuration
void publish_config(char* topic){
  #ifdef DEBUG
  Serial.println();   
  Serial.println("Publishing Device Description: ");  
  #endif
  client.publish(topic,String(device_uuid+";"+ontology).c_str());

  // Create END-Message for configuration and send
  //String(device_uuid+";END").toCharArray(configuration_end,sizeof(configuration_end));
  client.publish(topic,UUID";END");
  #ifdef DEBUG
  Serial.println("Ontology published.");
  #endif
}

void setup_sensors(){
   // Sensors are defined globally
   // BH1750: brigthness_1
   // Wire.begin takes two parameters: SDA pin and SCL pin;
   Wire.begin(brightness_sda,brightness_scl);
   hum_pres_temp_1.begin(0x76, &Wire);
   hum_pres_temp_1.setSampling(Adafruit_BME280::MODE_FORCED,
                    Adafruit_BME280::SAMPLING_X1, // temperature
                    Adafruit_BME280::SAMPLING_X1, // pressure
                    Adafruit_BME280::SAMPLING_X1, // humidity
                    Adafruit_BME280::FILTER_OFF   );

   #ifdef DEBUG
   Serial.println("BME280 Set up");
   #endif
   brightness_1.begin(BH1750::ONE_TIME_HIGH_RES_MODE_2);
   #ifdef DEBUG
   Serial.println("BH1750 High Resolution Mode");
   #endif

}

// Measure brightness and ouput to UI and SSN-Topic
void out_brightness_1(){
    char lux_msg[15];
    brightn =  brightness_1.readLightLevel();
    delay(120);
    //dtostrf(lux, 5, 2, lux_char);
   //dtostrf(lux, 5, 2, lux_char);
    sprintf(lux_msg, "%.1f", brightn);
    client.publish(BRIGHT1_TOPIC_P, lux_msg);
    if(bright_max_alarm && brightn > bright_max){
      client.publish(BRIGHT1_TOPIC_ALARM,"Brightness too high");
    }
    else if (bright_min_alarm && brightn < bright_min){
      client.publish(BRIGHT1_TOPIC_ALARM,"Brightness too low");
    }
    bright_last = millis();

}
// Measure all three
/*void out_bme_1(){
  char bme_msg[15];
  float result;
  result = hum_pres_temp_1.readHumidity();
  sprintf(bme_msg, "%.2f", result);
  client.publish(HUM1_TOPIC_P, bme_msg);
  result = hum_pres_temp_1.readPressure() / 100.00;
  sprintf(bme_msg, "%.2f", result);
  client.publish(PRES1_TOPIC_P, bme_msg);
  result = hum_pres_temp_1.readTemperature();
  sprintf(bme_msg, "%.2f", result);
  client.publish(TEMP1_TOPIC_P, bme_msg);
}*/

void out_temp_1(){
    char temp_msg[15];
    temp = hum_pres_temp_1.readTemperature();
    sprintf(temp_msg, "%.2f", temp);
    client.publish(TEMP1_TOPIC_P, temp_msg);
    temp_last = millis();
}

void out_pressure_1(){
    char pressure_msg[15];
    pressure = hum_pres_temp_1.readPressure()/100.00;
    sprintf(pressure_msg, "%.2f", pressure);
    client.publish(PRES1_TOPIC_P, pressure_msg);
    pressure_last = millis();
}

void out_humidity_1(){
    char humidity_msg[15];
    humidity = hum_pres_temp_1.readHumidity();
    sprintf(humidity_msg, "%.2f", humidity);
    client.publish(HUM1_TOPIC_P, humidity_msg);
    humidity_last = millis();
}
void out_rain_1(){
    bool sent;
    char rain_msg[15];
    int rain_analog = analogRead(rain_out);
    rain = float(100) - float(rain_analog-RAIN_WET) / float(RAIN_DRY-RAIN_WET) * 100;
    if (rain > 100){
      rain = 100;
    }
    #ifdef DEBUG
    Serial.print("Rain ");
    Serial.println(rain);
    #endif
    sprintf(rain_msg, "%.2f", rain);
    sent = client.publish(RAIN1_TOPIC_P, rain_msg);
    rain_last = millis();
}

void out_moisture_1(){
    char moisture_msg[15];
    int moisture_analog = analogRead(moisture_out);
    moisture = float(100) - float(moisture_analog-MOISTURE_WATER) / float(MOISTURE_AIR-MOISTURE_WATER) * 100;
    if (moisture > 100){
      moisture = 100;
    }
      #ifdef DEBUG
      //Serial.print("Moisture ");
      //Serial.println(moisture);
      #endif
      sprintf(moisture_msg, "%.2f", moisture);
      client.publish(MOIST1_TOPIC_P,moisture_msg);
      moisture_last = millis();
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
  #ifdef DEBUG
  Serial.print(topic);
  //Serial.print(". Message: ");
  Serial.println(message);
  #endif
  // Retrieve parts of message: For RPC seq_id is a message id, otherwise the first part of the message is the command. 
  char *seq_id, *cmd, *cmd_param, *response;
  const char delim[] = ";";
  seq_id = strtok((char*)message.c_str(),delim);
  #ifdef DEBUG
  Serial.print("Message id: ");
  Serial.println(seq_id);
  #endif
  cmd = strtok(NULL,delim);
  if (cmd == NULL){
    cmd = seq_id;
    seq_id = "none";
  }
  #ifdef DEBUG
  Serial.print("Command: ");
  Serial.println(cmd);
  #endif
  cmd_param = strtok(NULL,delim);
  if (cmd_param == NULL){
    cmd_param = "none";
  }
  #ifdef DEBUG
  Serial.print("Command Parameter: ");
  Serial.println(cmd_param);
  #endif
  
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
    #ifdef DEBUG
    Serial.println("Got ping request");
    #endif
    ping_response();
  }
  
  else if (strstr(topic,"led_1") != NULL){
    #ifdef DEBUG
    Serial.println("Command for LED1");
    #endif
    response = led_1_response(cmd,cmd_param, seq_id);
    publish_response(RESPONSE_TOPIC_P,response);
  }
  else if (strstr(topic,"pump_1") != NULL){
    #ifdef DEBUG
    Serial.println("Command for Pump1");
    #endif
    response = pump_1_response(cmd,cmd_param, seq_id);
    publish_response(RESPONSE_TOPIC_P,response);
    check_pump();
 
  }
  else if (strcmp(topic,BRIGHT1_CONFIG) == 0){
    if (strcmp(cmd,"interval") == 0){
      response = sensor_config_interval(topic, cmd, cmd_param,seq_id,bright_interv);
      publish_response(RESPONSE_TOPIC_P,response);
    }
    else {
      sprintf(response,"%s;ERROR",seq_id);
      publish_response(RESPONSE_TOPIC_P,response);

    }
  }
  else if (strcmp(topic,TEMP1_CONFIG) == 0){
    if (strcmp(cmd,"interval") == 0){
      response = sensor_config_interval(topic, cmd, cmd_param,seq_id,temp_interv);
      publish_response(RESPONSE_TOPIC_P,response);
    }
    else {
      sprintf(response,"%s;ERROR",seq_id);
      publish_response(RESPONSE_TOPIC_P,response);

    }
  }
  else if (strcmp(topic,PRES1_CONFIG) == 0){
    if (strcmp(cmd,"interval") == 0){
      response = sensor_config_interval(topic, cmd, cmd_param,seq_id,pressure_interv);
      publish_response(RESPONSE_TOPIC_P,response);
    }
    else {
      sprintf(response,"%s;ERROR",seq_id);
      publish_response(RESPONSE_TOPIC_P,response);

    }
  }
  else if (strcmp(topic,HUM1_CONFIG) == 0){
    if (strcmp(cmd,"interval") == 0){
      response = sensor_config_interval(topic, cmd, cmd_param,seq_id,humidity_interv);
      publish_response(RESPONSE_TOPIC_P,response);
    }
    else {
      sprintf(response,"%s;ERROR",seq_id);
      publish_response(RESPONSE_TOPIC_P,response);

    }
  }
  else if (strcmp(topic,MOIST1_CONFIG) == 0){
    if (strcmp(cmd,"interval") == 0){
      response = sensor_config_interval(topic, cmd, cmd_param,seq_id,moisture_interv);
      publish_response(RESPONSE_TOPIC_P,response);
    }
    else {
      sprintf(response,"%s;ERROR",seq_id);
      publish_response(RESPONSE_TOPIC_P,response);

    }
  }
  else if (strcmp(topic,RAIN1_CONFIG) == 0){
    if (strcmp(cmd,"interval") == 0){
      response = sensor_config_interval(topic, cmd, cmd_param,seq_id,rain_interv);
      publish_response(RESPONSE_TOPIC_P,response);
    }
    else {
      sprintf(response,"%s;ERROR",seq_id);
      publish_response(RESPONSE_TOPIC_P,response);

    }
  }
 
  else {
    #ifdef DEBUG
    Serial.println("Topic unknown");
    #endif
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
        #ifdef DEBUG
        Serial.println("Device config successfully sent");
        #endif
      }
      else {
        #ifdef DEBUG
        Serial.println("Error sending device config"); 
        #endif
      }
      got_crud_response = true;      
}
void read_react(char* cmd){
     if (strcmp(cmd,CRUD_OK) == 0 ){
        #ifdef DEBUG
        Serial.println("Device config known");
        #endif
        device_isknown = true;
      }
      else {
        #ifdef DEBUG
        Serial.println("Device config not known"); 
        #endif
      }    
      got_crud_response = true;      
}
void delete_react(char* cmd){
      if (strcmp(cmd,CRUD_OK) == 0 ){
        #ifdef DEBUG
        Serial.println("Device config deletion successful");
        #endif
        checkout_successful = true;
      }
      else {
        #ifdef DEBUG
        Serial.println("Device config deletion not successful"); 
        #endif
      }    
      got_crud_response = true;      
}
void update_react(char* cmd){
      if (strcmp(cmd,CRUD_OK) == 0 ){
        #ifdef DEBUG
        Serial.println("Device config update successful");
        #endif
      }
      else {
        #ifdef DEBUG
        Serial.println("Device config update not successful"); 
        #endif
      }    
      got_crud_response = true;     
}


// Changes in LED1: Input command and command parameters and message id all from mqtt message, output: response message
char* led_1_response(char* command, char* command_param, char* seq_id){
  static char reply[20];
  if (strcmp(command,"ON") == 0) {
    #ifdef DEBUG
    Serial.println("Command ON");
    #endif
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
    #ifdef DEBUG
    Serial.println("Command OFF");
    #endif
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
    #ifdef DEBUG
    Serial.println("Command RGB");
    Serial.println(command_param);
    #endif
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
      #ifdef DEBUG
      Serial.println("Topic known, command uknown");
      #endif
      sprintf(reply,"%s;ERROR",seq_id);
 }
 return reply;
}

char* pump_1_response(char* command, char* command_param, char* seq_id){
  static char reply[20];
  if (strcmp(command,"ON") == 0) {
    #ifdef DEBUG
    Serial.println("Command ON");
    #endif
    if (digitalRead(pump_1) == HIGH){
        sprintf(reply,"%s;NOOP",seq_id);

    }
    else {
      digitalWrite(pump_1,HIGH);
      sprintf(reply,"%s;OK",seq_id);
    }
  }
  else if (strcmp(command,"OFF") == 0) {
    #ifdef DEBUG
    Serial.println("Command OFF");
    #endif
    if (digitalRead(pump_1) == LOW ){
        sprintf(reply,"%s;NOOP",seq_id);
    }
    else {
      digitalWrite(pump_1,LOW);
      sprintf(reply,"%s;OK",seq_id);

    }
  }
  else {
      #ifdef DEBUG
      Serial.println("Topic known, command uknown");
      #endif
      sprintf(reply,"%s;ERROR",seq_id);
 }
 return reply;
}


char* sensor_config_interval(char* topic, char* command, char* command_param, char* seq_id, unsigned int &sensor_interval){
  static char reply[20];
  //if (strcmp(command,"interval") == 0) {
    int interval;
    command_param = strtok(command_param,"=");
    interval = atoi(strtok(NULL,"="));    
    if ((interval * 1000) == sensor_interval){
      sprintf(reply,"%s;NOOP",seq_id);
      #ifdef DEBUG
      Serial.print(command);
      Serial.print(" for sensor ");
      topic = strtok(topic,"/");
      topic = strtok(NULL,"/");
      topic = strtok(NULL,"/");
      topic = strtok(NULL,"/");
      Serial.print(topic);
      Serial.print(" unchanged. Same value as before: ");
      Serial.print(interval);
      Serial.println(" seconds");
      #endif
    }
    else{
      sensor_interval = interval * 1000;
      sprintf(reply,"%s;OK",seq_id);
      #ifdef DEBUG
      Serial.print("Changed ");
      Serial.print(command);
      Serial.print(" for sensor ");
      topic = strtok(topic,"/");
      topic = strtok(NULL,"/");
      topic = strtok(NULL,"/");
      topic = strtok(NULL,"/");
      Serial.print(topic);
      Serial.print(" to ");
      Serial.print(interval);
      Serial.println(" seconds");
      #endif

    }

  /*}
  else{
    sprintf(reply,"%s;ERROR",seq_id);
  }*/
  return reply;
}

void publish_response(char* topic,char* message){
  #ifdef DEBUG
  Serial.println(message);
  #endif
  client.publish(topic,message);
}

void to_sleep(){
  #ifdef DEBUG
  Serial.println("Disconnecting MQTT and WIFI. Going to sleep using timer.");
  #endif
  client.disconnect();
  WiFi.disconnect();
  while(client.connected() || WiFi.status() != WL_CONNECTED ) {
    delay(10);
  }
/*device_configuration("delete");
  if(checkout_successful == true) {
   checkout_successful = false;
   Serial.println("Checkout successful, going to sleep");
   esp_deep_sleep_start();
  }
  else{
   Serial.println("Checkout unsuccessful, sent error to mqtt, going to sleep anyways");
   client.publish(ERROR_P,UUID"; Error deleting configuration");
   esp_deep_sleep_start();
  }*/
  #ifdef DEBUG
  Serial.println("Sleeping");
  #endif
  esp_deep_sleep_start();
}
void check_pump(){
  if (digitalRead(pump_1) == HIGH){
    delay(pumping_time);
    digitalWrite(pump_1,LOW);
  }
}

void autom_pump(){
  out_moisture_1();
  Serial.print("Moisture: ");
  Serial.println(moisture);
  if (pump_min > moisture && needtopump == false){
    needtopump = true;
  }
  else if (pump_max < moisture && needtopump == true){
    needtopump = false;
  }
  else if (needtopump == true && (millis() - pumping_last > pumping_cooloff)){
    digitalWrite(pump_1,HIGH);
    delay(pumping_time);
    digitalWrite(pump_1,LOW);
    pumping_last = millis();
  }
  
}
void publish_measurements(){
unsigned long now = millis();
  if((now - bright_last) >= bright_interv){
    out_brightness_1();
  }
  if((now - temp_last) >= temp_interv){
    out_temp_1();
  }
  if((now - humidity_last) >= humidity_interv){
    out_humidity_1();
  }
  if((now - pressure_last) >= pressure_interv){
    out_pressure_1();
  }
  if((now - moisture_last) >= moisture_interv){
    out_moisture_1();
  }
  if((now - rain_last) >= rain_interv){
    out_rain_1();
  }
  //delay(1000);
}

void loop() {
  /*if (!client.connected()) {
    #ifdef DEBUG
    Serial.println("Lost MQTT-connection. Reconnecting");
    #endif
    if( WiFi.status() != WL_CONNECTED){
      #ifdef DEBUG
      Serial.println("Lost Wifi-Connection as well. Reconnecting");
      #endif
      setup_wifi();
    } 
    setup_mqtt();
  }*/
  //delay(10000);
  publish_measurements();

  client.publish(HELLO_TOPIC_P,"ping");
  client.loop();
  delay(250);
  to_sleep();
  //autom_pump();
  //check_pump();
  //delay(1000);


}
