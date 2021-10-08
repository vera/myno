device_dict_pa = { "110A-4F4F-8F69-6B8F3C2E78ED": ["Board10",
                    { "rpcs": {
                      "funcConfMoisture": ["configure moisture sensor for events", "input_6_ConfCrud", "enumeration", [["input_1_ConfEvName", "string"], ["input_2_ConfOperator", ["<", "<=", "==", ">", ">="]], ["input_3_ConfMoisture", "int", ["0", "100"]], ["input_4_ConfInterval", "int"], ["input_5_ConfDuration", "int"], ["input_6_ConfCrud", ["CREATE", "DELETE", "READ", "UPDATE"]]], None],
                      "funcAutoMoisture": ["automation rule for moisture sensor", "inputAutoMoisture", "string", [["inputAutoMoisture", "string"]], None],
                      "funcLed_1Rgb": ["set RGB values for led 1", "rgbinput", "union", [["inputBlue", ["0", "255"]], ["inputGreen", ["0", "255"]], ["inputRed", ["0", "255"]]], None],
                      "funcLed_1Off": ["turn led 1 off", [], [], [], None],
                      "funcPump_1On": ["turn pump 1 on", [], [], [], None],
                      "funcPump_1Off": ["turn pump 1 off", [], [], [], None],
                      "funcLed_1On": ["turn led 1 on", [], [], [], None],
                      "funcConfTemperature": ["configure event for temperature sensor", "input_6_ConfCrud", "enumeration", [["input_1_ConfEvName", "string"], ["input_2_ConfOperator", ["<", "<=", "==", ">", ">="]], ["input_3_ConfTemperature", "int", ["-20", "100"]], ["input_4_ConfInterval", "int"], ["input_5_ConfDuration", "int"], ["input_6_ConfCrud", ["CREATE", "DELETE", "READ", "UPDATE"]]], None]
                    }, "sensors": {
                      "funcEvMoisture": ["event/sensor/moisture/moisture_1/110A-4F4F-8F69-6B8F3C2E78ED", ""],
                      "funcGetMoisture": ["sensor/moisture/moisture_1/110A-4F4F-8F69-6B8F3C2E78ED", "%"],
                      "funcEvTemperature": ["event/sensor/temperature/temperature_1/110A-4F4F-8F69-6B8F3C2E78ED", ""],
                      "funcGetPressure": ["sensor/pressure/pressure_1/110A-4F4F-8F69-6B8F3C2E78ED", "hPa"],
                      "funcGetHumidity": ["sensor/humidity/humidity_1/110A-4F4F-8F69-6B8F3C2E78ED", "%"],
                      "funcGetTemperature": ["sensor/temperature/temperature_1/110A-4F4F-8F69-6B8F3C2E78ED", "\u00b0C"],
                      "funcGetRainDetect": ["sensor/rain/rain_1/110A-4F4F-8F69-6B8F3C2E78ED", "%"],
                      "funcGetBrightness": ["sensor/brightness/brightness_1/110A-4F4F-8F69-6B8F3C2E78ED", "lx"]}
                    }] }