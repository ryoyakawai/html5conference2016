#include "CurieIMU.h"
#include <CurieBLE.h>

BLEPeripheral blePeripheral;

BLEService imuService("917649A0-D98E-11E5-9EEC-984FEE0F808E"); // Custom UUID

BLECharacteristic imuAccCharacteristic("917649A1-D98E-11E5-9EEC-984FEE0F808E", BLERead | BLENotify, 12 );
BLEDescriptor imuAccDescriptor("2902", "block");


boolean blinkState = false;          // state of the LED
unsigned long loopTime = 0;          // get the time since program started
unsigned long interruptsTime = 0;    // get the time when motion event is detected

#define BLE_CONNECT 13 // T.addAttribute(imuAccDescriptor);

int lastOrientation = - 1; // previous orientation (for comparison)

union 
{
  float a[3];
  unsigned char bytes[12];
} accData;

int pxRaw=0, pyRaw=0, pzRaw=0;
int xRaw=0, yRaw=0, zRaw=0;

void setup() {

  // initialze serial port for debugging communications
  Serial.begin(9600); // initialize Serial communication
  while (!Serial);    // wait for the serial port to open
 
  Serial.println("Earth GruGru Started");
  
  CurieIMU.begin();
  CurieIMU.attachInterrupt(eventCallback);

  // prepare & initiazlie BLE
  // enable LED pins for output.
  pinMode(BLE_CONNECT, OUTPUT);
  
  CurieIMU.setDetectionThreshold(CURIE_IMU_MOTION, 20); // 20mg
  CurieIMU.setDetectionDuration(CURIE_IMU_MOTION, 10);  // trigger times of consecutive slope data points
  CurieIMU.interrupts(CURIE_IMU_MOTION);

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
      if(pxRaw!=xRaw || pyRaw!=yRaw || pyRaw!=yRaw) {
        accData.a[0] = convertRawAcceleration(xRaw);
        accData.a[1] = convertRawAcceleration(yRaw);
        accData.a[2] = convertRawAcceleration(zRaw);
        pxRaw=xRaw;
        pyRaw=yRaw; 
        pzRaw=zRaw; 

        unsigned char *acc = (unsigned char *)&accData;
        imuAccCharacteristic.setValue( acc, 12 );
      }
    }      
  }
}

static void eventCallback(void){
    if (CurieIMU.getInterruptStatus(CURIE_IMU_MOTION)) {

      // read accelerometer values:
      xRaw = CurieIMU.readAccelerometer(X_AXIS);
      yRaw = CurieIMU.readAccelerometer(Y_AXIS);
      zRaw = CurieIMU.readAccelerometer(Z_AXIS);

/*
      // for debugging
      if (CurieIMU.motionDetected(X_AXIS, POSITIVE)) {
        Serial.print("Negative motion detected on X-axis: ");
        Serial.print(x); Serial.println();
      }
      if (CurieIMU.motionDetected(X_AXIS, NEGATIVE)) {
        Serial.print("Positive motion detected on X-axis: ");
        Serial.print(x); Serial.println();
      }
      if (CurieIMU.motionDetected(Y_AXIS, POSITIVE)) {
        Serial.print("Negative motion detected on Y-axis: ");
        Serial.print(y); Serial.println();
      }
      if (CurieIMU.motionDetected(Y_AXIS, NEGATIVE)) {
        Serial.print("Positive motion detected on Y-axis: ");
        Serial.print(y); Serial.println();
      }
      if (CurieIMU.motionDetected(Z_AXIS, POSITIVE)) {
        Serial.print("Negative motion detected on Z-axis: ");
        Serial.print(z); Serial.println();
      }
      if (CurieIMU.motionDetected(Z_AXIS, NEGATIVE)) {
        Serial.print("Positive motion detected on Z-axis: ");
        Serial.print(z); Serial.println();
      }
*/

      Serial.print("(xRaw,yRaw,zRaw): "); Serial.print(xRaw); Serial.print(","); Serial.print(yRaw); Serial.print(","); Serial.print(zRaw); Serial.print(")");Serial.println();
    }

}


float convertRawAcceleration(int aRaw) {
  // since we are using 2G range
  // -2g maps to a raw value of -32768
  // +2g maps to a raw value of 32767
  
  float a = (aRaw * 2.0) / 32768.0;

  return a;
}

