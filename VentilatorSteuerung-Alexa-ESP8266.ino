/*
VentilatorSteuerung-Alexa-ESP8266

Copyright (C) 2021 PhilippHee


The MIT License (MIT)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.


Board:
- NodeMCU 1.0 (ESP-12E Module) (esp8266 by ESP8266 Community)
  https://github.com/esp8266/Arduino

Additionally used libraries / Zusaetzlich verwendete Bibliotheken:
- FauxmoESP by Paul Vint
  https://github.com/vintlabs/fauxmoESP
- IRremoteESP8266 by David Conran, Sebastien Warin, Mark Szabo, Ken Shirriff
  https://github.com/crankyoldgit/IRremoteESP8266
- ESPAsyncTCP by Hristo Gochkov
  https://github.com/me-no-dev/ESPAsyncTCP


BESCHREIBUNG DES PROJEKTES:

Idee:
Nicht Smarthome- oder internetfaehige Geraete im Haushalt smarter machen, sodass
man diese ueber Alexa steuern kann.

Voraussetzung:
Das Geraet laesst sich per Infrarot-Fernbedienung steuern.

Ablauf:
Alexa-Sprachbefehl -> Mikrocontroller-Board (ESP8266) mit IR-Sender -> Geraet (hier: Ventilator)

Funktionsweise:
Der Mikrocontroller (NodeMCU-Board) wird von Alexa als kompatibles Geraet erkannt und kann
somit ueber Alexa gesteuert werden. Zu jedem Alexa-Befehl (Routine) gibt es intern im 
Mikrocontroller-Board ein passendes Infrarot-Signal (z.B. Ventilator schneller drehen 
lassen), das ueber eine IR-LED an den Ventilator gesendet wird. Das Mikrocontroller-Board 
ersetzt die Infrarot-Fernbedienung.

Stand:
01.04.2021

Ventilator-Typ:
LeaderPro, 12" 3D Stand Fan (DC Motor, 26W)

*/


//Bibliotheken
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <fauxmoESP.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>

#define SERIAL_BAUDRATE 115200

//fauxmoESP Objekt anlegen
fauxmoESP fauxmo;

//WIFI-Verbindungsdaten
#define WIFI_SSID "SSID hier eintragen"
#define WIFI_PASS "Passwort hier eintragen"

//GPIO-Pin, der zum Senden der Signale verwendet werden soll. Empfohlen: 4 (D2)
IRsend irsend(4);      

//IR-Signale koennen nach dem Einbinden der Bibliotheken ueber passende Beispielprogramme 
//(Datei -> Beispiele -> IRremoteESP8266 -> IRrecvDumpV2) eingelesen/decodiert werden.
//IR-Codes (in Hex) des Ventilators:
#define IR_POWER          0x80C8C8
#define IR_TIMER          0x80C0C0
#define IR_SCHNELLER      0x80B1B1
#define IR_LANGSAMER      0x80D0D0
#define IR_HOCH_RUNTER    0x808888
#define IR_LINKS_RECHTS   0x80B0B0
#define IR_MODUS          0x806060

#define IR_BITS 24 //Anzahl Bits
#define IR_WDH  3  //Anzahl Wiederholungen

//Delay in ms zwischen dem Senden von mehrfachen IR-Befehlen (bei ...plus X / ...minus X).
#define IR_DELAY_MS 100 

//Variablen, damit Befehle global verfuegbar sind
bool merkerNeuerBefehl = false;
unsigned char befehlValue = 0;

int i = 0; //Zaehlervariable


