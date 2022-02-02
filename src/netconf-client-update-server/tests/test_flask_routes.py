import pytest
import time

from lxml.etree import tostring

from myno_web_app import create_app, notifications
from tests.device_dicts import *

mock_nc_manager = None
device_id = None
device_cat = None

"""
Fixtures
"""

@pytest.fixture(autouse=True)
def reset_notifications():
  notifications.reset()

@pytest.fixture
def client(mocker):
  global mock_nc_manager

  mocker.patch('myno_web_app.netconf_client.async_init')
  mocker.patch('myno_web_app.netconf_client.get_device_dict', return_value={})

  mocker_config = { 'dispatch.return_value': '<?xml version="1.0" encoding="UTF-8"?><nc:rpc-reply xmlns:nc="urn:ietf:params:xml:ns:netconf:base:1.0" message-id="urn:uuid:2050cd5b-f14d-bd3d-f791-cd95ed99"><data><retval>OK</retval></data></nc:rpc-reply>' }
  mock_nc_manager = mocker.patch('myno_web_app.netconf_client._netconf_manager', **mocker_config)

  app = create_app()
  yield app.test_client()

@pytest.fixture
def with_precision_agriculture_setup(mocker):
  global device_id, device_cat

  mocker.patch('myno_web_app.netconf_client.get_device_dict', return_value=device_dict_pa)

  device_id = list(device_dict_pa.items())[0][0]
  device_cat = list(device_dict_pa.items())[0][1][0]

@pytest.fixture
def with_mup_setup(mocker):
  global device_id, device_cat

  mocker.patch('myno_web_app.netconf_client.get_device_dict', return_value=device_dict_mup)

  device_id = list(device_dict_pa.items())[0][0]
  device_cat = list(device_dict_pa.items())[0][1][0]

"""
GET /
"""

def test_root_is_200(client):
  """
  Check that GET / is 200
  """
  res = client.get('/')
  assert res.status_code == 200

def test_root_is_empty(client):
  """
  Check that GET / displays 'no devices available' when no devices are registered
  """
  res = client.get('/')
  assert b'No devices available, press "Get Devices" to refresh.' in res.get_data()

def test_root_displays_pa_board(client, with_precision_agriculture_setup):
  """
  Check that GET / displays device that is registered incl. RPCs and sensors
  """
  res = client.get('/')
  # Board ID is displayed
  assert device_id in res.get_data(as_text=True)
  assert device_cat in res.get_data(as_text=True)
  # RPCs are displayed
  for rpc in list(device_dict_pa.items())[0][1][1]['rpcs']:
    assert rpc in res.get_data(as_text=True)
  # Sensor readings are displayed
  for sensor in list(device_dict_pa.items())[0][1][1]['sensors']:
    assert sensor in res.get_data(as_text=True)

"""
GET /function_call/<device_id>/<function_name>
"""

def test_toggle_rpc_for_pa_board_dispatch(client, with_precision_agriculture_setup):
  """
  Check that GET request for a toggle function results in the correct XML RPC being dispatched
  """
  toggle_func = 'funcLed_1On'
  res = client.get('/function_call/' + device_id + '/' + toggle_func)

  mock_nc_manager.dispatch.assert_called_once()
  xml_rpc = tostring(mock_nc_manager.dispatch.call_args.args[0]).decode("utf-8")
  assert xml_rpc == '<' + toggle_func + '><uuidInput>' + device_id + '</uuidInput></' + toggle_func + '>'

def test_toggle_rpc_for_pa_board_handle_reply(client, with_precision_agriculture_setup):
  """
  Check that GET request for a toggle function results in the correct XML RPC being dispatched
  """
  toggle_func = 'funcLed_1Off'
  res = client.get('/function_call/' + device_id + '/' + toggle_func)

  # Wait for the netconf_client thread to finish handling RPC reply
  # TODO This should be done more cleanly...
  time.sleep(1)

  info_notifications = notifications.get_info()
  assert len(info_notifications) == 2
  assert len(list(filter(lambda x: x[1] == 'Sent RPC to ' + device_id + ': ' + toggle_func, info_notifications))) == 1
  assert len(list(filter(lambda x: x[1] == 'State has changed successfully (OK)', info_notifications))) == 1
  assert len(notifications.get_errors()) == 0

"""
POST /function_call/<device_id>/<function_name>/<param_type>/<param_name>/<param_value>
"""

