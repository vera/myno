#### Netconf-Simulator

```
Autor: Alexander Lindemann 
Angepasst durch: Michael Nowak
```

Das Python Skript publiziert die Ontologie im Ordner auf den hinterlegtem MQTT-Broker. 

Zusätzlich wird das publiziertes Manifest getestet, ein Device-Token verschickt. Die Prüfung des Images erfolgt noch nicht, hier geht der Simulator noch von einer Übertragung als ganzes aus. Die Übertragung erfolgt jedoch in Teilen, wie auch beim Manifest.  

Hierfür werden die Signaturen und anderen Datenfelder des Manifests getestet. 

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

Der Output von sim.py sollte bei der Übergabe der Testwerte so aussehen: 

```
[+] got mqtt message:  80;PUB-UPDATE-MANIFEST;415050;30;30623761326434346364306463366630306137663137623338333734376338396332346131643231663737363532646265653631666562363263613635383934;353535;32;31;304402202FD3158BDAE044E6F14A5F04A0A81A52BBA38E834C3AB4EDDDADC5F7D5A841B702205EF8C6447114C7865D0A7825B74EEA832F72182EE4B2754FEF8F1DD0BB7B24BF;313233343536;3044022079A7B0221E661CDAC8D4372498B118C05ECD333EF86308E94A7857AD46DFA6B0022077AD0DB36985B582D15A1A200E8473F9D5C7DF54BD36F20B3219C75CF8D8A6C4
CTRL RECEIVED: PUB-UPDATE-MANIFEST
[+] innerSignature to verify: 304402202FD3158BDAE044E6F14A5F04A0A81A52BBA38E834C3AB4EDDDADC5F7D5A841B702205EF8C6447114C7865D0A7825B74EEA832F72182EE4B2754FEF8F1DD0BB7B24BF
[+] verifying data: APP;0;0b7a2d44cd0dc6f00a7f17b383747c89c24a1d21f77652dbee61feb62ca65894;555;2;1
[✔] Validation of inner Signature succeeded
[+] signed String to verify: APP;0;0b7a2d44cd0dc6f00a7f17b383747c89c24a1d21f77652dbee61feb62ca65894;555;2;1;304402202FD3158BDAE044E6F14A5F04A0A81A52BBA38E834C3AB4EDDDADC5F7D5A841B702205EF8C6447114C7865D0A7825B74EEA832F72182EE4B2754FEF8F1DD0BB7B24BF;123456
[✔] Validation of outer Signature succeeded
[✔] Validation of Manifest succeeded
```

