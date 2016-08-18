/*
   Copyright (c) 2016 Ryoya Kawai.  All rights reserved.
*/
#include <CurieBLE.h>

BLEPeripheral blePeripheral;  // BLE Peripheral Device (the board you're programming)
BLEService ledService("19B10000-E8F2-537E-4F6C-984FEE0F808E"); // BLE LED Service

// BLE LED Characteristic - custom 128-bit UUID, read and writable by central
BLECharCharacteristic ledCharacteristic("19B10001-E8F2-537E-4F6C-984FEE0F808E", BLERead | BLEWrite);

const int ledPin = 13; // pin to use for the LED

void setup() {
  Serial.begin(9600);

  // set LED pin to output mode
  pinMode(ledPin, OUTPUT);

  // set advertised local name and service UUID:
  blePeripheral.setLocalName("CandyMgc");
  blePeripheral.setAdvertisedServiceUuid(ledService.uuid());

  // add service and characteristic:
  blePeripheral.addAttribute(ledService);
  blePeripheral.addAttribute(ledCharacteristic);

  // set the initial value for the characeristic:
  ledCharacteristic.setValue(0);

  // begin advertising BLE service:
  blePeripheral.begin();

  Serial.println("BLE LED Peripheral");
}

void loop() {
  // listen for BLE peripherals to connect:
  BLECentral central = blePeripheral.central();

  // if a central is connected to peripheral:
  if (central) {
    Serial.print("Connected to central: ");
    // print the central's MAC address:
    Serial.println(central.address());

    // while the central is still connected to peripheral:
    while (central.connected()) {
      // if the remote device wrote to the characteristic,
      // use the value to control the LED:
      int duration=0;
      if (ledCharacteristic.written()) {
        if (ledCharacteristic.value()) {   // any value other than 0
          switch(ledCharacteristic.value()) {
            case 0x01:
              duration=1000;
              break;
            case 0x02:
              duration=2000;
              break;
            case 0x03:
              duration=3000;
              break;
            case 0x04:
              duration=4000;
              break;
            default:
              break;              
          }
          Serial.println(duration);

          digitalWrite(ledPin, HIGH);         // will turn the LED on
          delay(duration);
          digitalWrite(ledPin, LOW);          // will turn the LED off
          Serial.println(F("LED off"));
        }
      }
    }

    // when the central disconnects, print it out:
    Serial.print(F("Disconnected from central: "));
    Serial.println(central.address());
  }
}