def test_auto_func_for_pa_board(client, with_precision_agriculture_setup):
  """
  Check that POST request for an automation function results in the correct XML RPC being dispatched
  """
  auto_func = 'funcAutoMoisture'
  param_type = 'string'
  param_name = 'inputAutoMoisture'
  param_value = 'inputAutoMoisture;'
  form_data = { 'input_1_ConfEvName': 'Default',
                'input_2_ConfOperator': '<',
                'input_3_ConfMoisture': '50',
                'input_4_ConfInterval': '600',
                'input_5_ConfDuration': '300',
                'input_6_ConfCrud': 'CREATE',
                'command': 'funcPump_1On,' }

  res = client.post('/function_call/' + device_id + '/' + auto_func + '/' + param_type + '/' + param_name + '/' + param_value,
                    data=form_data)

  mock_nc_manager.dispatch.assert_called_once()
  xml_rpc = tostring(mock_nc_manager.dispatch.call_args.args[0]).decode("utf-8")
  expected_xml_rpc = '<' + auto_func + '>' + \
                     '<uuidInput>' + device_id + '</uuidInput>' + \
                     '<' + param_name + '>' + ','.join(form_data.values())[:-1].replace("<", "&lt;") + \
                     '</' + param_name + '>' + \
                     '</' + auto_func + '>'
  assert xml_rpc == expected_xml_rpc

"""
POST /function_call/<device_id>/<function_name>
"""

def test_manifest_post_for_mup_board(client, with_mup_setup):
  """
  Check that POST request for manifest results in the correct XML RPC being dispatched
  """
  func_name = 'funcPubUpdateManifest'
  form_data = { 'input_1_AppId': 'APP',
                'input_2_LinkOffset': '0',
                'input_3_Hash': 'cc6cf68547fe9a90070da75bdc953f34af1c708d1baf8c2912070d2f05845f73',
                'input_4_Size': '555',
                'input_5_Version': '2',
                'input_6_OldVersion': '1',
                'input_7_InnerKeyInfo': 'fecafecaf9ffffff',
                'input_8_InnerSignature': '99fe5c11de573700bbf10d56052d77e3faafc0239dd5381ae1723945970a31436094181e03b38e84ca5338f58354977f06d144ac65fc1e58e8dd6a73b9130b99',
                'input_9_DeviceNonce': '123456',
                'input_10_OuterKeyInfo': 'efbeaddef9ffffff',
                'input_11_OuterSignature': 'a1db0d8f0b42c6ac924fae045a6e4b22f0e081dd19d890b808c111377751f8525364d8170e1182523e008c5f75828b16518253f7c4a908e2e28cadd2f45af9e7' }

  client.post('/function_call/' + device_id + '/' + func_name, data=form_data)

  mock_nc_manager.dispatch.assert_called_once()
  xml_rpc = tostring(mock_nc_manager.dispatch.call_args.args[0]).decode("utf-8")
  expected_xml_rpc = '<' + func_name + '>' + '<uuidInput>' + device_id + '</uuidInput>'
  for input in form_data:
    expected_xml_rpc += '<' + input + '>' + form_data[input] + '</' + input + '>'
  expected_xml_rpc += '</' + func_name + '>'
  assert xml_rpc == expected_xml_rpc

def test_key_rollover_post_for_mup_board(client, with_mup_setup):
  """
  Check that POST request for key rollover results in the correct XML RPC being dispatched
  """
  func_name = 'funcPubUpdateKeyU'
  form_data = { 'input_1_AppId': 'APP',
                'input_2_Version': '2',
                'input_03_NewKeyInfo': 'efbeedfef9ffffff0200000001000000',
                'input_04_NewKey': 'c3b993f64651add0f13cdd98824f04aca351b8349a25194f8b2e1b59ad9a163d2b8732c822c533c7c2ca19d6192062ef9598cbd6759bd58f77ac2fae8970fc6f',
                'input_05_DeviceNonce': '1234567890',
                'input_06_KeyInfo': 'efbeaddef9ffffff0200000001000000',
                'input_07_Signature': '50c80cc699c782977b067f4825533fd3d6a7b6e00fa1728147945ac64b050b1141eb5f2e153926599fd78e3514b7b8b34662804ed096d14ad5e3b84e9af1319e' }

  client.post('/function_call/' + device_id + '/' + func_name, data=form_data)

  mock_nc_manager.dispatch.assert_called_once()
  xml_rpc = tostring(mock_nc_manager.dispatch.call_args.args[0]).decode("utf-8")
  expected_xml_rpc = '<' + func_name + '>' + '<uuidInput>' + device_id + '</uuidInput>'
  for input in form_data:
    expected_xml_rpc += '<' + input + '>' + form_data[input] + '</' + input + '>'
  expected_xml_rpc += '</' + func_name + '>'
  assert xml_rpc == expected_xml_rpc