// Includes the Servo library
#include <Servo.h>
#include <SPI.h>
#include <BLEPeripheral.h>

// Define variables for motor and button
#define MOTOR_PIN 12
#define BUTTON_PIN 7
#define BLE_REQ     11 // 
#define BLE_RDY     2 // 
#define BLE_RST     10 // 

// Defines Tirg and Echo pins of the Ultrasonic Sensor
const int trigPin = 8;
const int echoPin = 9;
// Variables for the duration and the distance
long duration;
int distance;
int val;
int currentState;
int debounceState;
int switchState = 0;   
int ledState = 0;

// create peripheral instance
BLEPeripheral blePeripheral = BLEPeripheral(BLE_REQ, BLE_RDY, BLE_RST);

// create service
BLEService lightswitch = BLEService("FF10");

// create switch characteristic
BLECharCharacteristic switchCharacteristic = BLECharCharacteristic("FF11", BLERead | BLEWrite);
BLEDescriptor switchDescriptor = BLEDescriptor("2901", "Switch");

BLECharCharacteristic stateCharacteristic = BLECharCharacteristic("FF12", BLENotify);
BLEDescriptor stateDescriptor = BLEDescriptor("2901", "State");

Servo myServo; // Creates a servo object for controlling the servo motor

void setup() {
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input
  pinMode(MOTOR_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT);
  Serial.begin(9600);
  myServo.attach(12); // Defines on which pin is the servo motor attached  
  // set advertised local name and service UUID
  blePeripheral.setLocalName("That Lead Tho");  // Advertised in scan data as part of GAP
  blePeripheral.setDeviceName("That Lead Tho"); // Advertised in generic access as part of GATT
  blePeripheral.setAdvertisedServiceUuid(lightswitch.uuid());

  // add service and characteristics
  blePeripheral.addAttribute(lightswitch);
  
  blePeripheral.addAttribute(switchCharacteristic);
  blePeripheral.addAttribute(switchDescriptor);
  
  blePeripheral.addAttribute(stateCharacteristic);
  blePeripheral.addAttribute(stateDescriptor);

  // assign event handler for characteristic
  switchCharacteristic.setEventHandler(BLEWritten, switchCharacteristicWritten);
  
  // begin initialization
  blePeripheral.begin();

  Serial.println(F("Smart Light Switch"));

}
void loop() {

  blePeripheral.poll();
  Serial.begin(9600);
 
  currentState = digitalRead(BUTTON_PIN);
  delay(10);
  debounceState = digitalRead(BUTTON_PIN);
  if( currentState == debounceState  ) { 
    if ( currentState != switchState ) { 

      if ( currentState == LOW ) { 
        // Button just released
        
      } else { 
        Serial.print(F("Button event: "));
        if ( ledState == 0 ) {
          stateCharacteristic.setValue(1);
          switchCharacteristic.setValue(1);
          digitalWrite(MOTOR_PIN, HIGH);
          ledState = 1;

          myServo.attach(12);
          Serial.println(F("light on"));
          for(int i=15;i<=165;i++){  
          myServo.write(i);
          delay(30);
          distance = calculateDistance(); }//
          for(int i=165;i>15;i--){  
          myServo.write(i);
          delay(30);
          distance = calculateDistance();

          }
          myServo.detach();
          Serial.println("Motor finished Rotation");
       } else {
          stateCharacteristic.setValue(0);
          switchCharacteristic.setValue(0);
          digitalWrite(MOTOR_PIN, LOW);
          ledState = 0;
           Serial.println(F("light off"));
           myServo.detach();
       }
      }
      switchState = currentState;
    }
  }
}

// Function for calculating the distance measured by the Ultrasonic sensor
int calculateDistance(){ 
  
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
void switchCharacteristicWritten(BLECentral& central, BLECharacteristic& characteristic) {
  Serial.begin(9600);

  Serial.print(F("Characteristic event: "));
  if (switchCharacteristic.value()) {
    Serial.println(F("light on"));
    digitalWrite(MOTOR_PIN, HIGH);
    ledState = 1;
    stateCharacteristic.setValue(1);  
    
  } else {
    Serial.println(F("light off"));
    digitalWrite(MOTOR_PIN, LOW);
    ledState = 0;
    stateCharacteristic.setValue(0);   
    
  }
}
