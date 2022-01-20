# NETCONF-Bridge 

## Installation 

1. Installiere die notwendigen Pakete für die NETCONF-Bridge: 

```
sudo apt install python3-lxml
```

   Weitere Hinweise zur Installation von `lxml`, auch für MAC OS, siehe: [https://lxml.de/installation.html](https://lxml.de/installation.html)

2. Installiere die notwendigen Python-Pakete (da die Bridge und der NETCONF-Client/Update-Server (`myno_web_app`) verschiedene Versionen benötigen, empfiehlt sich die Verwendung eines virtual environments für die Bridge):

```
python3 -m venv .venv
source .venv/bin/activate
pip3 install -r requirements.txt
```

3. Die Bridge benötigt einen privaten RSA-Schlüssel, der in einer Datei namens `host.key` vorliegen muss. Erzeuge das Schlüsselpaar:

```
$ ssh-keygen -t rsa
Generating public/private rsa key pair.
Enter file in which to save the key (/home/vera/.ssh/id_rsa): host.key
Enter passphrase (empty for no passphrase): 
Enter same passphrase again: 
Your identification has been saved in host.key
Your public key has been saved in host.key.pub
```

4. Installiere den MQTT-Broker `mosquitto` und starte ihn:

```
sudo apt install mosquitto
service mosquitto start
```

   Für MAC OS:

```
brew install mosquitto
brew services start mosquitto
```

5. Starte die NETCONF/MQTT-Bridge:

```
python3 netconf_mqtt_bridge.py
```

## Tests

Um die Tests auszuführen:

```
pytest
```