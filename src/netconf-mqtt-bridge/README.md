# NETCONF-Bridge 

## Installation 

Installiere die notwendigen Pakete für die NETCONF-Bridge: 

```
pip3 install -r requirements.txt
```

Zusätzlich muss die Bibliothek `rdflib_jsonld.zip` aus dem Ordner tools installiert werden. Diese beinhaltet einen Bugfix und wird durch den netconf_server benötigt um den Namespace der Ontologie richtig zu parsen. 



Starten Sie mosquitto und dann den NETCONF-Client:

```
mosquitto
python3 netconf_server.py
```



Die Bridge braucht eine modifizierte Version der [```rdflib_jsonld```](https://github.com/RDFLib/rdflib-jsonld). Die zum Zeitpunkt des schreibens aktuelle Version ist ```0.4.0```. Es ist jedoch ein Fix für das handling der json Dateien und die namespaces erforderlich. 

Hierfür muss das Packet manuell aus  ```../tools/rdflib_jsonld.zip``` im eigenen Pfad installiert werden. 

Anleitungen hierzu: 

[Python Dokumentation](https://docs.python.org/3.3/install/index.html)

Wenn man pycharm mit eigenem venv nutzt befinden sich die pakete meist unter: 

```bash
venv/lib/python3.8/site-packages
```

hier kann einfach der Inhalt des Ordners mit dem Inhalt des gleichnamigen Ordners ausgetauscht werden. Beim ausführen sollte aber nochmal sichergestellt werden das die richtige Libary genutzt wird.  
