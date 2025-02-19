from lxml import etree # Needed because ncclient sends lxml, not xml object
from ncclient import manager, xml_
from socket import error as socket_error
from .yang2yin_direct import direct_yang2yin_call

import csv
import logging
import threading
import xml.etree.ElementTree as ET

from . import config
from . import notifications
from . import namespaceparser
from . import mqtt_client

_device_dict = {}
_netconf_manager = None
nonce_dict={}
global_rpc_lock = threading.Lock()
topics=[]

def async_init(reload_event):
	netconf_client_thread = threading.Thread(target=init, args=(reload_event,))
	netconf_client_thread.daemon = True
	netconf_client_thread.start()

def init(reload_event):
	global _netconf_manager

	try:
		# Connect to NETCONF server via manager
		_netconf_manager = manager.connect_ssh(config.NETCONF_IP,
																					port=config.NETCONF_PORT,
																					username=config.NETCONF_USERNAME,
																					password=config.NETCONF_PASSWORD,
																					allow_agent=False, hostkey_verify=False,
																					look_for_keys=False,
																					manager_params={"timeout": 300})
		_get_devices()
		while True:
			reload_event.wait()
			reload_event.clear()
			_get_devices()
			logging.info("Reloaded devices")

	except socket_error as serr:
		notifications.add_error('netconf_client.init(): Socket error. '
														+ 'Maybe unable to open socket.')
		logging.exception("netconf_client.init(): socket_error couldnt open")
	except Exception as e:
		notifications.add_error('Error in netconf_client.init().')
		logging.exception("netconf_client.init(): Error. Something else not caught by socket_error.")

def get_device_dict():
	global _device_dict
	return _device_dict

def _get_devices():
	global _device_dict

	_device_dict = get_devices()

	# Subscribe to sensor topics
	for device_id, device_data in _device_dict.items():
		if not _device_dict[device_id][1] or not _device_dict[device_id][1]['sensors']:
			continue
		for topic, _ in _device_dict[device_id][1]['sensors'].values():
			mqtt_client.subscribe(topic)

def get_devices():
	"""
	Sends <get> and <get-schema> RPCs and builds device dictionary based on
	the responses.
	
	Example device dictionary:
	{ device_id: (device_category, rpcs_and_sensors_dict) }

	where rpcs_and_sensors_dict is structured like

	{ rpcs: { rpc_name = (rpc_description, param_name, param_type_name, parameters) },
		sensors: { func_name: mqtt_topic }
	}
	"""

	res = {}

	try:
		# Get device dictionary
		devices = get()
		# Get RPC + sensor dictionary
		scheme_dict = get_schema()

		for key, value in devices.items():
			res[key] = (value[0], scheme_dict[value[1]])
	except NameError as e:
		notifications.add_error('Exception in get_devices().')
		logging.exception("netconf_client.get_devices(): Error. Probably manager not connected.")
	except Exception as e:
		notifications.add_error('Exception in get_devices().')
		logging.exception("netconf_client.get_devices(): Error.")

	return res

def get():
	"""Sends <get> RPC to NETCONF server and parses XML response into device dictionary.
	Device dictionary entries are structured like: { device_id : (device_category, device_namespace) }

	Example XML response:
	<?xml version="1.0" encoding="UTF-8"?><nc:rpc-reply xmlns:nc="urn:ietf:params:xml:ns:netconf:base:1.0" message-id="urn:uuid:d7dbaf29-8710-440a-b9ce-37bca30a3565">
		<data>
			<device xmlns="https://www.cs.uni-potsdam.de/bs/research/myno/SETUP-1">
				<device-id>
					<uuid>MYNO-UPDATE-8A12-4F4F-8F69-6B8F3C2E78DD</uuid>
				</device-id>
				<device-category>SETUP-1</device-category>
			</device>
		</data>
	</nc:rpc-reply>

	Resulting device dictionary:
	{'MYNO-UPDATE-8A12-4F4F-8F69-6B8F3C2E78DD': ('SETUP-1', 'https://www.cs.uni-potsdam.de/bs/research/myno/SETUP-1')}
	"""
	devices = {}

	# Make GET RPC
	get_string = str(_netconf_manager.get())

	# Parse XML into tree representation
	tree = ET.fromstring(get_string)

	# Iterate over root's children
	for data in tree:
		# Get <data> tag
		if("data" in data.tag):
			schema_data = data

	# Iterate over <data> tag's children
	for device in schema_data:
		# Get <device> tag
		if("device" in device.tag):
			# Extract XML namespace
			device_namespace = (device.tag).partition('{')[-1].rpartition('}')[0]

			# Find first subelement matching "{namespace}device-id"
			device_id_node = (device.find(device.tag + '-id'))
			if device_id_node is not None:
				# Get text of first subelement matching "{namespace}uuid"
				device_uuid = device_id_node.find('{'+device_namespace+'}uuid').text
				# Get text of first subelement of <device> matching "{namespace}device-category"
				device_category = (device.find(device.tag+'-category')).text
				# Add device to dictionary
				devices[device_uuid] = (device_category, device_namespace)
			else:
				logging.warning('Device found but it has no device-id')

	return devices

