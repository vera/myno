v.0.3 

### Installation 

Installiere die notwendigen Pakete für den NETCONF Client: 

```
pip3 install -r requirements.txt
```

Kompiliere und kopiere das Dienstprogramm `createSig` für das Erstellen der Manifest-Signaturen:

```
cd ../tools/micro-ecc-signature/
make
cp createSig ../../NETCONF-Client/
```

Dabei sind auch die Hinweise in der dort abgelegten `Readme.md` zum Thema Schlüssel für Update-Server und Hersteller-Server zu beachten.

Starte den NETCONF-Client:

```
python3 __init__.py
```

Jetzt kann die Weboberfläche unter 

http://127.0.0.1:5000 erreicht werden. 

Für Testzwecke können die folgenden Daten (ohne "") genutzt werden: 

```bash
input_1_AppId="APP"
input_2_LinkOffset="0"
input_3_Hash="17c36f0ade2e54c614c2c5aef5e39cbc24d228c954222bf985026bf4fe18540d"
input_4_Size="100719"
input_5_Version="2"
input_6_OldVersion="1"
input_7_InnerSignature="786a4ae4c33882718f91d28a8a1bef846f98edb2e61760c413d5642d388632218496cd5d1502f712f322c0d57baf98c1c149e48d94b40410dd85399ba7cd774d"
input_8_DeviceNonce="123456"
input_9_OuterSignature=""
```



v.0.2
----------------RUN----------------
Edit IP address (and maybe auth) for NETCONF-Server and MQTT in __init__.py lines 35-42.
Runs with python 2.7 as well as 3.5, call 
	python __init__.py
to launch webserver. In your browser, navigate to 
	127.0.0.1:5000
to access web interface.
