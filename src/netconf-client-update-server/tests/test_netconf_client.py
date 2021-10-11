import pytest

from myno_web_app import netconf_client

device_id = "ABC-123-DEF-456"
device_cat = "ESP32"

"""
get_devices()
"""

def test_single_sensor_device_parsing(mocker):
  mocker_config = {
    'get.return_value': """<?xml version="1.0" encoding="UTF-8"?><nc:rpc-reply xmlns:nc="urn:ietf:params:xml:ns:netconf:base:1.0" message-id="urn:uuid:f8fd91ab-cfa8-4131-acb7-fd6b38e661cc">
                            <data>
                              <device xmlns="https://www.cs.uni-potsdam.de/bs/research/myno/""" + device_cat + """">
                                <device-id>
                                  <uuid>""" + device_id + """</uuid>
                                </device-id>
                                <device-category>""" + device_cat + """</device-category>
                              </device>
                            </data>
                          </nc:rpc-reply>""",
  'get_schema.return_value': """<?xml version="1.0" encoding="UTF-8"?><nc:rpc-reply xmlns:nc="urn:ietf:params:xml:ns:netconf:base:1.0" message-id="urn:uuid:37f38f3c-68c4-4501-a836-06d266c077f6">
                                  <data xmlns="urn:ietf:params:xml:ns:yang:ietf-netconf-monitoring"><![CDATA[module mqtt-""" + device_cat + """ {
                                  namespace "https://www.cs.uni-potsdam.de/bs/research/myno/""" + device_cat + """";
                                  prefix """ + device_cat + """;

                                  container device {
                                    description
                                      "MQTT-Device identified by UUID";
                                    list device-id {
                                      key "uuid";
                                      leaf uuid {
                                        default \"""" + device_id + """";
                                        type string;
                                      }
                                    }
                                    leaf device-category {
                                      description
                                        "Identifies the device category";
                                      type string;
                                    }
                                    container telemetry {
                                      leaf funcGetCO2Concentration {
                                        description
                                          "Get CO2 concentration from sensor";
                                        container datapoint {
                                          leaf funcGetCO2Concentration0 {
                                            default "sensor/co2/co2_1/""" + device_id + """";
                                            type string;
                                            units "partsPerMillion;ppm";
                                          }
                                        }
                                      }
                                    }
                                  }
                                }
                                ]]></data>
                                </nc:rpc-reply>""" }
  mocker.patch('myno_web_app.netconf_client._netconf_manager', **mocker_config)

  res = netconf_client.get_devices()

  assert res == { device_id: (device_cat, {'rpcs': {}, 'sensors': {'funcGetCO2Concentration': ['sensor/co2/co2_1/' + device_id, 'ppm']}})}