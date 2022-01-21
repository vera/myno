"""
Silencing werkzeug logs
"""
SUPPRESS_WERKZEUG_LOGS = True

"""
UI parameters
"""
TIMESTAMP_FORMAT = '%d.%m.%Y %H:%M:%S'
NOTIFICATIONS_FADEOUT_TIME = 15 # seconds

"""
Flask config parameters
"""
html_template_file = 'interface.html'

"""
MQTT config parameters
"""
MQTT_BROKER_URL = '127.0.0.1'
MQTT_BROKER_PORT = 1883
MQTT_REFRESH_TIME = 1.0

"""
NETCONF config parameters
"""
NETCONF_IP = '127.0.0.1'
NETCONF_PORT = 44555
NETCONF_USERNAME = 'user'
NETCONF_PASSWORD = 'admin'

"""
Update server config parameters
"""
BENCHMARK_UPDATES = True

# For update slicing
UPDATE_SLICE_SIZE = 600 # Bytes

# Update slice transmission flow control
UPDATE_FLOW_CONTROL_TYPE = 0 # Options: 0 = Using MQTT ACKs, 1 = Using sleep calls
UPDATE_SLICE_ACK = "OK" # Expected ACK content (used only if UPDATE_FLOW_CONTROL_TYPE == 0)
UPDATE_SLICE_RESPONSE_TOPIC_SUFFIX = "/response" # Suffix to append to update slice topic from ontology (used only if UPDATE_FLOW_CONTROL_TYPE == 0)

# If UPDATE_FLOW_CONTROL_TYPE == 0
UPDATE_SLICE_WAIT_TIME = 120000 # milliseconds = 2 minutes
UPDATE_SLICE_RESEND_INTERVAL = 5000 # milliseconds = 5 seconds

# If UPDATE_FLOW_CONTROL_TYPE == 1
UPDATE_SLICE_SLEEP_TIME = 0.585 # seconds