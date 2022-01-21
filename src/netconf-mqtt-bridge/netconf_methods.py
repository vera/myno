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

from lxml import etree
from netconf import server as netconf_server

import config
import netconf_mqtt_bridge as nmb

class NetconfMethods(netconf_server.NetconfMethods):
	def __init__(self, bridge):
		super()
		self.bridge = bridge

	# Return device list with UUIDs and category
	def rpc_get(self, unused_session, rpc, *unused_params):
		root = etree.Element("data")

		for device in self.bridge.devices:	
			child1 = etree.SubElement(root, "device", xmlns=config.NAMESPACE + device.category)
			child2 = etree.SubElement(child1, "device-id")
			child3 = etree.SubElement(child2, "uuid")
			child3.text = device.uuid
			child4 = etree.SubElement(child1, "device-category")
			child4.text = device.category

		return root

	# Return schema (YANG model)
	def rpc_get_schema(self, unused_session, rpc, *unused_params):
		root = etree.Element("data", xmlns="urn:ietf:params:xml:ns:yang:ietf-netconf-monitoring")
		root.text = etree.CDATA(self.bridge.yang_model)
		return root

	# TODO Implement ping