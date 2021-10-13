APPLICATION = myno_co2

# If no BOARD is found in the environment, use this default:
BOARD ?= nrf52840dk

# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= ./RIOT

# Comment this out to disable code in RIOT that does safety checking
# which is not needed in a production environment but helps in the
# development process:
DEVELHELP ?= 1

# Change this to 0 show compiler invocation lines by default:
QUIET ?= 1

# MYNO config
MYNO_ONTOLOGY_TOPIC ?= yang/config/create
CFLAGS += -DMYNO_ONTOLOGY_TOPIC=\"$(MYNO_ONTOLOGY_TOPIC)\"
# Note: The device ID must additionally be changed in ontology.h
MYNO_DEVICE_ID ?= A6-49-84-70-4B-16-F3-F4
CFLAGS += -DMYNO_DEVICE_ID=\"$(MYNO_DEVICE_ID)\"
MYNO_CO2_SENSOR_TOPIC ?= sensor/co2/co2_1/$(MYNO_DEVICE_ID)
CFLAGS += -DMYNO_CO2_SENSOR_TOPIC=\"$(MYNO_CO2_SENSOR_TOPIC)\"

# IP address to be assigned to the wireless interface
WIRELESS_IP ?= 2001:db8::1
CFLAGS += -DWIRELESS_IP=\"$(WIRELESS_IP)\"
WIRELESS_PREFIX_LENGTH ?= 64
CFLAGS += -DWIRELESS_PREFIX_LENGTH=\"$(WIRELESS_PREFIX_LENGTH)\"
# Link-local IP address of the border router
BORDER_ROUTER_LL_IP ?= fe80::1
CFLAGS += -DBORDER_ROUTER_LL_IP=\"$(BORDER_ROUTER_LL_IP)\"

# MQTT config
MQTT_BROKER_IP ?= 2001:db8::2
CFLAGS += -DMQTT_BROKER_IP=\"$(MQTT_BROKER_IP)\"
ifneq (,$(MQTT_CLIENT_ID))
  CFLAGS += -DMQTT_CLIENT_ID=\"$(MQTT_CLIENT_ID)\"
else
  CFLAGS += -DMQTT_CLIENT_ID=\"$(MYNO_DEVICE_ID)\"
endif
ifneq (,$(MQTT_USER))
  CFLAGS += -DMQTT_USER=\"$(MQTT_USER)\"
endif
ifneq (,$(MQTT_PWD))
  CFLAGS += -DMQTT_PWD=\"$(MQTT_PWD)\"
endif

USEMODULE += shell
USEMODULE += shell_commands
USEMODULE += ps
USEMODULE += netdev_default
USEPKG += paho-mqtt

USEMODULE += auto_init_gnrc_netif
# Specify the mandatory networking modules for IPv6 and TCP
USEMODULE += gnrc_ipv6_router_default
USEMODULE += sock_tcp
# Additional networking modules that can be dropped if not needed
USEMODULE += gnrc_icmpv6_echo

# CO2 sensor setup
USEMODULE += mq135
MQ135_ADC_LINE  ?= ADC_LINE\(0\)
CFLAGS += -DMQ135_ADC_LINE=$(MQ135_ADC_LINE)

####

include $(RIOTBASE)/Makefile.include