def get_schema():
	"""Sends <get-schema> RPC to NETCONF server and parses XML response into dictionary.
	Dictionary entries are structured like:
	{ rpcs: { rpc_name = (rpc_description, param_name, param_type_name, parameters) },
		sensors: { func_name: mqtt_topic }
	}

	Example XML response:
	<?xml version="1.0" encoding="UTF-8"?><nc:rpc-reply xmlns:nc="urn:ietf:params:xml:ns:netconf:base:1.0" message-id="urn:uuid:1bbe5905-801d-4d4b-8201-9685478cc17e">
	<data xmlns="urn:ietf:params:xml:ns:yang:ietf-netconf-monitoring"><![CDATA[module mqtt-SETUP-1 {
	namespace "https://www.cs.uni-potsdam.de/bs/research/myno/SETUP-1";
	prefix SETUP-1;

	rpc funcPubUpdateManifest {
		description
			"Publish Update Manifest to Device";
		input {
			leaf input_1_AppId {
				description
					"AnwendungsID der Zielplattform";
				type string;
			}
			...
		}
		output {
			leaf retval {
				type enumeration {
					enum 10_OK {
						description
							"10 successful";
					}
					...
				}
			}
		}
	}]]></data>
	</nc:rpc-reply>
	"""
	modules = {}

	# Make get-schema RPC (with empty identifier)
	get_schema_string = (str(_netconf_manager.get_schema('')))

	# Parse XML into tree representation
	root = ET.fromstring(get_schema_string)
	
	# Iterate over root's children
	for child in root:
		# Get <data> tag
		if("data" in child.tag):
			schema_data = child.text

	if not schema_data:
		logging.warning("No <data> tag found in <get-schema> RPC response")
		return

	# Split multiple modules and convert module(s) to YIN
	# (see https://datatracker.ietf.org/doc/html/rfc7950#section-13)
	# Resulting YIN contains <rpc> and <container> tags
	multimodule = schema_data.split("module")
	if len(multimodule) > 2:
		logging.info("Multiple modules found in <get-schema> RPC response")
		xml_root = []
		for s in multimodule[1:]:
			xml_schema = direct_yang2yin_call("module" + s)
			xml_root.append(ET.fromstring(xml_schema))
	else:
		xml_schema = direct_yang2yin_call(schema_data)
		xml_root = [ET.fromstring(xml_schema)]

	logging.info("Parsed modules to YIN")
	
	# For each module
	for root in xml_root:
		# Read all subelements
		for module in root.findall('.'):
			rpcs = {}
			sensors = {}
			rpc_names = []
			rpc_descriptions = []
			
			# Get all <rpc> tags
			for rpc in namespaceparser.findall(module, './', 'rpc'):
				# Extract name and description
				rpc_name = rpc.attrib['name']
				rpc_description = namespaceparser.find(namespaceparser.find(rpc, './', 'description'), './', 'text').text

				# Extract input parameters
				param_name = []
				param_type_name = []
				parameters = []
				# Parse child <leaf> tags of <input>
				for leaf in namespaceparser.find(rpc, './', 'input'):
					# uuidInput is standard, we are only interested in the other input parameters
					if(leaf.attrib['name'] != "uuidInput"):
						for param_type in namespaceparser.findall(leaf, './', 'type'):
							param_type_name = param_type.attrib['name']
							param_name = leaf.attrib['name']

						if len(namespaceparser.find(leaf, './', 'type')) != 0:
							# type is not None
							if namespaceparser.find(leaf, './', 'type').attrib['name'] == 'union':
								# e.g. RGB input for LED
								for parameter in namespaceparser.find(leaf, './', 'type'):
									value_range = namespaceparser.find(parameter, './', 'range').attrib['value'].split("..")
									parameters.append([parameter.attrib['name'], value_range])
							elif namespaceparser.find(leaf, './', 'type').attrib['name'] == 'enumeration':
								# e.g. event/automation CRUD operators
								enums = []
								for enum in namespaceparser.findall(param_type, './', 'enum'):
									option = enum.attrib['name']
									enums.append(option)
								parameters.append([leaf.attrib['name'], enums])
							elif namespaceparser.find(leaf, './', 'type').attrib['name'] == 'int':
								for element in namespaceparser.findall(leaf, './', 'type'):
									value_range = namespaceparser.find(element, './', 'range').attrib['value'].split("..")
									parameters.append([leaf.attrib['name'], namespaceparser.find(leaf, './', 'type').attrib['name'], value_range])
						else:
							# type is None, e.g. event configuration name, duration, interval
							parameters.append([leaf.attrib['name'], namespaceparser.find(leaf, './', 'type').attrib['name']])

						# # Extract <leaf name="..."> attribute
						# param_name.append(leaf.attrib['name'])
						# # Extract all <type name="..."/> tags into param_type_name
						# list_param_type = namespaceparser.findall(leaf, './', 'type')
						# for param_type in list_param_type:
						# 	param_type_name.append(param_type.attrib['name'])
						# # Extract first <type name="..."/> tag into parameters
						# for parameter in namespaceparser.find(leaf, './', 'type'):
						# 	try:
						# 		parameters.append(parameter.attrib['name'])
						# 	except KeyError:
						# 		pass

				# Extract topic from <default> tag
				topic = None
				if namespaceparser.find(rpc, './', 'default') != None:
					topic = namespaceparser.find(rpc,'./','default').attrib['value']

				# Add RPC
				rpcs[rpc_name] = (rpc_description, param_name, param_type_name, parameters, topic)

			# Get all <container> tags
			for container in namespaceparser.findall(module, './', 'container'):
				# Get all sub-<container> tags
				for telemetry in namespaceparser.findall(container, './', 'container'):
					# Get all sub-<leaf> tags
					for leaf in namespaceparser.findall(telemetry, './', 'leaf'):
						# Extract name and description
						leaf_name = leaf.attrib['name']
						leaf_description = (namespaceparser.find(namespaceparser.find(leaf, './', 'description'), './', 'text')).text

						# Get sub-<container> tag
						datapoint = (namespaceparser.find(leaf, './', 'container'))
						# Get all sub-<leaf> tags (contains function name + MQTT topic)
						for leaf in namespaceparser.findall(datapoint, './', 'leaf'):
							# Extract name and MQTT topic (stored in <default>)
							leaf_name0 = leaf.attrib['name']
							default = namespaceparser.find(leaf, './', 'default')
							if namespaceparser.findall(leaf, './', 'units') == []:
								unit = ''
							else:
								unit = namespaceparser.findall(leaf, './', 'units')[0].attrib['name'].split(";")[1]
								if (unit == "%%"):
									unit = "%"
							sensors[leaf_name] = [default.attrib['value'], unit]
			
			# Get all <namespace> tags
			for namespace in namespaceparser.findall(module, './', 'namespace'):
				components = { "rpcs": rpcs, "sensors": sensors }
				namespace_uri = namespace.attrib['uri']
				modules[namespace_uri] = components

	return modules

