from flask_mqtt import Mqtt

import datetime
import logging
import threading

import config
import update_server

mqtt = Mqtt()

sensor_dict = {}

def async_init(app):
	threading.Thread(target=init, args=[app]).start()

def init(app):
	app.config['MQTT_BROKER_URL'] = config.MQTT_BROKER_URL
	app.config['MQTT_BROKER_PORT'] = config.MQTT_BROKER_PORT
	# for auth, config is ['MQTT_USERNAME'] and ['MQTT_PASSWORD']
	app.config['MQTT_REFRESH_TIME'] = config.MQTT_REFRESH_TIME

	mqtt.init_app(app)

	@mqtt.on_message()
	def handle_mqtt_message(client, userdata, message):
		global sensor_dict

		logging.info("Received message '" + str(message.payload.decode()) + "' on topic '" + message.topic + "'")

		if message.topic in update_server.unacked_slices:
			payload = str(message.payload.decode()).split(',')
			if int(payload[0]) not in config.unacked_slices[message.topic]:
				logging.warning("Received response for unknown slice " + payload[0])
				return
			if payload[1] == UPDATE_SLICE_ACK:
				config.unacked_slices[message.topic].remove(int(payload[0]))
				logging.info("Received ACK for slice " + payload[0])
		else:
			sensor_dict[message.topic] = (datetime.datetime.now().strftime('%d.%m.%Y %H:%M:%S'), message.payload.decode())

def subscribe(*args):
	"""Passes through args to subscribe method."""
	mqtt.subscribe(*args)

def unsubscribe(*args):
	"""Passes through args to unsubscribe method."""
	mqtt.unsubscribe(*args)

def publish(*args):
	"""Passes through args to publish method."""
	mqtt.publish(*args)