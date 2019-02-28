#include <SPI.h>
#include <BLEPeripheral.h>
#include <Servo.h>

// defining the pins
#define MOTOR_PIN     12
#define BUTTON_PIN    7
#define BLE_REQ       11
#define BLE_RDY       2 
#define BLE_RST       10 

// Defines Tirg and Echo pins of the Ultrasonic Sensor
const int trigPin = 8;
const int echoPin = 9;

int currentState;
int debounceState; // intial state of the button should be OFF
int switchState = 0;
int motorState = 0;
long duration;
int distance;
int val;
int match;
// Secert code variables
char oneRot[] = "one";
char fiveRot[] = "5"; 
char tenRot[] = "5"; 

 
long openTime = 0;
Servo myServo; // Creates a servo object for controlling the servo motor

// Lets the arduino know where the chip is
BLEPeripheral blePeripheral = BLEPeripheral( BLE_REQ, BLE_RDY, BLE_RST);
BLEService motorswitch = BLEService( "FF45" );

// BLE Written 
BLEService motorWrite = BLEService ("D270");
BLECharacteristic writeCharacteristic = BLECharacteristic("D271",BLEWrite, 20);


// Adding BLE Characteristics
BLECharCharacteristic switchCharacteristic = BLECharCharacteristic("FF69", BLERead | BLEWrite);
BLECharCharacteristic stateCharacteristic = BLECharCharacteristic("D271", BLENotify);

// Adding BLE Descriptors 
BLEDescriptor switchDescriptor = BLEDescriptor("2901", "Motor Switch");
BLEDescriptor stateDescriptor = BLEDescriptor("2901", "Motor State");
BLEDescriptor writeDescriptor = BLEDescriptor("2901", "Activate Sensor");

void setup() {
  Serial.begin(9600);
  pinMode(MOTOR_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT);
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input
  // Wise to set both names incase a device uses one or the other
  blePeripheral.setLocalName("Dope Sensor");
  blePeripheral.setDeviceName("BLE Dope Sensor");

  blePeripheral.setAdvertisedServiceUuid(motorWrite.uuid());
  blePeripheral.addAttribute(motorWrite);
  blePeripheral.addAttribute(writeCharacteristic);
  blePeripheral.addAttribute(writeDescriptor);
  blePeripheral.addAttribute(motorswitch);
  blePeripheral.addAttribute(switchCharacteristic);
  blePeripheral.addAttribute(switchDescriptor);
  blePeripheral.addAttribute(stateCharacteristic);
  blePeripheral.addAttribute(stateDescriptor);

  // Adding event handler, will get called when BLE is written too.
  switchCharacteristic.setEventHandler(BLEWritten, switchCharacteristicWritten);
  writeCharacteristic.setEventHandler(BLEWritten, switchMotorWritten);

  //Begins BLE service
  blePeripheral.begin();
  Serial.println("BLE Dope Sensor");
}

void loop() {
  blePeripheral.poll();

  currentState = digitalRead(BUTTON_PIN);
  delay(10);
  debounceState = digitalRead(BUTTON_PIN);
  if( currentState != switchState ) {
    if( currentState == debounceState ) {

      if (currentState == LOW) {
        // do nothing, button has just been released
      } else {
        Serial.println("Button event: PRESSED ON");
        if ( motorState == 0 ) {
          switchCharacteristic.setValue(1);
          stateCharacteristic.setValue(1);
          digitalWrite(MOTOR_PIN, HIGH);
          // rotates the servo motor from 15 to 165 degrees
          myServo.attach(12);       
          sensorRotate();
          myServo.detach();
          motorState = 1;
        } else {
          switchCharacteristic.setValue(0);
          stateCharacteristic.setValue(0);
          digitalWrite(MOTOR_PIN, LOW);
          motorState = 0;           
        }
      }
      // Notifys the program the BUTTON STATE has been changed
      switchState = currentState;
}}}

void switchMotorWritten(BLECentral& central, BLECharacteristic& characteristic) { //BLE Function 
Serial.print(F("Motor Written Characteristic Event: "));
unlockMotor(characteristic.value(), characteristic.valueLength());
}

void unlockMotor(const unsigned char* code, int codeLength) {
  openTime = millis(); // set even if bad code is used

  //boolean match = false; // Does the code match secret

  if (strlen(oneRot) == codeLength) {
    for (int i = 0; i < codeLength; i++) {
      if (oneRot[i] == code[i]) {
          match = 1;
          break;
      } else if (fiveRot[i] == code[i]) {
          match = 5;
          break;
      } else {
          Serial.println("INCORRECT YOU WILL NOW BE ELIMANTED");
          stateCharacteristic.setValue("Invalid Code");        
      }
    }
  }


  if (match == 1) {
    // turn on motor
    Serial.println("Code matches one rotations, sensor is now active");
    digitalWrite(MOTOR_PIN, HIGH);
    stateCharacteristic.setValue("Valid Code");
    myServo.attach(12);    
    sensorRotate();
    Serial.println("\nRotation has been complete");
    myServo.detach();
    motorState = 1;  
  }  else {
    Serial.println("INCORRECT YOU WILL NOW BE ELIMANTED");
    stateCharacteristic.setValue("Invalid Code");    
  }
  
  if (match == 5) {
    Serial.println("Code matches five rotations, sensor is now active");
    digitalWrite(MOTOR_PIN, HIGH);
    stateCharacteristic.setValue("Valid Code");
    myServo.attach(12);    
    sensorRotate();
    Serial.println("\nRotation has been complete");
    myServo.detach();
    motorState = 1; 
  } else {
    Serial.println("INCORRECT YOU WILL NOW BE ELIMANTED");
    stateCharacteristic.setValue("Invalid Code");    
  }
  
}





void switchCharacteristicWritten(BLECentral& central, BLECharacteristic& characteristic) { //BLE Function 
Serial.print(F("Characteristic Sensor event: "));
if ( switchCharacteristic.value() ) {
    digitalWrite( MOTOR_PIN, HIGH);
    stateCharacteristic.setValue(1);  
    myServo.attach(12);    
    sensorRotate();
    Serial.println("\nRotation has been complete");
    myServo.detach();
    motorState = 1;
} else {
    digitalWrite( MOTOR_PIN, LOW);
    motorState = 0;
    stateCharacteristic.setValue(0);  
}
}
void sensorRotate() { // Rotates the Sensor 150 degrees 
      for(int i=15;i<=165;i++){  
      myServo.write(i);
      delay(30);
      distance = calculateDistance();
      Serial.print(i);
      Serial.print(",");
      Serial.print(distance);
      Serial.print(".");  
        }
      for(int i=165;i>15;i--){  
      myServo.write(i);
      delay(30);
      distance = calculateDistance();
      Serial.print(i);
      Serial.print(",");
      Serial.print(distance);
      Serial.print(".");
        }
      //motorState = 0;
      //currentState = LOW;
    }

int calculateDistance(){   // Calculats the sound when it echos forward and hits an object. (Need to half the results)
  digitalWrite(trigPin, LOW); 
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH); 
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH); // Reads the echoPin, returns the sound wave travel time in microseconds
  distance= duration*0.034/2;
  return distance;
}
