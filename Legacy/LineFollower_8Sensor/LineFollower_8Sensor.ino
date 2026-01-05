#include <SPI.h>
#include <MFRC522.h>

// IR Sensors (moved to analog pins to avoid SPI conflict)
#define IR_SENSOR_RIGHT A0
#define IR_SENSOR_LEFT A1
#define MOTOR_SPEED 150

// RFID Pins
#define SS_PIN 4
#define RST_PIN 2

// RFID Instance
MFRC522 rfid(SS_PIN, RST_PIN);

//Right motor
int enableRightMotor=6;
int rightMotorPin1=7;
int rightMotorPin2=8;

//Left motor
int enableLeftMotor=5;
int leftMotorPin1=9;
int leftMotorPin2=10;

// RFID tag detection variables
unsigned long stopStartTime = 0;
bool isStopped = false;
const unsigned long stopDuration = 30000; // Stop for 30 seconds

// Authorized RFID tag
byte book1[] = {0x00, 0xAD, 0xA9, 0x2B};

void setup()
{
  //The problem with TT gear motors is that, at very low pwm value it does not even rotate.
  //If we increase the PWM value then it rotates faster and our robot is not controlled in that speed and goes out of line.
  //For that we need to increase the frequency of analogWrite.
  //Below line is important to change the frequency of PWM signal on pin D5 and D6
  //Because of this, motor runs in controlled manner (lower speed) at high PWM value.
  //This sets frequecny as 7812.5 hz.
  TCCR0B = TCCR0B & B11111000 | B00000010 ;
  
  // Initialize SPI and RFID
  SPI.begin();
  rfid.PCD_Init();
  
  // put your setup code here, to run once:
  pinMode(enableRightMotor, OUTPUT);
  pinMode(rightMotorPin1, OUTPUT);
  pinMode(rightMotorPin2, OUTPUT);
  
  pinMode(enableLeftMotor, OUTPUT);
  pinMode(leftMotorPin1, OUTPUT);
  pinMode(leftMotorPin2, OUTPUT);

  pinMode(IR_SENSOR_RIGHT, INPUT);
  pinMode(IR_SENSOR_LEFT, INPUT);
  rotateMotor(0,0);
}


void loop()
{
  // Check for RFID tag (non-blocking)
  checkRFIDTag();
  
  // If stopped due to RFID detection, wait for duration
  if (isStopped) {
    if (millis() - stopStartTime < stopDuration) {
      rotateMotor(0, 0); // Keep motors stopped
      return; // Skip line following logic
    } else {
      isStopped = false; // Resume movement
    }
  }

  // Read sensors immediately with no delay
  int rightIRSensorValue = digitalRead(IR_SENSOR_RIGHT);
  int leftIRSensorValue = digitalRead(IR_SENSOR_LEFT);

  // Immediate motor response based on sensor readings - NO DELAYS
  if (rightIRSensorValue == LOW && leftIRSensorValue == LOW)
  {
    // Both sensors on black - go straight
    rotateMotor(MOTOR_SPEED, MOTOR_SPEED);
  }
  else if (rightIRSensorValue == LOW && leftIRSensorValue == HIGH )
  {
    // Right sensor on black, left on white - turn right
    rotateMotor(-MOTOR_SPEED, MOTOR_SPEED); 
  }
  else if (rightIRSensorValue == HIGH && leftIRSensorValue == LOW)
  {
    // Left sensor on black, right on white - turn left
    rotateMotor(MOTOR_SPEED, -MOTOR_SPEED); 
  } 
  else 
  {
    // Both sensors on white - stop
    rotateMotor(0, 0);
  }
  
  // NO DELAY - immediate loop restart for fastest response
}

void checkRFIDTag()
{
  // Check for new card
  if (!rfid.PICC_IsNewCardPresent())
    return;
  
  // Verify if the card can be read
  if (!rfid.PICC_ReadCardSerial()) {
    return; // Silent fail for speed
  }
  
  // Check if detected tag matches book1
  bool isBook1 = true;
  if (rfid.uid.size == 4) {
    for (byte i = 0; i < 4; i++) {
      if (rfid.uid.uidByte[i] != book1[i]) {
        isBook1 = false;
        break;
      }
    }
  } else {
    isBook1 = false;
  }
  
  // Only stop if it's book1
  if (isBook1) {
    rotateMotor(0, 0);
    isStopped = true;
    stopStartTime = millis();
  }
  
  rfid.PICC_HaltA(); // Halt the card
}


void rotateMotor(int rightMotorSpeed, int leftMotorSpeed)
{
  
  if (rightMotorSpeed < 0)
  {
    digitalWrite(rightMotorPin1,LOW);
    digitalWrite(rightMotorPin2,HIGH);    
  }
  else if (rightMotorSpeed > 0)
  {
    digitalWrite(rightMotorPin1,HIGH);
    digitalWrite(rightMotorPin2,LOW);      
  }
  else
  {
    digitalWrite(rightMotorPin1,LOW);
    digitalWrite(rightMotorPin2,LOW);      
  }

  if (leftMotorSpeed < 0)
  {
    digitalWrite(leftMotorPin1,LOW);
    digitalWrite(leftMotorPin2,HIGH);    
  }
  else if (leftMotorSpeed > 0)
  {
    digitalWrite(leftMotorPin1,HIGH);
    digitalWrite(leftMotorPin2,LOW);      
  }
  else 
  {
    digitalWrite(leftMotorPin1,LOW);
    digitalWrite(leftMotorPin2,LOW);      
  }
  analogWrite(enableRightMotor, abs(rightMotorSpeed));
  analogWrite(enableLeftMotor, abs(leftMotorSpeed));    
}