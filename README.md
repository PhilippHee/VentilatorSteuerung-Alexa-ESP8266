# VentilatorSteuerung-Alexa-ESP8266

**ESP8266 als Alexa-fähige Infrarot-Fernbedienung nutzen**

Dieses kleine Projekt (Sketch) zeigt, wie man jedes nicht internet- oder Smarthome-fähige Geräte im Haushalt smarter machen kann, sodass man es über Amazon Alexa steuern kann. **Voraussetzung ist, dass das Gerät über eine Infrarot-Fernbedienung gesteuert werden kann.** Die Fernbedienung wird durch den ESP8266 (NodeMCU ESP8266) mit Infrarot-LED ersetzt.

**Das Projekt bezieht sich speziell auf folgendes Gerät und Board, lässt sich aber für jedes Gerät mit Infrarot-Fernbedienung adaptieren:**
- Ventilator *LeaderPro, 12" 3D Stand Fan (DC Motor, 26W)* ([Amazon Link](https://amzn.to/31Csbez))
- Board *NodeMCU V3 ESP8266* ([Amazon Link](https://amzn.to/3m8HipJ))

[![license](https://img.shields.io/badge/license-MIT-orange.svg)](LICENSE)

## Hardware und Schaltplan
Für dieses Projekt werden folgende Hardware-Komponenten und Bauteile verwendet:
- NodeMCU V3 ESP8266 ([Link](https://amzn.to/3m8HipJ))
- 680 &#8486; und 330 &#8486; Widerstand ([Link](https://www.reichelt.de/widerstand-kohleschicht-680-ohm-0207-250-mw-5--1-4w-680-p1460.html), [Link](https://www.reichelt.de/widerstand-kohleschicht-330-ohm-0207-250-mw-5--1-4w-330-p1410.html))
- BC337-16 Transistor ([Link](https://www.reichelt.de/bipolartransistor-npn-45v-0-8a-0-625w-to-92-bc-337-16-p4985.html))
- 940 nm 5 mm IR-LED ([Link](https://www.reichelt.de/infrarot-diode-gaalas-940-nm-50-5-mm-t-1-3-4-ir-7373c-evl-p219706.html?&nbc=1&trstct=lsbght_sldr::219705))
- Lochrasterplatine ([Link](https://www.reichelt.de/lochrasterplatine-hartpapier-75x100mm-h25pr075-p8269.html))
- 5 V DC &#8805;0,5 A Netzteil ([Link](https://amzn.to/3uZEFd8))
- DC-Buchse (optional, [Link](https://amzn.to/3mUrJSJ))
- Stiftleisten (optional, [Link](https://www.reichelt.de/stiftleiste-10-polig-vergoldet-2-54mm-bkl-10120506-p235657.html))
- Buchsenleisten (optional, [Link](https://www.reichelt.de/buchsenleiste-16-pol-gerade-rm-2-54-h-7-0-mm-bl-1x16g7-2-54-p180555.html))
- Kupferlitze (optional, [Link](https://www.reichelt.de/kupferlitze-isoliert-10-m-1-x-0-14-mm-schwarz-litze-sw-p10298.html?&trstct=vrt_pdn&nbc=1))
- LED-Einbaufassung (optional, [Link](https://www.reichelt.de/einbaufassung-fuer-5-mm-leds-aussenreflektor-schwarz-ebf-a-5-s-p7289.html))
- Gehäuse (optional)

Die Komponenten sind entsprechend folgendem Schaltplan auf der Platine miteinander zu verbinden:

![Schaltplan4](https://user-images.githubusercontent.com/81238678/115127564-bcd82900-9fd7-11eb-8827-1a1219cd77a0.png)


## Vorbereitungen
Bevor das Projekt/der Quellcode und die Platine genutzt werden können, sind ein paar Vorbereitungen zu erledigen. Hierzu zählen die Konfiguration der Arduino IDE, die Installation benötigter Bibliotheken sowie das Auslesen der Infrarot-Signale der Fernbedienung.

### Boardmanager
Die Arduino IDE ist standardmäßig nicht für den ESP8266 (NodeMCU) konfiguriert, sodass man zunächst einen zusätzlichen Boardmanager installieren muss (siehe auch [hier](https://github.com/esp8266/Arduino)). Hierfür muss man in der Arduino IDE unter *Datei > Voreinstellungen* im Reiter *Einstellungen* die folgende Boardverwalter-URL einfügen: `http://arduino.esp8266.com/stable/package_esp8266com_index.json`

Anschließend kann das entsprechende Board über *Werkzeuge > Board > Boardverwalter...* installieren werden, indem man in der Suchleiste nach *esp8266* ([**esp8266** by ESP8266 Community](https://github.com/esp8266/Arduino)) sucht. Danach muss das passende Board in der Auswahlliste (*Werkzeuge > Board*) ausgewählt werden. In diesem Projekt wird der NodeMCU V3 ESP8266 verwendet, der richtige Eintrag hierfür lautet *NodeMCU 1.0 (ESP-12E Module)*. Durch die Installation des Boards werden auch viele Bibliotheken, die spezielle für dieses Board sind, installiert, wie zum Beispiel zur Nutzung des WiFi-Moduls.

Um das Board nach dem Anschluss an den Rechner nutzen zu können, muss in der IDE zudem der richtige COM-Port unter *Werkzeuge > Port* ausgewählt werden. Da hier der NodeMCU V3 verwendet wird, wird dieses Board mit einem CH340-Chip für die USB-Schnittstelle ausgeliefert. Eine manuelle Installation des CH340-Treibers des Boards war nicht erforderlich, ist es jedoch unter Umständen.

### Zusätzliche Bibliotheken
Zusätzlich werden folgende Bibliotheken benötigt, die man ...
1. direkt über die Arduino IDE suchen und installieren kann (*Werkzeuge > Bibliotheken verwalten...*) oder
2. auf der jeweiligen GIT-Seite herunterladen und manuell zum lokalen Arduino-Bibliotheks-Verzeichnis hinzufügen kann.

(Falls eine Bibliothek nicht direkt über die IDE installiert werden kann, ist 2. zu befolgen.)

|Bibliothek|Repository|
|-|-|
|**FauxmoESP** by Paul Vint|[GIT](https://github.com/vintlabs/fauxmoESP)|
|**IRremoteESP8266** by David Conran, Sebastien Warin, Mark Szabo, Ken Shirriff|[GIT](https://github.com/crankyoldgit/IRremoteESP8266)|
|**ESPAsyncTCP** by Hristo Gochkov|[GIT](https://github.com/me-no-dev/ESPAsyncTCP)|

Hinweis: Bitte die allgemeinen Informationen sowie Hinweise zur Fehlerbehebung bei der Nutzung der jeweiligen Bibliotheken beachten!

### Infrarot-Signale auslesen
Das Auslesen der Infrarot-Signale der Fernbedienung erfolgt ebenfalls mithilfe des NodeMCU-Boards über die IRremoteESP8266-Bibliothek. Nach der Installation der Bibliothek kann in der Arduino IDE über *Datei > Beispiele > IRremoteESP8266* das Beispiel-Projekt *IRrecvDumpV2* (Copyright 2009 by Ken Shirriff | Copyright 2017-2019 by David Conran) geöffnet und genutzt werden.

Der NodeMCU muss hierfür mit einem Infrarot-Empfänger ausgestattet werden. Als Empfänger ist der TSOP4838 ([Link](https://www.reichelt.de/ir-empfaenger-module-38khz-90-side-view-tsop-4838-p158403.html)) zu empfehlen, der entsprechend der angegebenen Pin-Konfiguration mit dem NodeMCU zu verbinden ist. Nach dem Hochladen des Projektes auf den NodeMCU können über den seriellen Monitor der IDE (*Werkzeuge > Serieller Monitor*) die Infrarot-Signale ausgelesen werden. Einfach dafür die Fernbedienung auf den Empfänger richten und die Tasten drücken. Die einzelnen Signale der Fernbedienung (z.B. im Hex-Format), die Anzahl der Bits (z.B. 24) sowie das genutzte Protokoll der Fernbedienung (z.B. MIDEA24) sind zu speichern und werden später beim Senden der Signale wiederverwendet.

Hinweis: Ggf. Baudrate des seriellen Monitors anpassen!

Nachfolgend ist beispielhaft das Auslesen der Power-Taste des Ventilators abgebildet:

![IR_Power-Taste](https://user-images.githubusercontent.com/81238678/113317999-487f6380-9310-11eb-848d-b74e4246ba1a.PNG)


## Quellcode
Der Quellcode ([**VentilatorSteuerung-Alexa-ESP8266.ino**](VentilatorSteuerung-Alexa-ESP8266.ino)) unterteilt sich grob in folgende Bereiche/Funktionalitäten:
- Herstellung der WiFi-Verbindung mit dem heimischen Router
- Anlegen von Alexa-Geräten
- Überprüfung auf neue, empfangene Alexa-Befehle
- Auswertung der Befehle und Senden der entsprechenden Infrarot-Signale

Weitere Informationen sind den Kommentaren im Code zu entnehmen.

## Alexa-Routinen
Nachdem der Quellcode auf den NodeMCU geladen wurde und der NodeMCU eine Verbindung zum Router aufgebaut hat, können über die Alexa-App die neuen (virtuellen) Alexa-Geräte gesucht werden. In diesem Projekt wird nur ein (virtuelles) Geräte angelegt. Um die verschiedenen Signale der Fernbedienung (und noch ein paar eigenen Befehle) umsetzen zu können, wird die Helligkeit (in Prozent) des Alexa-Befehls abgefragt und verarbeitet.

In diesem Projekt wurden speziell für den eingesetzten Ventilator folgende Befehle und Helligkeitsstufen als Routinen in der App hinterlegt:

![Alexa-Routinen](https://user-images.githubusercontent.com/81238678/113314529-b88bea80-930c-11eb-99e0-169c4f6b15a7.png)

## Video
Auf YouTube ist ein kurzes Video zu diesem Projekt zu finden: [YouTube Link](https://www.youtube.com/watch?v=Q9eijz_0OVI)