void wifiSetup() {
  //Setze WIFI-Module auf STA mode
  WiFi.mode(WIFI_STA);
 
  //Verbinden
  Serial.printf("[WIFI] Verbinden mit %s ", WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
 
  //Warten bis Verbindung hergestellt wurde
  while (WiFi.status() != WL_CONNECTED) {
      Serial.print(".");
      delay(100);
  }
  Serial.println();
 
  //Verbunden!
  Serial.printf("[WIFI] STATION Mode, SSID: %s, IP-Adresse: %s\n", WiFi.SSID().c_str(), WiFi.localIP().toString().c_str());
}

 
void setup() {
  delay(1500);
 
  Serial.begin(SERIAL_BAUDRATE);

  //IR
  irsend.begin();

  //WIFI
  wifiSetup();

  //Alexa-Geraete definieren - fauxmo
  Serial.println("fauxmo Setup");
  fauxmo.addDevice("VentiArduino"); //Geraet mit Namen "VentiArduino" hinzufuegen

  fauxmo.setPort(80);  //erforderlich fuer gen3-Geraete
  fauxmo.enable(true); //fauxmo aktiv schalten
  
   fauxmo.onSetState([](unsigned char device_id, const char * device_name, bool state, unsigned char value) 
   {
      //value (4 bis 254) in ganzzahligen Prozentwert (1 bis 100 %) umrechnen
      unsigned char valueProzent = (unsigned char)((float)value * 0.396 - 0.584 + 0.5); //(+ 0.5 zum Runden)
      Serial.printf("[MAIN] Geraet #%d (%s) Status: %s Value: %d (%d %%)\n", device_id, device_name, state ? "ON" : "OFF", value, valueProzent);

      if ((strcmp(device_name, "VentiArduino") == 0 && valueProzent <= 100 && valueProzent >= 1)) {
        //Den Wert von value bzw. valueProzent fuer die verschiedenen Zustaende/Tasten der Fernbedienung nutzen,
        //valueProzent liegt zwischen 1% und 100%.
        //Die eigentliche Umsetzung der Befehle erfolgt in der loop() (siehe unten),
        //damit hier die Rueckmeldung an Alexa schneller erfolgt.
        befehlValue = valueProzent;
        merkerNeuerBefehl = true;
      }      
   }); 
}

 
void loop() {
  fauxmo.handle(); //fauxmo Event-Loop

  //Wenn neuer Befehl empfangen wird, entsprechendes IR-Signal senden.
  if (merkerNeuerBefehl) {
    merkerNeuerBefehl = false;

    if (befehlValue == 10) {  //10% (Timer einfach)
      //Je nach IR-Fernbedienung kann sich die aufzurufende Methode unterscheiden.
      //Dekodierungs-Typ (Protokoll) beim Einlesen der Fernbedienung beachten!
      irsend.sendMidea24(IR_TIMER, IR_BITS, IR_WDH);
    
    } else if (befehlValue >= 11 && befehlValue <= 18) { //11-18% (Timer mehrfach)
      int val = 0;

      switch (befehlValue) {
        case 11:
          val = 1;
          break;
        case 12:
          val = 2;
          break;
        case 13:
          val = 3;
          break;
        case 14:
          val = 4;
          break;
        case 15:
          val = 5;
          break;        
        case 16:
          val = 6;
          break;      
        case 17:
          val = 7;
          break;
        case 18:
          val = 8;
          break;
        default:
          val = 0;
          break;
      }
                      
      irsend.sendMidea24(IR_TIMER, IR_BITS, IR_WDH);
      for (i = 0; i < val; i++) {
        delay(IR_DELAY_MS);
        irsend.sendMidea24(IR_TIMER, IR_BITS, IR_WDH);
      }

    } else if (befehlValue == 20) { //20% (Modus)
      irsend.sendMidea24(IR_MODUS, IR_BITS, IR_WDH);
      
    } else if (befehlValue >= 30 && befehlValue <= 33) { //30-33% (Langsamer einfach/mehrfach)
      int val = 0;
      
      switch (befehlValue) {
        case 30:
          val = 1;
          break;
        case 31:
          val = 3;
          break;
        case 32:
          val = 5;
          break;
        case 33:
          val = 10;
          break;
        default:
          val = 0;
          break;
      }
                      
      irsend.sendMidea24(IR_LANGSAMER, IR_BITS, IR_WDH);
      for (i = 1; i < val; i++) {
        delay(IR_DELAY_MS);
        irsend.sendMidea24(IR_LANGSAMER, IR_BITS, IR_WDH);
      }
      
    } else if (befehlValue >= 40 && befehlValue <= 43) { //40-43% (Schneller einfach/mehrfach)
      int val = 0;

      switch (befehlValue) {
        case 40:
          val = 1;
          break;
        case 41:
          val = 3;
          break;
        case 42:
          val = 5;
          break;
        case 43:
          val = 10;
          break;
        default:
          val = 0;
          break;
      }
                      
      irsend.sendMidea24(IR_SCHNELLER, IR_BITS, IR_WDH);
      for (i = 1; i < val; i++) {
        delay(IR_DELAY_MS);
        irsend.sendMidea24(IR_SCHNELLER, IR_BITS, IR_WDH);
      }
      
    } else if (befehlValue == 50) {  //50% (Power)
      irsend.sendMidea24(IR_POWER, IR_BITS, IR_WDH);
      
    } else if (befehlValue == 60) {  //60% (Bewegung Hoch-Runter)
      irsend.sendMidea24(IR_HOCH_RUNTER, IR_BITS, IR_WDH);
      
    } else if (befehlValue == 70) {  //70% (Bewegung Links-Rechts)
      irsend.sendMidea24(IR_LINKS_RECHTS, IR_BITS, IR_WDH);
      
    }
       
  }
}
