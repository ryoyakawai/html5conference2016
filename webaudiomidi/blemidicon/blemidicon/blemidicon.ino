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

#define TXRX_BUF_LEN 20 //max number of bytes
#define RX_BUF_LEN 20 //max number of bytes

#define BLE_CONNECT 13
#define ccPin 0

float YAW_PRESET=3.14, YAW_MIN=0, YAW_MAX=620;
float ROLL_PRESET=1.50, ROLL_MIN=0, ROLL_MAX=294;
float PITCH_PRESET=3.14, PITCH_MIN=0, PITCH_MAX=620;

//Buffer to hold 5 bytes of MIDI data. Note the timestamp is forced
uint8_t midiBuf[] = {0x80, 0x80, 0x00, 0x00, 0x00};

BLEPeripheral midiDevice; // create peripheral instance

BLEService midiSvc("03B80E5A-EDE8-4B33-A751-6CE34EC4C700"); // create service
BLECharacteristic midiIOChar("7772E5DB-3868-4112-A1A9-F2669D106BF3", BLEWrite | BLEWriteWithoutResponse | BLENotify | BLERead, 5);

const int PIN_VAL_MIN=151;
const int PIN_VAL_MAX=1023;

BLEDescriptor imuAccDescriptor("2902", "block");

Madgwick filter; // initialise Madgwick object
int ax, ay, az;
int gx, gy, gz;
float yaw, pitch, roll;
float lyaw=1000, lpitch=1000, lroll=1000;

bool MIDIConnected=false;

int factor = 800; // variable by which to divide gyroscope values, used to control sensitivity
// note that an increased baud rate requires an increase in value of factor

int calibrateOffsets = 1; // int to determine whether calibration takes place or not

void setup() {
  Serial.begin(9600);
  //while (!Serial);    // wait for the serial port to open

  Serial.println("BLE MIDI Started");

  // enable LED pins for output.
  pinMode(BLE_CONNECT, OUTPUT);
  digitalWrite(BLE_CONNECT, LOW);

  // set alalog/digital pin to input mode
  pinMode(ccPin, INPUT);


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


  BLESetup();
  Serial.println(("Bluetooth device active, waiting for connections\n"));
}

void BLESetup()
{
  // set the local name peripheral advertises
  midiDevice.setLocalName("101Ctrlr");
  midiDevice.setDeviceName("101Ctrlr");

  // set the UUID for the service this peripheral advertises
  midiDevice.setAdvertisedServiceUuid(midiSvc.uuid());

  // add service and characteristic
  midiDevice.addAttribute(midiSvc);
  midiDevice.addAttribute(midiIOChar);

  // assign event handlers for connected, disconnected to peripheral
  midiDevice.setEventHandler(BLEConnected, midiDeviceConnectHandler);
  midiDevice.setEventHandler(BLEDisconnected, midiDeviceDisconnectHandler);

  // assign event handlers for characteristic
  midiIOChar.setEventHandler(BLEWritten, midiCharacteristicWritten);
  // set an initial value for the characteristic
  midiIOChar.setValue(midiBuf, 5);

  // advertise the service
  midiDevice.begin();
}

int l_cc=0;
int l_yawMidi=0, l_rollMidi=0, l_pitchMidi=0;

