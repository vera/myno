import pytest

from netconf import client
from importlib import reload
from lxml.etree import tostring
import cbor2

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

bridge = None


"""
Fixtures
"""

@pytest.fixture(autouse=True)
def setup(mqtt_client):
	global bridge
	bridge = netconf_mqtt_bridge.NetconfMqttBridge(mqtt_client)
	with open('tests/assets/ontology_mup.jsonld') as ontology:
		bridge.device_descriptions[uuids["mup"]] = ontology.read()
		bridge.create_device(uuids["mup"])
	with open('tests/assets/ontology_led+sensor.jsonld') as ontology:
		bridge.device_descriptions[uuids["sensor+led"]] = ontology.read()
		bridge.create_device(uuids["sensor+led"])
	with open('tests/assets/ontology_precision_agriculture.jsonld') as ontology:
		bridge.device_descriptions[uuids["pa"]] = ontology.read()
		bridge.create_device(uuids["pa"])

@pytest.fixture()
def session():
	yield client.NetconfSSHSession("127.0.0.1", username=config.NC_USERNAME, password=config.NC_PASSWORD, port=config.NC_PORT)

@pytest.fixture
def netconf_methods():
	netconf_methods = ncm.NetconfMethods(bridge)
	yield netconf_methods

@pytest.fixture()
def mqtt_client(mocker):
	class MqttClient:
		def publish(self):
			pass

	config.SECS_TO_WAIT_FOR_RESPONSE = 0.2

	mocker.patch.object(MqttClient, 'publish')
	mock_client = MqttClient()
	yield mock_client


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
	bridge.current_req_id = req_id
	bridge.responses[str(req_id)] = response

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
		"image_hash_binary": "abcdef",
		"image_size": 500,
		"version": 2,
		"old_version": 1,
		"inner_key_info_binary": "fecafecaf9ffffff",
		"inner_signature_binary": "abcdef",
		"nonce": 123456,
		"outer_key_info_binary": "efbeaddef9ffffff",
		"outer_signature_binary": "abcdef"
	}
	req_id = 0
	response = "OK!"
	bridge.current_req_id = req_id
	bridge.responses[str(req_id)] = response

	_, _, res = session.send_rpc(
"""<funcPubUpdateManifest>
  <uuidInput>%s</uuidInput>
  <input_01_AppId>%s</input_01_AppId>
  <input_02_LinkOffset>%d</input_02_LinkOffset>
  <input_03_Hash>%s</input_03_Hash>
  <input_04_Size>%d</input_04_Size>
  <input_05_Version>%d</input_05_Version>
  <input_06_OldVersion>%d</input_06_OldVersion>
  <input_07_InnerKeyInfo>%s</input_07_InnerKeyInfo>
  <input_08_InnerSignature>%s</input_08_InnerSignature>
  <input_09_DeviceNonce>%d</input_09_DeviceNonce>
  <input_10_OuterKeyInfo>%s</input_10_OuterKeyInfo>
  <input_11_OuterSignature>%s</input_11_OuterSignature>
</funcPubUpdateManifest>
""" % (uuids["mup"], params["app_id"], params["offset"], params["image_hash_binary"], params["image_size"], params["version"], params["old_version"], params["inner_key_info_binary"], params["inner_signature_binary"], params["nonce"], params["outer_key_info_binary"], params["outer_signature_binary"]))

	assert res == (
"""<?xml version="1.0" encoding="UTF-8"?><rpc-reply xmlns="urn:ietf:params:xml:ns:netconf:base:1.0" message-id="%d">
  <data>
    <retval>%s</retval>
  </data>
</rpc-reply>
""" % (req_id, response))
	mqtt_client.publish.assert_called_once()
	expected_message = str(req_id) + ";PUB-UPDATE-MANIFEST;"
	for key in params:
		if "_binary" in key:
			# Special treatment of binary params
			try:
				expected_message = bytes(expected_message, "utf-8") + bytes.fromhex(params[key]) + b","
			except TypeError:
				# If message already includes binary params
				expected_message += bytes.fromhex(params[key]) + b","
		else:
			try:
				expected_message += str(params[key]) + ","
			except TypeError:
				# If message already includes binary params
				expected_message += bytes(str(params[key]) + ",", "utf-8")
	expected_message = expected_message[:-1]
	assert mqtt_client.publish.call_args.args == ("yang/update/manifest", expected_message)

