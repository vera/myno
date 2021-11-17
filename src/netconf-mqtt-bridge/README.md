# NETCONF-Bridge 

## Installation 

1. Installiere die notwendigen Pakete für die NETCONF-Bridge: 

```
sudo apt install lxml lxslt
```

   Weitere Hinweise zur Installation von `lxml`, auch für MAC OS, siehe: [https://lxml.de/installation.html](https://lxml.de/installation.html)

2. Installiere die notwendigen Python-Pakete (da die Bridge und die Web-UI verschiedene Versionen benötigen, empfiehlt sich die Verwendung eines virtual environments für die Bridge):

```
python3 -m venv .venv
source .venv/bin/activate
pip3 install -r requirements.txt
```

3. Zusätzlich muss die Bibliothek `rdflib_jsonld.zip` aus dem Ordner tools/ installiert werden. Die zum Zeitpunkt des Schreibens aktuelle Version von `rdflib_jsonld` ist 0.4.0. Die Version aus tools/ beinhaltet einen Bugfix, der von der Bridge benötigt wird, um den Namespace der Ontologie richtig zu parsen.

   Wenn man ein virtual environment nutzt, befinden sich die Pakete unter: 

```
.venv/lib/python3.8/site-packages
```

   Hier kann einfach der Inhalt des Ordners mit dem Inhalt des gleichnamigen Ordners ausgetauscht werden.

   Alternative Anleitungen hierzu: siehe [Python-Dokumentation](https://docs.python.org/3.3/install/index.html)

4. Die Bridge benötigt einen privaten RSA-Schlüssel, der in einer Datei namens `host.key` vorliegen muss. Erzeugen Sie das Schlüsselpaar:

```
$ ssh-keygen -t rsa
Generating public/private rsa key pair.
Enter file in which to save the key (/home/vera/.ssh/id_rsa): host.key
Enter passphrase (empty for no passphrase): 
Enter same passphrase again: 
Your identification has been saved in host.key
Your public key has been saved in host.key.pub
```

5. Installieren Sie den MQTT-Broker `mosquitto` und starten Sie ihn:

```
sudo apt install mosquitto
service mosquitto start
```

   Für MAC OS:

```
brew install mosquitto
brew services start mosquitto
```

6. Starten Sie die NETCONF/MQTT-Bridge:

```
python3 netconf_server.py
```
