# -*- coding: utf-8 -*-#
#
# Copyright (c)
# August 2020, Kristina Sahlmann <sahlmann@uni-potsdam.de>
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

import pyang
import pyang.context
import pyang.repository
import pyang.plugin

from pyang.statements import Statement
from rdflib import Graph
from io import StringIO
from optparse import OptionParser

import config


# Define short prefixes to be used in SPARQL queries
initNs = {
	"base": config.BASE_IRI,
	"onem2m": "http://www.onem2m.org/ontology/Base_Ontology/base_ontology#",
	"rdf": "http://www.w3.org/1999/02/22-rdf-syntax-ns#",
	"om-2": "http://www.ontology-of-units-of-measure.org/resource/om-2/"
}


class MynoDevice:
	def __init__(self, dd):
		"""Initializes device from device description (JSON-LD)."""
		self.uuid = None
		self.category = None
		self.description = None
		self.rpcs = []
		self.telemetry_functions = []

		g = Graph()
		g.parse(data=dd, format="json-ld", base=config.BASE_IRI)

		# Check number of devices described
		result = g.query(
			"SELECT *"
			"WHERE {"
			" ?device rdf:type onem2m:Device ."
			" }",
			initNs=initNs)
		if len(result) != 1:
			raise ValueError("Device description contains multiple devices or no device.")

		# Get device UUID and category
		result = g.query(
			"SELECT *"
			"WHERE {"
			" ?device rdf:type onem2m:Device ."
			" ?device onem2m:hasThingProperty ?property ."
			" ?property onem2m:hasValue ?value ."
			" }",
			initNs=initNs)
		for row in result:
			if (str(row["property"]) == config.BASE_IRI + "deviceUuid"):
				self.uuid = str(row["value"])
			elif (str(row["property"]) == config.BASE_IRI + "deviceCategory"):
				self.category = str(row["value"])
			elif (str(row["property"]) == config.BASE_IRI + "deviceDesc"):
				self.description = str(row["value"])
		if not self.uuid or not self.category:
			raise ValueError("UUID or device category not found.")

		# Get RPCs (functions with operations and commands)
		result = g.query(
			"SELECT *"
			"WHERE {"
			" ?device rdf:type onem2m:Device ."
			" ?device onem2m:hasFunctionality ?function ."
			" ?service onem2m:hasOperation ?operation ."
			" ?service onem2m:exposesFunctionality ?function."
			" ?function onem2m:hasCommand ?command."
			" ?operation onem2m:exposesCommand ?command."
			" ?operation base:mqttTopic ?topic."
			" ?operation base:mqttMethod ?method."
			" ?function onem2m:hasThingProperty ?property."
			" ?property rdf:type base:YangDescription."
			" ?property onem2m:hasValue ?description."
			" }"
			" ORDER BY ?function",
			initNs=initNs)
		for row in result:
			rpc = {
				"name": row["function"].split("#")[1],
				"description": str(row["description"]),
				"mqttTopic": str(row["topic"]),
				"mqttMethod": str(row["method"]),
				"inputs": [],
				"returnStates": []
			}
			op = row["operation"].split("#")[1]
			# Get function inputs
			result = g.query(
				"""SELECT *
WHERE {
	base:%s onem2m:hasInput ?input.
	?input onem2m:hasThingProperty ?property.
	?property rdf:type base:YangDescription.
	?property onem2m:hasValue ?description.
	OPTIONAL { ?input onem2m:hasInput ?input2. ?input2 onem2m:hasDataType ?type. OPTIONAL { ?input2 onem2m:hasDataRestriction_minInclusive ?min. ?input2 onem2m:hasDataRestriction_maxInclusive ?max. } }
} ORDER BY ?input""" % op,
				initNs=initNs)
			for row in result:
				inputName = row["input"].split("#")[1]
				i = {
					"name": inputName,
					"description": str(row["description"]),
					"datatype": "string" # default
				}
				if row["min"] and row["max"]:
					i["datatypeRange"] = [str(row["min"]), str(row["max"])]
				if row["type"]:
					i["datatype"] = row["type"].split("#")[1]
				else:
					result = g.query(
						"""SELECT *
WHERE {
	base:%s onem2m:hasDataRestriction_pattern ?pattern.
} ORDER BY ?pattern""" % inputName,
						initNs=initNs)
					if len(result) > 0:
						i["datatype"] = []
					for row in result:
						i["datatype"].append(str(row["pattern"]))
				rpc["inputs"].append(i)
			# Get possible return states
			result = g.query(
				"""SELECT *
WHERE {
	base:%s onem2m:hasOperationState ?state.
	?state onem2m:hasDataRestriction_pattern ?pattern.
} ORDER BY ?pattern""" % op,
				initNs=initNs)
			for row in result:
				returnState = {
					"name": str(row["pattern"])
				}
				rpc["returnStates"].append(returnState)
			# Get return states' descriptions
			# (unfortunately they are not connected with the states themselves by any triple,
			# so they must be queried separately and matched using the alphanumeric order.)
			result = g.query(
				"""SELECT *
WHERE {
	base:%s onem2m:hasOperationState ?state.
	?state onem2m:hasThingProperty ?property.
	?property rdf:type base:YangDescription.
	?property onem2m:hasDataRestriction_pattern ?pattern.
} ORDER BY ?pattern""" % op,
				initNs=initNs)
			for i, row in enumerate(result):
				rpc["returnStates"][i]["description"] = str(row["pattern"])
			self.rpcs.append(rpc)

		# Get telemetry functions (without operations and commands)
		result = g.query(
			"SELECT *"
			"WHERE {"
			" ?device rdf:type onem2m:Device ."
			" ?device onem2m:hasFunctionality ?function ."
			" ?function onem2m:hasThingProperty ?property."
			" ?property rdf:type base:YangDescription."
			" ?property onem2m:hasValue ?description."
			" OPTIONAL { ?function onem2m:hasCommand ?command. }"
			" FILTER(!bound(?command))"
			" }"
			" ORDER BY ?function",
			initNs=initNs)
		for row in result:
			telemetry_function = {
				"name": row["function"].split("#")[1],
				"description": str(row["description"]),
				"outputs": []
			}

			result = g.query(
				"""SELECT *
WHERE {
	?service onem2m:exposesFunctionality base:%s.
	?service onem2m:hasOutputDataPoint ?dp.
	?dp base:mqttTopic ?topic.
	OPTIONAL { ?dp om-2:hasUnit ?unit. ?unit om-2:symbol ?usymbol. }
} ORDER BY ?dp""" % telemetry_function["name"],
				initNs=initNs)
			for row in result:
				output = {
					"name": row["dp"].split("#")[1],
					"topic": str(row["topic"]),
					"datatype": "string"
				}
				if row["unit"]:
					output["unit"] = str(row["unit"].split("/")[-1] + ";" + row["usymbol"])
				telemetry_function["outputs"].append(output)
			self.telemetry_functions.append(telemetry_function)

	def get(self):
		"""Returns device as dict."""
		return {
			"uuid": self.uuid,
			"description": self.description,
			"category": self.category,
			"rpcs": self.rpcs
		}

	def get_yang_model(self):
		"""Returns yang model of device."""
		yang_data = []

		result = Statement(None, None, None, u'module', u'mqtt-' + self.category)
		namespace = Statement(None, result, None, u'namespace', config.NAMESPACE + self.category)
		result.substmts.append(namespace)
		prefix = Statement(None, result, None, u'prefix', self.category)
		result.substmts.append(prefix)

		# Generate YANG for RPCs
		for rpc in self.rpcs:
			rpcstate = Statement(None, None, None, "rpc", rpc["name"])
			rpcstate.substmts.append(Statement(None, None, None, "description", rpc["description"]))
			rpcstate.substmts.append(Statement(None, None, None, "default", rpc["mqttTopic"]))

			inputstate = Statement(None, None, None, 'input', None)
			for i in rpc["inputs"]:
				typestate = Statement(None, None, None, 'leaf', i["name"])
				typestate.substmts.append(Statement(None, None, None, u'description', i["description"]))
				if type(i["datatype"]) == list:
					enumstate = Statement(None, None, None, "type", "enumeration")
					for value in i["datatype"]:
						enumstate.substmts.append(Statement(None, None, None, "enum", value))
					typestate.substmts.append(enumstate)
				elif "datatypeRange" in i:
					rangestate = Statement(None, None, None, "type", i["datatype"])
					rangestate.substmts.append(Statement(None, None, None, "range", "..".join(i["datatypeRange"])))
					typestate.substmts.append(rangestate)
				else:
					typestate.substmts.append(Statement(None, None, None, "type", i["datatype"]))
				inputstate.substmts.append(typestate)
			rpcstate.substmts.append(inputstate)

			outputstate = Statement(None, None, None, 'output', None)
			typestate = Statement(None, None, None, 'leaf', 'retval')
			enum = Statement(None, None, None, u'type', u'enumeration')
			for s in rpc["returnStates"]:
				statestate = Statement(None, None, None, u'enum', s["name"])
				statestate.substmts.append(Statement(None, None, None, u'description', s["description"]))
				enum.substmts.append(statestate)
			typestate.substmts.append(enum)
			outputstate.substmts.append(typestate)
			rpcstate.substmts.append(outputstate)

			result.substmts.append(rpcstate)

		# Generate YANG for device
		devstate = Statement(None, None, None, 'container', 'device')
		devstate.substmts.append(Statement(None, None, None, u'description', self.description))
		idstate = Statement(None, None, None, 'list', 'device-id')
		idstate.substmts.append(Statement(None, None, None, 'key', u'uuid'))
		idstatesub = Statement(None, None, None, 'leaf', u'uuid')
		idstatesub.substmts.append(Statement(None, None, None, u'default', self.uuid))
		idstatesub.substmts.append(Statement(None, None, None, u'type', u'string'))
		idstate.substmts.append(idstatesub)
		devstate.substmts.append(idstate)
		# Add category subtree
		catstate = Statement(None, None, None, 'leaf', u'device-category')
		catstate.substmts.append(Statement(None, None, None, u'description', u'Identifies the device category')) # TODO add to yang or server
		catstate.substmts.append(Statement(None, None, None, u'type', u'string'))
		devstate.substmts.append(catstate)
		result.substmts.append(devstate)

		# Add telemetry functions
		if self.telemetry_functions:
			telemstate = Statement(None, None, None, "container", "telemetry")
			for telemetry_function in self.telemetry_functions:
				telemfuncstate = Statement(None, None, None, "leaf", telemetry_function["name"])
				telemfuncstate.substmts.append(Statement(None, None, None, "description", telemetry_function["description"]))
				telemdpsstate = Statement(None, None, None, "container", "datapoint")
				for dp in telemetry_function["outputs"]:
					telemdpstate = Statement(None, None, None, "leaf", dp["name"])
					telemdpstate.substmts.append(Statement(None, None, None, "default", dp["topic"]))
					telemdpstate.substmts.append(Statement(None, None, None, "type", dp["datatype"]))
					if "unit" in dp:
						telemdpstate.substmts.append(Statement(None, None, None, "units", dp["unit"]))
					telemdpsstate.substmts.append(telemdpstate)
				telemfuncstate.substmts.append(telemdpsstate)
				telemstate.substmts.append(telemfuncstate)
			result.substmts.append(telemstate)

		yang_data.append(result)

		stream = StringIO()

		# gets filled with all availabe pyang output format plugins
		PYANG_PLUGINS = {}

		# register and initialise pyang plugin
		pyang.plugin.init([])
		for plugin in pyang.plugin.plugins:
			plugin.add_output_format(PYANG_PLUGINS)
		del plugin

		plugin = PYANG_PLUGINS['yang']

		optparser = OptionParser()
		plugin.add_opts(optparser)

		# pyang plugins also need a pyang.Context
		#ctx = pyang.Context(DummyRepository())
		ctx = pyang.context.Context(DummyRepository())

		# which offers plugin-specific options (just take defaults)
		ctx.opts = optparser.parse_args([])[0]

		# ready to serialize
		plugin.emit(ctx, yang_data, stream)

		# and return the resulting data
		stream.seek(0)
		yang = stream.getvalue()
		return yang


class DummyRepository(pyang.repository.Repository):
	"""Dummy implementation of abstract :class:`pyang.Repository`
	   for :class:`pyang.Context` instantiations
	"""

	def get_modules_and_revisions(self, ctx):
		"""Just a must-have dummy method, returning empty ``tuple``.
		Modules are directly given to pyang output plugins

		"""
		return ()