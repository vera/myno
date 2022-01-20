import pytest

from netconf import client
from importlib import reload
from lxml.etree import tostring

import config
import netconf_mqtt_bridge
import netconf_methods as ncm

uuids = {
	"mup": "TEST-DEVICE-MUP",
	"sensor+led": "TEST-DEVICE-SENSOR+LED",
	"pa": "TEST-DEVICE-PA"
}
device_category = "TestDevices"
base = "https://www.cs.uni-potsdam.de/bs/research/myno/"


"""
Fixtures
"""

@pytest.fixture(autouse=True, scope="session")
def setup():
	netconf_mqtt_bridge.bridge = netconf_mqtt_bridge.NetconfMqttBridge()
	with open('tests/assets/ontology_mup.jsonld') as ontology:
		netconf_mqtt_bridge.bridge.device_descriptions[uuids["mup"]] = ontology.read()
		netconf_mqtt_bridge.bridge.create_device(uuids["mup"])
	with open('tests/assets/ontology_led+sensor.jsonld') as ontology:
		netconf_mqtt_bridge.bridge.device_descriptions[uuids["sensor+led"]] = ontology.read()
		netconf_mqtt_bridge.bridge.create_device(uuids["sensor+led"])
	with open('tests/assets/ontology_precision_agriculture.jsonld') as ontology:
		netconf_mqtt_bridge.bridge.device_descriptions[uuids["pa"]] = ontology.read()
		netconf_mqtt_bridge.bridge.create_device(uuids["pa"])

@pytest.fixture()
def session():
	yield client.NetconfSSHSession("127.0.0.1", username=config.NC_USERNAME, password=config.NC_PASSWORD, port=config.NC_PORT)

@pytest.fixture
def netconf_methods():
	netconf_methods = ncm.NetconfMethods()
	yield netconf_methods

@pytest.fixture
def mqtt_client(mocker):
	class MqttClient:
		def publish(self):
			pass

	config.SECS_TO_WAIT_FOR_RESPONSE = 0.2

	mocker.patch.object(MqttClient, 'publish')
	mock_client = MqttClient()
	yield mocker.patch('netconf_mqtt_bridge.bridge.mqtt_client', mock_client)


"""
Tests
"""

def test_GET(session, netconf_methods):
	get_res = tostring(session.get()).decode("utf-8")

	expected = '<data xmlns="urn:ietf:params:xml:ns:netconf:base:1.0">'
	for device in uuids:
		expected += """
    <device xmlns="%s%s">
      <device-id>
        <uuid>%s</uuid>
      </device-id>
      <device-category>%s</device-category>
    </device>""" % (base, device_category, uuids[device], device_category)

	expected += """
  </data>
"""

	assert get_res == expected

def test_GET_SCHEMA(session, netconf_methods):
	_, _, get_schema_res = session.send_rpc("<get-schema/>")

	with open('tests/assets/expected_schema.xml') as expected:
		assert get_schema_res == expected.read()

def test_rpc_single_param(session, netconf_methods, mqtt_client):
	req_id = 0
	response = "OK!"
	netconf_mqtt_bridge.bridge.current_req_id = req_id
	netconf_mqtt_bridge.bridge.responses[str(req_id)] = response

	_, _, res = session.send_rpc("<funcGetDeviceToken><uuidInput>%s</uuidInput></funcGetDeviceToken>" % uuids["mup"])

	assert res == (
"""<?xml version="1.0" encoding="UTF-8"?><rpc-reply xmlns="urn:ietf:params:xml:ns:netconf:base:1.0" message-id="%d">
  <data>
    <retval>%s</retval>
  </data>
</rpc-reply>
""" % (req_id, response))
	mqtt_client.publish.assert_called_once()
	assert mqtt_client.publish.call_args.args == ("yang/update/token", str(req_id) + ";GET-DEVICE-TOKEN")