def build_xml_rpc(thing, function, param_type, param_name, value, form_items):
	"""Builds XML RPCs.

	Example:
	<?xml version="1.0" encoding="UTF-8"?>
	<nc:rpc xmlns:nc="urn:ietf:params:xml:ns:netconf:base:1.0" message-id="urn:uuid:a25f84f9-db7f-40ce-a2d8-71db7128a1ef">
		<funcRelayOn>
			<uuidInput>MYNO-UPDATE-8A12-4F4F-8F69-6B8F3C2E78DD</uuidInput>
		</funcRelayOn>
	</nc:rpc>
	"""
	xml_rpc = xml_.to_ele('<'+function+'/>')
	child = etree.SubElement(xml_rpc, "uuidInput")
	child.text = thing

	if param_type == 'union' or param_type == 'enumeration':
		parameters = value.split(";")
		value = ""
		del parameters[-1]
		for parameter in parameters:
			value = value + request.form[parameter] + ","
		value = value[:-1]

	# Generate value for automation functions (recognized by function name)
	if "Auto" in function:
			parameters = value.split(";")
			value = ""
			del parameters[-1]
			for key, val in form_items:
					if (key == 'command' and val.split(",")[1] !='union'):
							# drop 'union' field
							value = value + val.split(",")[0] + ","
							break
					elif (key == 'command' and val.split(",")[1] == 'union'):
							# parameters are separated by comma (board behavior), however,
							# RGB inputs are also separated by comma, change to ':'
							value = value + val.split(",")[0] + ":"
					elif ("input" in key and "input_" not in key):
							value = value + val + ":"
					else:
							value = value + val + ","
			value = value[:-1]
	else:
		for key, value in form_items:
			inputParameters = etree.SubElement(xml_rpc, key)
			inputParameters.text = value

	if value and param_name:
		child = etree.SubElement(xml_rpc, param_name)
		child.text = value

	return xml_rpc