def test_rpc_multiple_params_cbor(session, netconf_methods, mqtt_client):
	bridge._toggle_cbor()
	params = {
		"app_id": "TestApp",
		"offset": 8000,
		"image_hash_binary": "abcdef",
		"image_size": 500,
		"version": 2,
		"old_version": 1,
		"inner_key_info_binary": "fecafecaf9ffffff",
		"inner_signature_binary": "abcdef",
		"nonce": 123456,
		"outer_key_info_binary": "efbeaddef9ffffff",
		"outer_signature_binary": "abcdef"
	}
	params_list = ["app_id", "offset", "image_hash_binary", "image_size", "version", "old_version", "inner_key_info_binary", "inner_signature_binary", "nonce", "outer_key_info_binary", "outer_signature_binary"]
	req_id = 0
	response = "OK!"
	bridge.current_req_id = req_id
	bridge.responses[str(req_id)] = response

	rpc_params = (uuids["mup"],) + tuple(map(lambda key: params[key], params_list))
	_, _, res = session.send_rpc(
"""<funcPubUpdateManifest>
  <uuidInput>%s</uuidInput>
  <input_01_AppId>%s</input_01_AppId>
  <input_02_LinkOffset>%d</input_02_LinkOffset>
  <input_03_Hash>%s</input_03_Hash>
  <input_04_Size>%d</input_04_Size>
  <input_05_Version>%d</input_05_Version>
  <input_06_OldVersion>%d</input_06_OldVersion>
  <input_07_InnerKeyInfo>%s</input_07_InnerKeyInfo>
  <input_08_InnerSignature>%s</input_08_InnerSignature>
  <input_09_DeviceNonce>%d</input_09_DeviceNonce>
  <input_10_OuterKeyInfo>%s</input_10_OuterKeyInfo>
  <input_11_OuterSignature>%s</input_11_OuterSignature>
</funcPubUpdateManifest>
""" % rpc_params)
	bridge._toggle_cbor()

	assert res == (
"""<?xml version="1.0" encoding="UTF-8"?><rpc-reply xmlns="urn:ietf:params:xml:ns:netconf:base:1.0" message-id="%d">
  <data>
    <retval>%s</retval>
  </data>
</rpc-reply>
""" % (req_id, response))
	mqtt_client.publish.assert_called_once()
	expected_message = str(req_id) + ";PUB-UPDATE-MANIFEST;"
	expected_message  = bytes(expected_message, "utf-8") + cbor2.dumps(list(map(lambda key: bytes.fromhex(params[key]) if "_binary" in key else params[key], params_list)))
	assert mqtt_client.publish.call_args.args == ("yang/update/manifest", expected_message)

def test_rpc_no_response(session, netconf_methods, mqtt_client):
	req_id = 0
	bridge.current_req_id = req_id
	if str(req_id) in bridge.responses:
		del bridge.responses[str(req_id)]

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
	bridge.current_req_id = req_id

	_, _, res = session.send_rpc("<funcGetDeviceToken><uuidInput>%s</uuidInput><someTag>someText</someTag></funcGetDeviceToken>" % uuids["mup"])

	assert res == (
"""<?xml version="1.0" encoding="UTF-8"?><rpc-reply xmlns="urn:ietf:params:xml:ns:netconf:base:1.0" message-id="%d">
  <data>
    <retval>UNKNOWN PARAMS: someTag</retval>
  </data>
</rpc-reply>
""" % req_id)
	mqtt_client.publish.assert_not_called()

def test_rpc_too_few_params(session, netconf_methods, mqtt_client):
	req_id = 0
	bridge.current_req_id = req_id

	_, _, res = session.send_rpc("<funcGetDeviceToken></funcGetDeviceToken>")

	assert res == (
"""<?xml version="1.0" encoding="UTF-8"?><rpc-reply xmlns="urn:ietf:params:xml:ns:netconf:base:1.0" message-id="%d">
  <data>
    <retval>MISSING PARAMS: uuidInput</retval>
  </data>
</rpc-reply>
""" % req_id)
	mqtt_client.publish.assert_not_called()

def test_rpc_too_many_and_too_few_params(session, netconf_methods, mqtt_client):
	req_id = 0
	bridge.current_req_id = req_id

	_, _, res = session.send_rpc("<funcGetDeviceToken><someTag>someText</someTag></funcGetDeviceToken>")

	assert res == (
"""<?xml version="1.0" encoding="UTF-8"?><rpc-reply xmlns="urn:ietf:params:xml:ns:netconf:base:1.0" message-id="%d">
  <data>
    <retval>UNKNOWN PARAMS: someTag, MISSING PARAMS: uuidInput</retval>
  </data>
</rpc-reply>
""" % req_id)
	mqtt_client.publish.assert_not_called()