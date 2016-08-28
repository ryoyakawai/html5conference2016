#include <CurieBLE.h>
#include "CurieIMU.h"
#include <MadgwickAHRS.h>

BLEPeripheral blePeripheral;

BLEService imuService("917649A0-D98E-11E5-9EEC-984FEE0F808E"); // Custom UUID

BLECharacteristic imuAccCharacteristic("917649A1-D98E-11E5-9EEC-984FEE0F808E", BLERead | BLENotify, 12 );
BLEDescriptor imuAccDescriptor("2902", "block");

Madgwick filter; // initialise Madgwick object
int ax, ay, az;
int gx, gy, gz;
float yaw, pitch, roll;
float lyaw=1000, lpitch=1000, lroll=1000;

int factor = 800; // variable by which to divide gyroscope values, used to control sensitivity
// note that an increased baud rate requires an increase in value of factor

int calibrateOffsets = 1; // int to determine whether calibration takes place or not

#define BLE_CONNECT 13 // T.addAttribute(imuAccDescriptor);

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
      
  // prepare & initiazlie BLE
  // enable LED pins for output.
  pinMode(BLE_CONNECT, OUTPUT);
  
  blePeripheral.setLocalName("eGruGru");
  blePeripheral.setAdvertisedServiceUuid(imuService.uuid());  // add the service UUID
  blePeripheral.addAttribute(imuService);   
  blePeripheral.addAttribute(imuAccCharacteristic);
  blePeripheral.addAttribute(imuAccDescriptor);

  const unsigned char initializerAcc[4] = { 0,0,0,0 }; 
  imuAccCharacteristic.setValue( initializerAcc, 12);
  
  blePeripheral.begin();  
}

void loop() {  

  BLECentral central = blePeripheral.central();
  if (central) {
    Serial.print("Connected to central: "); 
    Serial.println(central.address());

    digitalWrite(BLE_CONNECT, HIGH); // LED ON in 13PIN

    while (central.connected()) {

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
        imuAccCharacteristic.setValue(orientation, 12);
        lyaw=yaw; lroll=roll; lpitch=pitch;
      }

    }  // while (central.connected())

  } // if (central)
  
}

int cround(float val) {
  return (int) 8*val;
}

