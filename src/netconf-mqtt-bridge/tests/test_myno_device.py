import pytest

import myno_device as md
from myno_device_description_errors import *

def test_no_device():
	with pytest.raises(NotASingleDeviceError, match="contains 0 devices"):
		device = md.MynoDevice("{}")

def test_multiple_devices():
	with pytest.raises(NotASingleDeviceError, match="contains 2 devices"):
		device = md.MynoDevice('[{"@context":{"onem2m": "http://www.onem2m.org/ontology/Base_Ontology/base_ontology#"},"@id":"base:myDevice","@type":"onem2m:Device"},{"@context":{"onem2m": "http://www.onem2m.org/ontology/Base_Ontology/base_ontology#"},"@id":"base:myDevice2","@type":"onem2m:Device"}]')

def test_missing_device_uuid():
	with pytest.raises(MissingDeviceUuidOrCategoryError):
		device = md.MynoDevice('{"@context":{"onem2m": "http://www.onem2m.org/ontology/Base_Ontology/base_ontology#"},"@id":"base:myDevice","@type":"onem2m:Device","onem2m:hasThingProperty":[{"@id":"base:deviceCategory","@type":"onem2m:ThingProperty","onem2m:hasValue":"TestDevices"}]}')

def test_missing_device_category():
	with pytest.raises(MissingDeviceUuidOrCategoryError):
		device = md.MynoDevice('{"@context":{"onem2m": "http://www.onem2m.org/ontology/Base_Ontology/base_ontology#"},"@id":"base:myDevice","@type":"onem2m:Device","onem2m:hasThingProperty":[{"@id":"base:deviceUuid","@type":"onem2m:ThingProperty","onem2m:hasValue":"MY-DEVICE"}]}')

def test_MUP():
	with open("tests/assets/ontology_mup.jsonld") as ontology:
		device = md.MynoDevice(ontology.read())

		assert device.uuid == "TEST-DEVICE-MUP" 
		assert device.description == "MQTT device that accepts firmware updates"
		assert device.category == "TestDevices"
		assert device.rpcs == [
			{
				"name": "funcGetDeviceToken",
				"description": "Get token for MYNO update",
				"mqttTopic": "yang/update/token",
				"mqttMethod": "GET-DEVICE-TOKEN",
				"inputs": [
					{
						"name": "uuidInput",
						"description": "Target UUID for request",
						"datatype": "string"
					}
				],
				"returnStates": [
					{
						"name": "10_OK",
						"description": "10 successful"
					},
					{
						"name": "11_NOOP",
						"description": "11 nothing to do"
					},
					{
						"name": "12_ERROR",
						"description": "12 error"
					}
				]
			},
			{
				"name": "funcPubUpdateImage",
				"description": "Publish update image",
				"mqttTopic": "yang/update/image",
				"mqttMethod": "PUB-UPDATE-IMAGE",
				"inputs": [
					{
						"name": "inputUpdateImage",
						"description": "Update image",
						"datatype": "hexBinary"
					},
					{
						"name": "uuidInput",
						"description": "Target UUID for request",
						"datatype": "string"
					}
				],
				"returnStates": [
					{
						"name": "10_OK",
						"description": "10 successful"
					},
					{
						"name": "11_NOOP",
						"description": "11 nothing to do"
					},
					{
						"name": "12_ERROR",
						"description": "12 error"
					}
				]
			},
			{
				"name": "funcPubUpdateManifest",
				"description": "Publish update manifest",
				"mqttTopic": "yang/update/manifest",
				"mqttMethod": "PUB-UPDATE-MANIFEST",
				"inputs": [
					{
						"name": "input_01_AppId",
						"description": "App ID",
						"datatype": "string"
					},
					{
						"name": "input_02_LinkOffset",
						"description":
						"Link offset",
						"datatype": "int"
					},
					{
						"name": "input_03_Hash",
						"description": "Input hash",
						"datatype": "hexBinary"
					},
					{
						"name": "input_04_Size",
						"description": "Image size",
						"datatype": "int"
					},
					{
						"name": "input_05_Version",
						"description": "Firmware version",
						"datatype": "int"
					},
					{
						"name": "input_06_OldVersion",
						"description": "Old firmware version",
						"datatype": "int"
					},
					{
						"name": "input_07_InnerKeyInfo",
						"description": "Information on key used for inner signature",
						"datatype": "hexBinary"
					},
					{
						"name": "input_08_InnerSignature",
						"description": "Inner signature",
						"datatype": "hexBinary"
					},
					{
						"name": "input_09_DeviceNonce",
						"description": "Nonce",
						"datatype": "int"
					},
					{
						"name": "input_10_OuterKeyInfo",
						"description": "Information on key used for outer signature",
						"datatype": "hexBinary"
					},
					{
						"name": "input_11_OuterSignature",
						"description": "Outer signature",
						"datatype": "hexBinary"
					},
					{
						"name": "uuidInput",
						"description": "Target UUID for request",
						"datatype": "string"
					}
				],
				"returnStates": [
					{
						"name": "10_OK",
						"description": "10 successful"
					},
					{
						"name": "11_NOOP",
						"description": "11 nothing to do"
					},
					{
						"name": "12_ERROR",
						"description": "12 error"
					}
				]
			}
		]
		assert device.telemetry_functions == []

