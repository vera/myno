# -*- coding: utf-8 -*-#
#
# Copyright (c)
# February 23 2017, Thomas Scheffler <scheffler@beuth-hochschule.de>
# August 2017, Alexander H. W. Lindemann <allindem@cs.uni-potsdam.de>
# August 2018-2019, Kristina Sahlmann <sahlmann@uni-potsdam.de>
# 2022, Vera Clemens <clemens@uni-potsdam.de>
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

import logging
import paho.mqtt.client as mqtt
import threading
import time

from netconf import server as netconf_server
from ncclient.xml_ import sub_ele
from lxml import etree

import config
from myno_device import MynoDevice
import netconf_methods as ncm


class NetconfMqttBridge:
	def __init__(self, mqtt_client=None):
		# Set up logging
		if config.LOG_DEBUG:
			logging.basicConfig(level=logging.DEBUG)
		else:
			logging.basicConfig(level=logging.INFO)

		self.logger = logging.getLogger(config.LOGGER_NAME)

		if not mqtt_client:
			# Set up MQTT connection
			self.mqtt_client = mqtt.Client()
			self.mqtt_client.on_connect = self.on_mqtt_connect
			self.mqtt_client.on_message = self.on_mqtt_message
			self.mqtt_client.connect(host=config.BROKER_ADDR, port=config.BROKER_PORT)
			self.mqtt_client.loop_start()
		else:
			self.mqtt_client = mqtt_client

		# Set up NETCONF connection
		sctrl = netconf_server.SSHUserPassController(username=config.NC_USERNAME, password=config.NC_PASSWORD)
		try:
			self.nc_server = netconf_server.NetconfSSHServer(server_ctl=sctrl,
															 server_methods=ncm.NetconfMethods(self),
															 port=config.NC_PORT,
															 host_key=config.NC_HOST_KEY,
															 debug=config.LOG_DEBUG)
		except Exception as error:
			self.logger.error("Error occured during NETCONF connection setup: %s" % error)

		# Initialize attributes
		self.devices = []
		self.yang_model = ""
		self.device_descriptions = {}
		self.device_descriptions_lock = threading.Lock()
		self.current_req_id = 0
		self.current_req_id_lock = threading.Lock()
		self.responses = {}
		self.responses_lock = threading.Lock()

		# Start main loop
		self.logger.info("Init complete")

	def run(self):
		self.logger.info("Starting up")
		running = True
		while running:
			try:
				time.sleep(100000)
			except KeyboardInterrupt:
				self.logger.info("got keyboard interrupt")
				running = False

		self.logger.info("Shutting down")

	def on_mqtt_connect(self, mqtt_client, userdata, flags, rc):
		self.logger.info("Connected to broker with result code %d" % rc)
		mqtt_client.subscribe(config.TOPIC_DEVICE_DESCRIPTIONS)
		mqtt_client.subscribe(config.TOPIC_RESPONSES)

	def on_mqtt_message(self, mqtt_client, userdata, msg):
		self.logger.info("Received message on topic %s: %s" % (msg.topic, msg.payload.decode()))

		# TODO Support update topic yang/config/update
		# TODO Support delete topic yang/config/delete

		if msg.topic in config.TOPIC_CREATE:
			uuid, dd = msg.payload.decode().split(";")

			self.device_descriptions_lock.acquire()

			if dd != "END":
				if uuid in self.device_descriptions:
					self.device_descriptions[uuid] += dd
				else:
					self.device_descriptions[uuid] = dd
			else:
				device = self.create_device(uuid)
				del self.device_descriptions[uuid]
				self.logger.info("Created device: %s" % (device.uuid))
				self.mqtt_client.publish(msg.topic + config.TOPIC_SUFFIX_RESPONSE + device.uuid, "OK")
				self.logger.info("Creating device: sent OK")

			self.device_descriptions_lock.release()
		else:
			topic_parts = msg.topic.split("/")
			if topic_parts[0] == "response":
				msg_id, response = msg.payload.decode().split(";")
				self.responses_lock.acquire()
				self.responses[msg_id] = response
				self.responses_lock.release()
				self.logger.info("Received response for request %s: %s" % (msg_id, response))
			elif topic_parts[2] == "ping" and topic_parts[3] == "response":
				uuid = topic_parts[4]
				self.logger.info("Received ping response from %s: %s" % (uuid, msg.payload.decode()))

	def create_device(self, uuid):
		device = MynoDevice(self.device_descriptions[uuid])
		self.devices.append(device)
		self.yang_model += device.get_yang_model()

		for rpc in device.rpcs:
			rpc_function = self.generate_rpc_function(uuid, rpc["name"])
			setattr(ncm.NetconfMethods, "rpc_" + rpc["name"], rpc_function)

		return device
	
	def generate_rpc_function(self, uuid, name):
		def rpc_function(self2, unused_session, rpc, *unused_params):
			self.logger.info("RPC %s executed" % name)
			return self.rpc_to_mqtt_bridge_function(uuid, name, unused_params)

		return rpc_function

	def rpc_to_mqtt_bridge_function(self, uuid, name, *params):
		if params == None:
			return

		try:
			device = next(d for d in self.devices if d.uuid == uuid)
			try:
				rpc = next(r for r in device.rpcs if r["name"] == name)
			except StopIteration:
				self.logger.error("Failed to execute RPC: RPC name not found.")
				return
		except StopIteration:
			self.logger.error("Failed to execute RPC: device UUID not found.")
			return

		self.current_req_id_lock.acquire()
		req_id = self.current_req_id
		self.current_req_id += 1
		self.current_req_id_lock.release()

		uuid = ""
		param_check_failed = False
		parameters = list(map(lambda p: { "tag": str(p.tag).split("}")[-1], "text": str(p.text) }, params[0]))

		if set(map(lambda p: p["tag"], parameters)) != set(map(lambda i: i["name"], rpc["inputs"])):
			response = "TOO MANY OR TOO FEW PARAMS"
		else:
			parameter_str = ""
			for param in parameters:
				if param["tag"] == "uuidInput":
					uuid = param["text"]
				else:
					parameter_str += ";" + param["text"]

			msg = str(req_id) + ";" + rpc["mqttMethod"] + parameter_str
			topic = rpc["mqttTopic"]
			self.logger.info("Publishing %s to topic %s" % (msg, topic))

			self.mqtt_client.publish(topic, msg)

			response = "NO RESPONSE FROM DEVICE"
			secs_waited = 0

			while secs_waited < config.SECS_TO_WAIT_FOR_RESPONSE:
				time.sleep(config.SECS_SINGLE_WAIT)
				secs_waited += config.SECS_SINGLE_WAIT
				if str(req_id) in self.responses:
					self.responses_lock.acquire()
					response = self.responses[str(req_id)]
					self.responses_lock.release()
					break

		root = etree.Element('data')
		retval = etree.SubElement(root, "retval")
		retval.text = response

		return root

	def delete_device(self, uuid):
		# TODO Implement
		pass


def main():
	NetconfMqttBridge().run()

if __name__ == "__main__":
	main()