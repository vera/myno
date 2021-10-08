import pytest

from lxml.etree import tostring

from myno_web_app import create_app
from tests.device_dicts import *

mock_nc_manager = None
device_id = None
device_cat = None

"""
Fixtures
"""

@pytest.fixture
def client(mocker):
  global mock_nc_manager

  mocker.patch('myno_web_app.netconf_client.async_init')
  mocker.patch('myno_web_app.netconf_client.get_device_dict', return_value={})

  mocker_config = { 'dispatch.return_value': None }
  mock_nc_manager  = mocker.patch('myno_web_app.netconf_client.netconf_manager', **mocker_config)

  app = create_app()
  yield app.test_client()

@pytest.fixture
def with_precision_agriculture_setup(mocker):
  global device_id, device_cat

  mocker.patch('myno_web_app.netconf_client.get_device_dict', return_value=device_dict_pa)

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

def test_toggle_rpc_for_pa_board(client, with_precision_agriculture_setup):
  """
  Check that GET request for a toggle function results in the correct XML RPC being dispatched
  """
  toggle_func = 'funcLed_1On'
  res = client.get('/function_call/' + device_id + '/' + toggle_func)

  mock_nc_manager.dispatch.assert_called_once()
  xml_rpc = tostring(mock_nc_manager.dispatch.call_args.args[0]).decode("utf-8") 
  assert xml_rpc == '<' + toggle_func + '><uuidInput>' + device_id + '</uuidInput></' + toggle_func + '>'

"""
POST /function_call/<device_id>/<function_name>/<param_type>/<param_name>/<param_value>
"""

def test_auto_func_for_pa_board(client, with_precision_agriculture_setup):
  """
  Check that GET request for an automation function results in the correct XML RPC being dispatched
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