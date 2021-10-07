import copy
import hashlib
import logging
import subprocess
import time

from lxml import etree # Needed because ncclient sends lxml, not xml object
from ncclient import xml_

from . import config

# For benchmarking
benchmark_array_slices = []
benchmark_array_whole = []
start_update = 0
end_update = 0

unacked_slices = {}

def publish_image(thing, function, slice_topic, image_file):
  """
  Publishes firmware image in slices.
  """
  global unacked_slices
  global start_update, end_update
  global benchmark_array_slices, benchmark_array_whole

  i = 0
  response_topic = slice_topic + UPDATE_SLICE_RESPONSE_TOPIC_SUFFIX
  unacked_slices[response_topic] = []

  if config.BENCHMARK_UPDATES:
    benchmark_array_slices.clear()
    start_update = time.time()

  if config.UPDATE_FLOW_CONTROL_TYPE == 0:
    mqtt_client.subscribe(response_topic)

  while True:
    if config.BENCHMARK_UPDATES:
      start_part = time.time()

    slice = image_file.read(config.UPDATE_SLICE_SIZE)
    if not slice:
      break

    msg = struct.pack(str(len(str(i))) + 'sc' + str(len(slice)) + 's', bytes(str(i), 'utf-8'), bytes(',', 'utf-8'), slice)
    mqtt_client.publish(slice_topic, msg)

    if config.UPDATE_FLOW_CONTROL_TYPE == 0:
      # Wait for response
      unacked_slices[response_topic].append(i)
      milliseconds_waited = 0
      # Time out after 2 minutes if no ACK received
      while milliseconds_waited < config.UPDATE_SLICE_WAIT_TIME:
        if i in unacked_slices[response_topic]:
          time.sleep(0.1)
          milliseconds_waited += 100
          # Re-send slice
          if milliseconds_waited % config.UPDATE_SLICE_RESEND_INTERVAL == 0:
            mqtt_client.publish(slice_topic, msg)
        else:
          break
      if i not in config.unacked_slices[response_topic]:
        i += 1
      else:
        break
    elif config.UPDATE_FLOW_CONTROL_TYPE == 1:
      time.sleep(config.UPDATE_SLICE_SLEEP_TIME)
      i += 1

    if config.BENCHMARK_UPDATES:
      end_part = time.time()
      benchmark_array_slices.append(end_part - start_part)

  if UPDATE_FLOW_CONTROL_TYPE == 0:
    mqtt_client.unsubscribe(response_topic)

  if len(unacked_slices[response_topic]) == 0 or UPDATE_FLOW_CONTROL_TYPE == 1:
    # Build final "FIN" RPC
    n = xml_.to_ele('<' + function + '/>')
    child = etree.SubElement(n, "uuidInput")
    child.text = thing
    inputParameters = etree.SubElement(n, "inputUpdateImage")
    inputParameters.text = "FIN"
    return copy.deepcopy(n)
  else:
    logging.error("Update transmission aborted due to timeout of slice " + str(i) + ".")
    return None

def build_manifest_rpcs(thing, function, form_items):
  """
  Builds manifest RPCs.
  """
  # Build extended manifest and RPCs for all manifest fields
  extendedManifest = ""
  manifest_rpcs = []
  i = 0
  for key, value in form_items:
    extendedManifest += value + ";"
    xml_rpc = xml_.to_ele('<' + function + '/>')
    child = etree.SubElement(xml_rpc, "uuidInput")
    child.text = thing
    inputParameters = etree.SubElement(xml_rpc, key)
    inputParameters.text = str(i) + "," + value
    manifest_rpcs.append(copy.deepcopy(xml_rpc))
    i += 1
  extendedManifest = extendedManifest.rstrip(';')

  # Add outer signature if necessary
  if("input_9_OuterSignature" in request.form.keys()):
    pass
  xml_rpc = xml_.to_ele('<' + function + '/>')
  child = etree.SubElement(xml_rpc, "uuidInput")
  child.text = thing
  hashString = hashlib.sha256(extendedManifest.encode()).hexdigest()
  out = subprocess.Popen(['./createSig', 'signU', extendedManifest], stdout=subprocess.PIPE)
  stdout, stderr = out.communicate()
  outerSignature = str(stdout.decode("utf-8")).strip("\n")
  inputParameters = etree.SubElement(xml_rpc, "input_9_OuterSignature")
  inputParameters.text = str(i) + "," + outerSignature
  manifest_rpcs.append(copy.deepcopy(xml_rpc))
  i += 1

  # Generate final "START-UPDATE" RPC
  xml_rpc = xml_.to_ele('<' + function + '/>')
  child = etree.SubElement(n, "uuidInput")
  child.text = thing
  inputParameters = etree.SubElement(xml_rpc, "input_9_OuterSignature")
  inputParameters.text = str(i) + "," + "START-UPDATE"
  manifest_rpcs.append(copy.deepcopy(xml_rpc))

  return manifest_rpcs