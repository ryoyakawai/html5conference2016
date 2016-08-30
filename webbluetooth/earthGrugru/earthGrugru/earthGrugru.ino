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
#include "CurieIMU.h"
#include <MadgwickAHRS.h>

#define BLE_CONNECT 13

BLEPeripheral blePeripheral;

BLEService grgrService("77b10400-5912-4391-8826-509e9bcf2204");

BLECharacteristic grgrAccCharacteristic("77b10401-5912-4391-8826-509e9bcf2204", BLERead | BLENotify, 12 );
BLEDescriptor grgrAccDescriptor("2902", "block");

Madgwick filter; // initialise Madgwick object
int ax, ay, az;
int gx, gy, gz;
float yaw, pitch, roll;
float lyaw=1000, lpitch=1000, lroll=1000;

int factor = 800; // variable by which to divide gyroscope values, used to control sensitivity

int calibrateOffsets = 1; // int to determine whether calibration takes place or not

union 
{
  float a[3];
  unsigned char bytes[12];
} oData;


void setup() {

  // initialze serial port for debugging communications
  Serial.begin(9600); // initialize Serial communication
  //while (!Serial);    // wait for the serial port to open
 
  Serial.println("Earth GruGru Started");
  
  CurieIMU.begin();

  if (calibrateOffsets == 1) {
    // use the code below to calibrate accel/gyro offset values
    Serial.println("Internal sensor offsets BEFORE calibration...");
    Serial.print(CurieIMU.getAccelerometerOffset(X_AXIS)); Serial.print("\t");
    Serial.print(CurieIMU.getAccelerometerOffset(Y_AXIS)); Serial.print("\t");
    Serial.print(CurieIMU.getAccelerometerOffset(Z_AXIS)); Serial.print("\t");
    Serial.print(CurieIMU.getGyroOffset(X_AXIS)); Serial.print("\t");
    Serial.print(CurieIMU.getGyroOffset(Y_AXIS)); Serial.print("\t");
    Serial.print(CurieIMU.getGyroOffset(Z_AXIS)); Serial.print("\t");
    Serial.println("");

    Serial.print("Starting Gyroscope calibration...");
    CurieIMU.autoCalibrateGyroOffset();
    Serial.println(" Done");
    Serial.print("Starting Acceleration calibration...");
    CurieIMU.autoCalibrateAccelerometerOffset(X_AXIS, 0);
    CurieIMU.autoCalibrateAccelerometerOffset(Y_AXIS, 0);
    CurieIMU.autoCalibrateAccelerometerOffset(Z_AXIS, 1);
    Serial.println(" Done");

    Serial.println("Internal sensor offsets AFTER calibration...");
    Serial.print(CurieIMU.getAccelerometerOffset(X_AXIS)); Serial.print("\t");
    Serial.print(CurieIMU.getAccelerometerOffset(Y_AXIS)); Serial.print("\t");
    Serial.print(CurieIMU.getAccelerometerOffset(Z_AXIS)); Serial.print("\t");
    Serial.print(CurieIMU.getGyroOffset(X_AXIS)); Serial.print("\t");
    Serial.print(CurieIMU.getGyroOffset(Y_AXIS)); Serial.print("\t");
    Serial.print(CurieIMU.getGyroOffset(Z_AXIS)); Serial.print("\t");
    Serial.println("");
  }
      
  // enable LED pins for output.
  pinMode(BLE_CONNECT, OUTPUT);

  // prepare & initiazlie BLE  
  blePeripheral.setLocalName("eGruGru");
  blePeripheral.setAdvertisedServiceUuid(grgrService.uuid());  // add the service UUID
  blePeripheral.addAttribute(grgrService);   
  blePeripheral.addAttribute(grgrAccCharacteristic);
  blePeripheral.addAttribute(grgrAccDescriptor);

  const unsigned char initializerAcc[4] = { 0,0,0,0 }; 
  grgrAccCharacteristic.setValue( initializerAcc, 12);
  
  blePeripheral.begin();  
}

void loop() {  
  readyBlinkLED(BLE_CONNECT);

  BLECentral central = blePeripheral.central();
  if (central) {
    Serial.print("Connected to central: "); 
    Serial.println(central.address());

    while (central.connected()) {
      digitalWrite(BLE_CONNECT, HIGH);

      CurieIMU.readMotionSensor(ax, ay, az, gx, gy, gz);       

      // use function from MagdwickAHRS.h to return quaternions
      filter.updateIMU(gx / factor, gy / factor, gz / factor, ax, ay, az);

      // functions to find yaw roll and pitch from quaternions
      yaw = filter.getYaw();
      roll = filter.getRoll();
      pitch = filter.getPitch();

      if(cround(abs(yaw-lyaw))>0 || cround(abs(roll-lroll))>0 || cround(abs(pitch-lpitch))>0) {

        Serial.print(yaw); Serial.print("\t");
        Serial.print(roll); Serial.print("\t");
        Serial.print(pitch); Serial.print("\n");

        oData.a[0]=yaw;
        oData.a[1]=roll;
        oData.a[2]=pitch;

        unsigned char *orientation = (unsigned char *)&oData;
        grgrAccCharacteristic.setValue(orientation, 12);
        lyaw=yaw; lroll=roll; lpitch=pitch;
      }
    }  // while (central.connected())
  } // if (central)
}

int cround(float val) {
  return (int) 8*val;
}

void readyBlinkLED(int PINNO) {
    int sumReadyLedTime=0;
  int readyLedTimer=500;
  int readyBlinkTime=60;
  while (readyLedTimer>sumReadyLedTime) {
    digitalWrite(PINNO, HIGH);
    delay(readyBlinkTime);
    digitalWrite(PINNO, LOW);
    delay(readyBlinkTime);
    digitalWrite(PINNO, HIGH);
    delay(readyBlinkTime);
    digitalWrite(PINNO, LOW);
    delay(38*readyBlinkTime);
    sumReadyLedTime+=42*readyBlinkTime;
  }
}

