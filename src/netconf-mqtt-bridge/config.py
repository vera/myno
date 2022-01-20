"""
General config
"""
NAMESPACE = "https://www.cs.uni-potsdam.de/bs/research/myno/"
BASE_IRI = "https://www.cs.uni-potsdam.de/bs/research/myno#"

"""
MQTT config
"""
BROKER_ADDR = "127.0.0.1"
BROKER_PORT = 1883
TOPIC_DEVICE_DESCRIPTIONS = "yang/config/#"
TOPIC_RESPONSES = "response/#"
TOPIC_CREATE = ["yang/config/create", "yang/config"]
TOPIC_SUFFIX_RESPONSE = "/response/"
SECS_TO_WAIT_FOR_RESPONSE = 120
SECS_SINGLE_WAIT = 0.2

"""
NETCONF config
"""
NC_PORT = 44555 # default is 830 (RFC 6242), but using unprivileged port for testing
NC_HOST_KEY = "host.key"
NC_USERNAME = "user"
NC_PASSWORD = "admin"

"""
Logging config
"""
LOGGER_NAME = "netconf-mqtt-bridge"
LOG_DEBUG = False