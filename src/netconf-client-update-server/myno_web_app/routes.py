from flask import render_template, jsonify, url_for, request, redirect

from . import config
from . import mqtt_client
from . import netconf_client
from . import notifications

@app.route("/", methods=['GET'])
def root():
	"""
	Renders main page.
	"""
	global reload_event

	reload_event.set()

	return render_template(config.html_template_file,
												device_dict=netconf_client.device_dict)

@app.route('/ajax', methods=['GET'])
def ajax():
	"""
	Returns errors, notifications, nonces and sensor data as JSON.
	"""
	return jsonify(error=notifications.get_errors(),
								notification=notifications.get_info(),
								events=notifications.get_events(),
								sensors=mqtt_client.sensor_dict,
								nonce=netconf_client.nonce_dict)

@app.route('/function_call/<thing>/<function>', defaults={'param_type' : None,'param_name': None, 'value': None}, methods=['GET', 'POST'])
@app.route('/function_call/<thing>/<function>/<param_name>', methods=['GET', 'POST']) # ?
@app.route('/function_call/<thing>/<function>/<param_type>/<param_name>/<value>', methods=['GET', 'POST']) # ?
def function_click(thing, function, param_type, param_name, value):
	"""
	Handles RPC button clicks.
	"""
	if function == "funcPubUpdateImage":
		xml_rpcs = update_server.publish_image(thing, function,
																					netconf_client.device_dict[thing][1]['rpcs']['funcPubUpdateImage'][4] + '/' + thing,
																					request.files["inputUpdateImage"])
	elif function == "funcPubUpdateManifest":
		xml_rpcs = update_server.build_manifest_rpcs(thing, function,
																								sorted(request.form.items()))
	else:
		xml_rpcs = netconf_client.build_xml_rpc(thing, function, param_type, param_name, value)
	
	# Send RPC(s) in new thread so we don't block the main thread
	if isinstance(xml_rpcs, list):
		threading.Thread(target=netconf_client.send_rpcs, args=(xml_rpcs,)).start()
		content =	"Sent RPCs to "  + thing + ": " + function
	else:
		threading.Thread(target=netconf_client.send_rpc, args=(xml_rpcs,)).start()
		content =	"Sent RPC to "  + thing + ": " + function

	if value != None:
		content += " set to " + value
	notifications.add_info(content)
	return redirect(url_for('root'))

@app.template_test("isAutoOrConfFunction")
def is_auto_or_conf_function(function):
	return "Auto" in function[0] or "Conf" in function[0]