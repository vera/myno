from . import config
from . import update_server

from flask_mqtt import Mqtt

import datetime
import logging
import threading

if config.UPLOAD_TO_AWS:
	import requests
	import csv

mqtt = Mqtt()

_sensor_dict = {}

def async_init(app):
	mqtt_thread = threading.Thread(target=init, args=[app])
	mqtt_thread.daemon = True
	mqtt_thread.start()

def init(app):
	app.config['MQTT_BROKER_URL'] = config.MQTT_BROKER_URL
	app.config['MQTT_BROKER_PORT'] = config.MQTT_BROKER_PORT
	# for auth, config is ['MQTT_USERNAME'] and ['MQTT_PASSWORD']
	app.config['MQTT_REFRESH_TIME'] = config.MQTT_REFRESH_TIME

	mqtt.init_app(app)

	@mqtt.on_message()
	def handle_mqtt_message(client, userdata, message):
		global _sensor_dict

		logging.info("Received message '" + str(message.payload.decode()) + "' on topic '" + message.topic + "'")

		payload = str(message.payload.decode()).split(',')

		if message.topic in update_server.unacked_slices:
			if int(payload[0]) not in update_server.unacked_slices[message.topic]:
				logging.warning("Received response for unknown slice " + payload[0])
				return
			if payload[1] == config.UPDATE_SLICE_ACK:
				update_server.unacked_slices[message.topic].remove(int(payload[0]))
				logging.info("Received ACK for slice " + payload[0])
		else:
			_sensor_dict[message.topic] = (datetime.datetime.now(), message.payload.decode())

		if config.UPLOAD_TO_AWS:
			_upload_to_aws(message.topic, payload)

def get_sensor_dict():
	# Prettify timestamps
	temp = { k : (lambda x: (x[0].strftime(config.TIMESTAMP_FORMAT), x[1]))(v) for k, v in _sensor_dict.items() }
	return temp

def subscribe(*args):
	"""Passes through args to subscribe method."""
	mqtt.subscribe(*args)

def unsubscribe(*args):
	"""Passes through args to unsubscribe method."""
	mqtt.unsubscribe(*args)

def publish(*args):
	"""Passes through args to publish method."""
	mqtt.publish(*args)

def _upload_to_aws(topic, payload):
	if topic.startswith("event") and len(payload[0]) > 0:
		requests.post(config.AWS_API_URL, json = { "event": payload[0] })

	if topic.startswith("sensor"):
		with open(config.SENSOR_VALUES_CSV_PATH, "a") as measures:
			writer = csv.writer(measures, delimiter=",", quotechar='"', quoting=csv.QUOTE_MINIMAL)
			writer.writerow([datetime.datetime.now(), topic, payload[0]])