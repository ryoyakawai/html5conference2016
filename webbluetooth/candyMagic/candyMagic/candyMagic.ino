/*
  Copyright (c) 2016 Ryoya Kawai.  All rights reserved.

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
*/
#include <CurieBLE.h>

BLEPeripheral blePeripheral;  // BLE Peripheral Device (the board you're programming)
BLEService cmgcService("77b10300-5912-4391-8826-509e9bcf2204");

// BLE LED Characteristic - custom 128-bit UUID, read and writable by central
BLECharCharacteristic cmgcCharacteristic("77b10301-5912-4391-8826-509e9bcf2204", BLERead | BLEWrite);

const int motorPin = 12; // pin to use for the Candy Magic Motor
const int ledPinB = 7;
const int ledPin = 13;
const int trimPin = 0;

const int TRIM_VAL_MIN=151;
const int TRIM_VAL_MAX=1023;

void setup() {
  Serial.begin(9600);
  //while (!Serial);    // wait for the serial port to open

  Serial.println("Candy Magic Started");

  // set Motor/LED pin to output mode
  pinMode(motorPin, OUTPUT);
  pinMode(ledPinB, OUTPUT);
  pinMode(ledPin, OUTPUT);

  digitalWrite(ledPin, LOW);          // initialize LED
  digitalWrite(ledPinB, LOW);          // initialize LED

  // set advertised local name and service UUID:
  blePeripheral.setLocalName("CandyMgc");
  blePeripheral.setAdvertisedServiceUuid(cmgcService.uuid());

  // add service and characteristic:
  blePeripheral.addAttribute(cmgcService);
  blePeripheral.addAttribute(cmgcCharacteristic);

  // set the initial value for the characeristic:
  cmgcCharacteristic.setValue(0);

  // begin advertising BLE service:
  blePeripheral.begin();

}

void loop() {
  readyBlinkLED(ledPin);
  
  BLECentral central = blePeripheral.central();

  // if a central is connected to peripheral:
  if (central) {
    Serial.print("Connected to central:");
    // print the central's MAC address:
    Serial.println(central.address());

    // while the central is still connected to peripheral:
    while (central.connected()) {
      digitalWrite(ledPin, HIGH);
      // if the remote device wrote to the characteristic,
      // use the value to control the time of running moter's duration:
      int duration=0;
      if (cmgcCharacteristic.written()) {
        switch(cmgcCharacteristic.value()) {
        case 0x01:
          duration=10;
          break;
        case 0x02:
          duration=20;
          break;
        case 0x03:
          duration=30;
          break;
        case 0x04:
          duration=40;
          break;
        default:
          break;              
        }
        if (duration>0) {
          // get Trim value
          int trimedDuration = duration * map(analogRead(trimPin), TRIM_VAL_MIN, TRIM_VAL_MAX, 0, 100);
          Serial.println(trimedDuration);
            
          digitalWrite(ledPinB, HIGH);
          digitalWrite(motorPin, HIGH);
          int sumTimer=0, blinkTime=150;
          //delay(trimedDuration);
          while(trimedDuration>sumTimer) {
            digitalWrite(ledPin, LOW);
            delay(blinkTime);
            digitalWrite(ledPin, HIGH);
            delay(blinkTime);
            sumTimer+=2*blinkTime;
          }
          digitalWrite(ledPinB, LOW);
          digitalWrite(motorPin, LOW);
        }
      }
    } // central.connected();
    
    // when the central disconnects, print it out:
    Serial.print("Disconnected from central.");
    Serial.println(central.address());
  }
}

void readyBlinkLED(int PINNO) {
  int sumReadyLedTime=0;
  int readyLedTimer=500;
  int readyBlinkTime=60;
  while (readyLedTimer>sumReadyLedTime) {
    digitalWrite(PINNO, HIGH);
    delay(readyBlinkTime);
    digitalWrite(PINNO, LOW);
    delay(39*readyBlinkTime);
    sumReadyLedTime+=40*readyBlinkTime;
  }
}