void loop() {
  readyBlinkLED(BLE_CONNECT);

  while(MIDIConnected) {
    /*
    int cc;
    // control change
    cc = map(analogRead(ccPin), PIN_VAL_MIN, PIN_VAL_MAX, 0, 127);
    // check PIN_VAL_MIN, PIN_VAL_MAX value
    // Serial.println(analogRead(ccPin));
    if(abs(l_cc-cc)>1) {
      midiBuf[2]=0xb0;
      midiBuf[3]=0x10; // 14
      midiBuf[4]=cc;
      midiIOChar.setValue(midiBuf, 5);
      l_cc=cc;
    }
    */

    // sensor data
    CurieIMU.readMotionSensor(ax, ay, az, gx, gy, gz);       

    // use function from MagdwickAHRS.h to return quaternions
    filter.updateIMU(gx / factor, gy / factor, gz / factor, ax, ay, az);

    // functions to find yaw roll and pitch from quaternions
    yaw = filter.getYaw();
    roll = filter.getRoll();
    pitch = filter.getPitch();

    if(cround(abs(yaw-lyaw))>0 || cround(abs(roll-lroll))>0 || cround(abs(pitch-lpitch))>0) {

      int yawMidi=midiYaw(yaw);
      int rollMidi=midiRoll(roll);
      int pitchMidi=midiPitch(pitch);

      midiBuf[2]=0x99;
      midiBuf[3]=rollMidi;
      midiBuf[4]=yawMidi; //pitchMidi;
      midiIOChar.setValue(midiBuf, 5);


      /*
        int yawMidi=midiYaw(yaw);
        if(abs(l_yawMidi-yawMidi)>1) {
        midiBuf[2]=0xb0;
        midiBuf[3]=0x11; // 15
        midiBuf[4]=yawMidi;
        midiIOChar.setValue(midiBuf, 5);
        l_yawMidi=yawMidi;
        }
          
        int rollMidi=midiRoll(roll);
        if(abs(l_rollMidi-rollMidi)>1) {
        midiBuf[2]=0xb0;
        midiBuf[3]=0x12; // 16
        midiBuf[4]=rollMidi;
        midiIOChar.setValue(midiBuf, 5);
        l_rollMidi=rollMidi;
        }

        int pitchMidi=midiPitch(pitch);
        if(abs(l_pitchMidi-pitchMidi)>1) {
        midiBuf[2]=0xb0;
        midiBuf[3]=0x13; // 17
        midiBuf[4]=pitchMidi;
        midiIOChar.setValue(midiBuf, 5);
        l_pitchMidi=pitchMidi;
        }
      */

      Serial.print(yawMidi); Serial.print("\t");
      Serial.print(rollMidi); Serial.print("\t");
      Serial.print(pitchMidi); Serial.print("\n");
        
      lyaw=yaw; lroll=roll; lpitch=pitch;
    }
  } // while(MIDIConnected)
}

void midiDeviceConnectHandler(BLECentral& central) {
  // central connected event handler
  MIDIConnected=true;
  digitalWrite(BLE_CONNECT, HIGH);
  Serial.print("Connected event, central\n");
  Serial.println(central.address());
}

void midiDeviceDisconnectHandler(BLECentral& central) {
  // central disconnected event handler
  MIDIConnected=false;
  digitalWrite(BLE_CONNECT, LOW);
  Serial.print("Disconnected event, central\n");
  Serial.println(central.address());
}

void midiCharacteristicWritten(BLECentral& central, BLECharacteristic& characteristic) { 
  // central wrote new value to characteristic, update LED
  Serial.print("Characteristic event, written");
}

int cround(float val) {
  return (int) 8*val;
}

int midiYaw(float val) {
  return map((int)(100*(val+YAW_PRESET)), YAW_MIN, YAW_MAX, 0, 127);
}

int midiRoll(float val) {
  return map((int)(100*(val+ROLL_PRESET)), ROLL_MIN, ROLL_MAX, 0, 127);
}

int midiPitch(float val) {
  return map((int)(100*(val+PITCH_PRESET)), PITCH_MIN, PITCH_MAX, 0, 127);
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
    delay(4*readyBlinkTime);
    digitalWrite(PINNO, HIGH);
    delay(readyBlinkTime);
    digitalWrite(PINNO, LOW);
    delay(readyBlinkTime);
    digitalWrite(PINNO, HIGH);
    delay(readyBlinkTime);
    digitalWrite(PINNO, LOW);

    delay(38*readyBlinkTime);
    sumReadyLedTime+=44*readyBlinkTime;
  }
}