def test_led_and_sensor():
	with open("tests/assets/ontology_led+sensor.jsonld") as ontology:
		device = md.MynoDevice(ontology.read())

		assert device.uuid == "TEST-DEVICE-SENSOR+LED" 
		assert device.description == "Simple MQTT device with 1 sensor and 1 LED"
		assert device.category == "TestDevices"
		assert device.rpcs == [
			{
				"name": "funcLedOff",
				"description": "LED ausschalten",
				"mqttTopic": "led",
				"mqttMethod": "ledOff",
				"inputs": [
					{
						"name": "uuidInput",
						"description": "Target UUID for request",
						"datatype": "string"
					}
				],
				"returnStates": [
					{
						"name": "10_OK",
						"description": "10 successful"
					},
					{
						"name": "11_NOOP",
						"description": "11 nothing to do"
					},
					{
						"name": "12_ERROR",
						"description": "12 error"
					}
				]
			},
			{
				"name": "funcLedOn",
				"description": "LED anschalten",
				"mqttTopic": "led",
				"mqttMethod": "ledOn",
				"inputs": [
					{
						"name": "uuidInput",
						"description": "Target UUID for request",
						"datatype": "string"
					}
				],
				"returnStates": [
					{
						"name": "10_OK",
						"description": "10 successful"
					},
					{
						"name": "11_NOOP",
						"description": "11 nothing to do"
					},
					{
						"name": "12_ERROR",
						"description": "12 error"
					}
				]
			}
		]
		assert device.telemetry_functions == [
			{
				"name": "funcGetTemperature",
				"description": "Gibt die Umgebungstemperatur zurück",
				"outputs": [
					{
						"name": "outDpTemperature",
						"topic": "sensor/temperature/TEST-DEVICE-SENSOR+LED",
						"datatype": "string",
						"unit": "degreeCelsius;°C"
					}
				]
			}
		]