def test_rpc_multiple_params(session, netconf_methods, mqtt_client):
	params = {
		"app_id": "TestApp",
		"offset": 8000,
		"image_hash": "3582958",
		"image_size": 500,
		"version": 2,
		"old_version": 1,
		"inner_signature": "01010101",
		"nonce": 42,
		"outer_signature": "01010101"
	}
	req_id = 0
	response = "OK!"
	netconf_mqtt_bridge.bridge.current_req_id = req_id
	netconf_mqtt_bridge.bridge.responses[str(req_id)] = response

	_, _, res = session.send_rpc(
"""<funcPubUpdateManifest>
  <uuidInput>%s</uuidInput>
  <input_1_AppId>%s</input_1_AppId>
  <input_2_LinkOffset>%d</input_2_LinkOffset>
  <input_3_Hash>%s</input_3_Hash>
  <input_4_Size>%d</input_4_Size>
  <input_5_Version>%d</input_5_Version>
  <input_6_OldVersion>%d</input_6_OldVersion>
  <input_7_InnerSignature>%s</input_7_InnerSignature>
  <input_8_DeviceNonce>%d</input_8_DeviceNonce>
  <input_9_OuterSignature>%s</input_9_OuterSignature>
</funcPubUpdateManifest>
""" % (uuids["mup"], params["app_id"], params["offset"], params["image_hash"], params["image_size"], params["version"], params["old_version"], params["inner_signature"], params["nonce"], params["outer_signature"]))

	assert res == (
"""<?xml version="1.0" encoding="UTF-8"?><rpc-reply xmlns="urn:ietf:params:xml:ns:netconf:base:1.0" message-id="%d">
  <data>
    <retval>%s</retval>
  </data>
</rpc-reply>
""" % (req_id, response))
	mqtt_client.publish.assert_called_once()
	expected_message = str(req_id) + ";PUB-UPDATE-MANIFEST"
	for key in params:
		expected_message += ";" + str(params[key])
	assert mqtt_client.publish.call_args.args == ("yang/update/manifest", expected_message)

def test_rpc_no_response(session, netconf_methods, mqtt_client):
	req_id = 0
	netconf_mqtt_bridge.bridge.current_req_id = req_id
	if str(req_id) in netconf_mqtt_bridge.bridge.responses:
		del netconf_mqtt_bridge.bridge.responses[str(req_id)]

	_, _, res = session.send_rpc("<funcGetDeviceToken><uuidInput>%s</uuidInput></funcGetDeviceToken>" % uuids["mup"])

	assert res == (
"""<?xml version="1.0" encoding="UTF-8"?><rpc-reply xmlns="urn:ietf:params:xml:ns:netconf:base:1.0" message-id="%d">
  <data>
    <retval>NO RESPONSE FROM DEVICE</retval>
  </data>
</rpc-reply>
""" % (req_id))
	mqtt_client.publish.assert_called_once()
	assert mqtt_client.publish.call_args.args == ("yang/update/token", str(req_id) + ";GET-DEVICE-TOKEN")

def test_rpc_too_many_params(session, netconf_methods, mqtt_client):
	req_id = 0
	netconf_mqtt_bridge.bridge.current_req_id = req_id

	_, _, res = session.send_rpc("<funcGetDeviceToken><uuidInput>%s</uuidInput><someTag>someText</someTag></funcGetDeviceToken>" % uuids["mup"])

	assert res == (
"""<?xml version="1.0" encoding="UTF-8"?><rpc-reply xmlns="urn:ietf:params:xml:ns:netconf:base:1.0" message-id="%d">
  <data>
    <retval>TOO MANY OR TOO FEW PARAMS</retval>
  </data>
</rpc-reply>
""" % req_id)
	mqtt_client.publish.assert_not_called()

def test_rpc_too_few_params(session, netconf_methods, mqtt_client):
	req_id = 0
	netconf_mqtt_bridge.bridge.current_req_id = req_id

	_, _, res = session.send_rpc("<funcGetDeviceToken></funcGetDeviceToken>")

	assert res == (
"""<?xml version="1.0" encoding="UTF-8"?><rpc-reply xmlns="urn:ietf:params:xml:ns:netconf:base:1.0" message-id="%d">
  <data>
    <retval>TOO MANY OR TOO FEW PARAMS</retval>
  </data>
</rpc-reply>
""" % req_id)
	mqtt_client.publish.assert_not_called()