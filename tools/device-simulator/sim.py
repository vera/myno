import time
import random
import threading
import paho.mqtt.client as mqtt

# signing and verifying ecc support
import base64
import hashlib
import os
import subprocess as sp

threadLock = threading.Lock()
#DEVICE_UUID = "MYNO-UPDATE-8A12-4F4F-8F69-6B8F3C2E78DD"
DEVICE_UUID = "110A-4F4F-8F69-6B8F3C2E78ED"
BOARD_NAME = "Board10"

DEVICE_TOKEN = 123456
DEVICE_FW_VERSION = 1
DEVICE_LINK_OFFSET = 1

BROKER_ADDR = "localhost"
BROKER_PORT = 1883

# ONTOLOGY_FILE = "../../ontologies/myno-update-protocol/MYNO-Update-opt.owl"
ONTOLOGY_FILE = "../../ontologies/precision-agriculture/SensWOUnit.owl"

CONFIG_TOPIC = 'yang/config'
WILDCARD = "/#"
UPDATE_TOPIC = 'yang/update/+/' + DEVICE_UUID
CMD_TOPIC = 'led/' + DEVICE_UUID
#SENSOR_TOPIC = 'sensor/moisture/' + DEVICE_UUID
SENSOR_TOPIC = 'sensor/moisture/moisture_1/' + DEVICE_UUID
EVENT_TOPIC = 'event/sensor/moisture/moisture_1/' + DEVICE_UUID
RESPONSE_TOPIC = 'response/' + DEVICE_UUID

deviceAppID = "APP"

STATE = "OFF"
savedHash = ""

# Save Slots that simulate the device
app_Id=""
link_Offset=""
hashManifest=""
size=""
version=""
old_version =""
innerSignature=""
deviceNonceManifest=""
outerSignature=""


def mqtt_connect(mqtt_client, userdata, flags, rc):
    print("Sub to " + "led/" + DEVICE_UUID)
    mqtt_client.subscribe("led/" + DEVICE_UUID)
    print("Sub to " + UPDATE_TOPIC)
    mqtt_client.subscribe(UPDATE_TOPIC)
    print("Sub to " + "relay/MYNO-UPDATE-8A12-4F4F-8F69-6B8F3C2E78DD")
    mqtt_client.subscribe("relay/MYNO-UPDATE-8A12-4F4F-8F69-6B8F3C2E78DD")


def checkManifest():
    global app_Id, link_Offset, hashManifest, size, version, old_version, innerSignature, deviceNonceManifest, outerSignature
    global savedHash

    try:
        if(app_Id != deviceAppID):
            return "WRONG-APP-ID"
        elif(deviceNonceManifest != str(DEVICE_TOKEN)):
            return "INVALID-NONCE"
        # validate inner signature
        innerContent = [app_Id, link_Offset, hashManifest, size, version, old_version]
        outerContent = []
        print("[+] innerSignature to verify:", innerSignature)
        dataToVerify = ";".join(innerContent)
        print("[+] verifying data:", dataToVerify)
        hashString = hashlib.sha256(dataToVerify.encode()).hexdigest()
        try:
            command = "./createSig " + 'verifyHash' ' 1 ' + hashString + " " + innerSignature
            rc = os.system(command)
        except Exception as e:
            print(e)
            return "INNER-SIGN-INVALID"
        if(rc != 0):
            print("[+] Inner Verification failed!")
            #return "OUTER-SIGN-INVALID"

        print("[\N{HEAVY CHECK MARK}] Validation of inner Signature succeeded")

        outerContent = innerContent
        outerContent.append(innerSignature)
        outerContent.append(deviceNonceManifest)
        dataToVerify = ";".join(outerContent)

        hashString = hashlib.sha256(dataToVerify.encode()).hexdigest()
        print("[+] signed String to verify:", dataToVerify)

        try:
            command = "./createSig " + 'verifyHash' ' 2 "' + hashString + '" "' + outerSignature +'"'
            rc = os.system(command)
        except Exception as e:
            print(e)
            #return "OUTER-SIGN-INVALID"
        if(rc != 0):
            print("[+] Outer Verification failed!")
            return "OUTER-SIGN-INVALID"
        print("[\N{HEAVY CHECK MARK}] Validation of outer Signature succeeded")

        if(link_Offset != '0' and link_Offset != str(DEVICE_LINK_OFFSET)):
            return "SAVESLOT-IN-USE"
        if(int(version) <= DEVICE_FW_VERSION ):
            return "INVALID-VERSIONNR"

        print("[\N{HEAVY CHECK MARK}] Validation of Manifest succeeded")
        return "MANIFEST-SUCCESS"

    except Exception as ex:
        print(ex)
        return "ERROR"