def send_rpcs(xml_rpcs):
	"""
	Sends multiple RPCs and extracts return value from final response.
	"""
	try:
		for xml_rpc in xml_rpcs:
			# Send RPC
			rpc_reply = _netconf_manager.dispatch(xml_rpc)
			logging.debug("Received RPC reply: " + str(rpc_reply))

			# All RPC replies except for the final RPC are discarded
			if xml_rpc == xml_rpcs[-1]:
				_handle_rpc_reply(str(rpc_reply), xml_rpc.tag)
	except Exception as e:
		notifications.add_error("RPC failed. NETCONF connection still active?")
		logging.exception("RPC failed")

def send_rpc(xml_rpc):
	"""
	Sends a single RPC and extracts return value from response.
	"""
	try:
		rpc_reply = _netconf_manager.dispatch(xml_rpc)
		logging.debug("Received RPC reply: " + str(rpc_reply))

		_handle_rpc_reply(str(rpc_reply), xml_rpc.tag)
	except Exception:
		notifications.add_error("RPC failed. NETCONF connection still active?")
		logging.exception("RPC failed")

def _rpc_retval_to_notification(retval):
	if retval == "OK":
		res = "State has changed successfully" + " (" + retval + ")"
	elif retval == "NOOP":
		res = "No change, device already had this state" + " (" + retval + ")"
	elif retval == "NACK":
		res = "No change, RPC contained unknown control" + " (" + retval + ")"
	else:
		res = retval
	return res

def _handle_rpc_reply(rpc_reply, xml_rpc_tag):
	"""
	Handles RPC reply.

	Example:
	<?xml version="1.0" encoding="UTF-8"?><nc:rpc-reply xmlns:nc="urn:ietf:params:xml:ns:netconf:base:1.0"
	message-id="urn:uuid:2050cd5b-f14d-bd3d-f791-cd95ed99">
		<data>
			<retval>NOOP</retval>
		</data>
	</nc:rpc-reply>
	"""
	global nonce_dict

	rpc_reply_xml_root = ET.fromstring(rpc_reply)
	for data_node in rpc_reply_xml_root.findall('./data'):
		for retval_node in data_node.findall('./retval'):
			# Display RPC reply
			notifications.add_info(_rpc_retval_to_notification(retval_node.text))
			# Extract nonce and old version from getDeviceToken reply
			if xml_rpc_tag == "funcGetDeviceToken":
				returnValues = retval_node.text.split(',')
				nonce_dict["input_8_DeviceNonce"] = returnValues[0]
				nonce_dict["input_6_OldVersion"] = returnValues[1]