def test_precision_agriculture():
	with open("tests/assets/ontology_precision_agriculture.jsonld") as ontology:
		device = md.MynoDevice(ontology.read())

		assert device.uuid == "TEST-DEVICE-PA" 
		assert device.description == "MQTT device for precision agriculture"
		assert device.category == "TestDevices"
		assert device.rpcs == [
			{
				"name": "funcAutoMoisture",
				"description": "automation rule for moisture sensor",
				"mqttTopic": "automation/sensor/moisture/moisture_1/TEST-DEVICE-PA",
				"mqttMethod": "AUTOMATION",
				"inputs": [
					{
						"name": "inputAutoMoisture",
						"description": "automation for moisture",
						"datatype": "string"
					},
					{
						"name": "uuidInput",
						"description": "Target UUID for request",
						"datatype": "string"
					}
				],
				"returnStates": [
					{
						"name": "ERROR",
						"description": "error"
					},
					{
						"name": "NOOP",
						"description": "nothing to do"
					},
					{
						"name": "OK",
						"description": "successful"
					}
				]
			},
			{
				"name": "funcConfMoisture",
				"description": "configure moisture sensor for events",
				"mqttTopic": "config/sensor/moisture/moisture_1/TEST-DEVICE-PA",
				"mqttMethod": "CONFIGEVENT",
				"inputs": [
					{
						"name": "input_1_ConfEvName",
						"description": "event name caused by configuration",
						"datatype": "string"
					},
					{
						"name": "input_2_ConfOperator",
						"description": "describes configuration operators",
						"datatype": ["<", "<=", "==", ">", ">="]
					},
					{
						"name": "input_3_ConfMoisture",
						"description": "threshold values for moisture sensor",
						"datatype": "int",
						"datatypeRange": ["0", "100"]
					},
					{
						"name": "input_4_ConfInterval",
						"description": "interval for event configuration",
						"datatype": "int"
					},
					{
						"name": "input_5_ConfDuration",
						"description": "duration for event configuration",
						"datatype": "int"
					},
					{
						"name": "input_6_ConfCrud",
						"description": "select CRUD operations for events",
						"datatype": ["CREATE", "DELETE", "READ", "UPDATE"]
					},
					{
						"name": "uuidInput",
						"description": "Target UUID for request",
						"datatype": "string"
					}
				],
				"returnStates": [
					{
						"name": "ERROR",
						"description": "error"
					},
					{
						"name": "NOOP",
						"description": "nothing to do"
					},
					{
						"name": "OK",
						"description": "successful"
					}
				]
			},
			{
				"name": "funcConfTemperature",
				"description": "configure event for temperature sensor",
				"mqttTopic": "config/sensor/temperature/temperature_1/TEST-DEVICE-PA",
				"mqttMethod": "CONFIGEVENT",
				"inputs": [
					{
						"name": "input_1_ConfEvName",
						"description": "event name caused by configuration",
						"datatype": "string"
					},
					{
						"name": "input_2_ConfOperator",
						"description": "describes configuration operators",
						"datatype": ["<", "<=", "==", ">", ">="]
					},
					{
						"name": "input_3_ConfTemperature",
						"description": "threshold values for temperature sensor",
						"datatype": "int",
						"datatypeRange": ["-20", "100"]
					},
					{
						"name": "input_4_ConfInterval",
						"description": "interval for event configuration",
						"datatype": "int"
					},
					{
						"name": "input_5_ConfDuration",
						"description": "duration for event configuration",
						"datatype": "int"
					},
					{
						"name": "input_6_ConfCrud",
						"description": "select CRUD operations for events",
						"datatype": ["CREATE", "DELETE", "READ", "UPDATE"]
					},
					{
						"name": "uuidInput",
						"description": "Target UUID for request",
						"datatype": "string"
					}
				],
				"returnStates": [
					{
						"name": "ERROR",
						"description": "error"
					},
					{
						"name": "NOOP",
						"description": "nothing to do"
					},
					{
						"name": "OK",
						"description": "successful"
					}
				]
			},
			{
				"name": "funcPump_1Off",
				"description": "turn pump 1 off",
				"mqttTopic": "actuator/pump/pump_1/TEST-DEVICE-PA",
				"mqttMethod": "OFF",
				"inputs": [
					{
						"name": "uuidInput",
						"description": "Target UUID for request",
						"datatype": "string"
					}
				],
				"returnStates": [
					{
						"name": "ERROR",
						"description": "error"
					},
					{
						"name": "NOOP",
						"description": "nothing to do"
					},
					{
						"name": "OK",
						"description": "successful"
					}
				]
			},
			{
				"name": "funcPump_1On",
				"description": "turn pump 1 on",
				"mqttTopic": "actuator/pump/pump_1/TEST-DEVICE-PA",
				"mqttMethod": "ON",
				"inputs": [
					{
						"name": "uuidInput",
						"description": "Target UUID for request",
						"datatype": "string"
					}
				],
				"returnStates": [
					{
						"name": "ERROR",
						"description": "error"
					},
					{
						"name": "NOOP",
						"description": "nothing to do"
					},
					{
						"name": "OK",
						"description": "successful"
					}
				]
			}
		]
		assert device.telemetry_functions == [
			{
				"name": "funcEvMoisture",
        		"description": "event function for moisture sensor",
        		"outputs": [
					{
						"name": "outDpEvMoisture",
						"topic": "event/sensor/moisture/moisture_1/TEST-DEVICE-PA",
						"datatype": "string"
					}
				]
			},
			{
				"name": "funcEvTemperature",
        		"description": "event function for temperature sensor",
        		"outputs": [
					{
						"name": "outDpEvTemperature",
						"topic": "event/sensor/temperature/temperature_1/TEST-DEVICE-PA",
						"datatype": "string"
					}
				]
			},
			{
				"name": "funcGetBrightness",
        		"description": "Get brightness from sensor",
        		"outputs": [
					{
						"name": "outDpBrightness",
						"topic": "sensor/brightness/brightness_1/TEST-DEVICE-PA",
						"datatype": "string",
						"unit": "lux;lx"
					}
				]
			},
			{
				"name": "funcGetHumidity",
        		"description": "Get humidity from sensor",
        		"outputs": [
					{
						"name": "outDpHumidity",
						"topic": "sensor/humidity/humidity_1/TEST-DEVICE-PA",
						"datatype": "string",
						"unit": "percent;%%"
					}
				]
			},
			{
				"name": "funcGetMoisture",
        		"description": "Get moisture from sensor",
        		"outputs": [
					{
						"name": "outDpMoisture",
						"topic": "sensor/moisture/moisture_1/TEST-DEVICE-PA",
						"datatype": "string",
						"unit": "percent;%%"
					}
				]
			},
			{
				"name": "funcGetPressure",
        		"description": "Get pressure from sensor",
        		"outputs": [
					{
						"name": "outDpPressure",
						"topic": "sensor/pressure/pressure_1/TEST-DEVICE-PA",
						"datatype": "string",
						"unit": "hectopascal;hPa"
					}
				]
			},
			{
				"name": "funcGetRainDetect",
        		"description": "Get rain detection signal from sensor",
        		"outputs": [
					{
						"name": "outDpRainDetect",
						"topic": "sensor/rain/rain_1/TEST-DEVICE-PA",
						"datatype": "string",
						"unit": "percent;%%"
					}
				]
			},
			{
				"name": "funcGetTemperature",
        		"description": "Get temperature from sensor",
        		"outputs": [
					{
						"name": "outDpTemperature",
						"topic": "sensor/temperature/temperature_1/TEST-DEVICE-PA",
						"datatype": "string",
						"unit": "degreeCelsius;°C"
					}
				]
			}
		]