def checkImage(imagestring):
    global savedHash
    image = base64.b16decode(imagestring.strip("\n"))
    m = hashlib.sha256()
    m.update(image)
    print("SHA256: {0}".format(m.hexdigest()))

    new_hash = m.hexdigest()

    if(new_hash == savedHash):
        return "OK"
    else:
        return "IMAGE-INVALID"

def mqtt_message(mqtt_client, userdata, msg):
    global DEVICE_TOKEN, app_Id, link_Offset, hashManifest, size, version
    global old_version, innerSignature, deviceNonceManifest, outerSignature

    print("[+] got mqtt message: ", str(msg.payload.decode()))
    global STATE
    rawcmd = msg.payload.decode().split(';')
    seq_id = rawcmd[0]
    cmd = rawcmd[1]
    inputs = rawcmd[2:]

    # give out device token
    if cmd == "GET-DEVICE-TOKEN":
        print("CTRL RECEIVED: GET-DEVICE-TOKEN")
        responseString = seq_id + ";" + str(DEVICE_TOKEN) + "," + str(DEVICE_FW_VERSION) + ";OK"
        mqtt_client.publish(RESPONSE_TOPIC, responseString)

    # validate update image
    elif cmd == "PUB-UPDATE-IMAGE":
        print("CTRL RECEIVED: PUB-UPDATE-IMAGE")
        print(inputs[0])
        #validSignature = checkImage(inputs[0])
        #mqtt_client.publish(RESPONSE_TOPIC, seq_id + validSignature)
        try:
            mqtt_client.publish(RESPONSE_TOPIC, seq_id + ";OK")
        except Exception as e:
            print(e)

    # validate manifest and signatures
    elif cmd == "PUB-UPDATE-MANIFEST":
        print("CTRL RECEIVED: PUB-UPDATE-MANIFEST")
        try:
            validSignature = ""
            print(inputs)
            number = int(str(inputs[0]).split(",")[0])
            value = str(inputs[0]).split(",")[1]
            if (number == 0):
                app_Id = value
                validSignature = "OK"
            elif(number == 1):
                link_Offset = value
                validSignature = "OK"
            elif(number == 2):
                hashManifest = value
                validSignature = "OK"
            elif (number == 3):
                size = value
                validSignature = "OK"
            elif (number == 4):
                version = value
                validSignature = "OK"
            elif (number == 5):
                old_version = value
                validSignature = "OK"
            elif (number == 6):
                innerSignature = value
                validSignature = "OK"
            elif (number == 7):
                deviceNonceManifest = value
                validSignature = "OK"
            elif (number == 8):
                outerSignature = value
                validSignature = "OK"
            elif(number == 9):
                print("[+] Starting Validation")
                validSignature = checkManifest()

            mqtt_client.publish(RESPONSE_TOPIC, seq_id + ";" + validSignature)
        except Exception as e:
            print(e)
    else:
        print("CTRL RECEIVED, UNKNOWN CTRL")
        mqtt_client.publish(RESPONSE_TOPIC, seq_id + ";ERROR")


def sensor_loop():
    while True:
        sensor_reading = random.randint(0,100)
        if sensor_reading <= 50:
            mqtt_client.publish(EVENT_TOPIC, "Event: Default. {:.2f} <= 50.00. funcPump_1On".format(sensor_reading))
        else:
            mqtt_client.publish(EVENT_TOPIC, "")
        mqtt_client.publish(SENSOR_TOPIC, sensor_reading)
        time.sleep(5)


if __name__ == "__main__":
    global pubkeyManufacturer
    global pubkeyUpdateServer

    mqtt_client = mqtt.Client()
    mqtt_client.on_connect = mqtt_connect
    mqtt_client.on_message = mqtt_message
    mqtt_client.connect(BROKER_ADDR, BROKER_PORT, 60)


    with open(ONTOLOGY_FILE) as data_file:
        Ontologystring = str(data_file.read())
        ontology = DEVICE_UUID + ";" + Ontologystring.replace("%sBoardName", BOARD_NAME).replace("%s", DEVICE_UUID)
        mqtt_client.publish(CONFIG_TOPIC, ontology)
        mqtt_client.publish(CONFIG_TOPIC, DEVICE_UUID + ";END")
        print("Ontology published")

    sensor_handler = threading.Thread(target=sensor_loop)
    sensor_handler.daemon = True
    sensor_handler.start()

    run = True
    while run:
        mqtt_client.loop()

    #mqtt_client.loop_start()
    #sensor_handler.join()
