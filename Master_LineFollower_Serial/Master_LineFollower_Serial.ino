// MASTER ARDUINO - Line Follower with RFID Detection
// This Arduino controls the motors and line following
// Receives selected book number from Slave Arduino via Serial (RX/TX)

#include <SPI.h>
#include <MFRC522.h>
#include <SoftwareSerial.h>

// IR Sensors (analog pins to avoid SPI conflict)
#define IR_SENSOR_RIGHT A0
#define IR_SENSOR_LEFT A1
#define MOTOR_SPEED 150

// RFID Pins (for detecting books on the shelf)
#define SS_PIN 4
#define RST_PIN 2

// Software Serial for communication with Slave
// RX on Master = TX on Slave, TX on Master = RX on Slave
#define MASTER_RX 3  // Connect to Slave TX
#define MASTER_TX A2 // Connect to Slave RX (moved from D11 to free MOSI!)

// RFID Instance
MFRC522 rfid(SS_PIN, RST_PIN);

// Software Serial Instance
SoftwareSerial slaveSerial(MASTER_RX, MASTER_TX);

// Right motor
int enableRightMotor = 6;
int rightMotorPin1 = 7;
int rightMotorPin2 = 8;

// Left motor
int enableLeftMotor = 5;
int leftMotorPin1 = 9;
int leftMotorPin2 = 10;

// RFID tag detection variables
unsigned long stopStartTime = 0;
bool isStopped = false;
const unsigned long stopDuration = 10000; // Stop for 30 seconds

// Book selection from slave
int selectedBook = 0; // 0 = no book selected, 1-6 = book number
bool bookFoundAndProcessed = false; // Track if book was already found in this session

// All book RFID tags (same as in book_select.ino)
byte book1[] = {0x00, 0xAD, 0xA9, 0x2B};
byte book2[] = {0x30, 0xE7, 0xA8, 0x2B};
byte book3[] = {0xA0, 0x9D, 0xB5, 0x2B};
byte book4[] = {0x10, 0x2C, 0xAC, 0x2B};
byte book5[] = {0x90, 0xFA, 0xBA, 0x2B};
byte book6[] = {0xC9, 0x7D, 0x6A, 0x05};

void setup()
{
  Serial.begin(9600); // For debugging
  slaveSerial.begin(9600); // For communication with Slave
  
  // The problem with TT gear motors is that, at very low pwm value it does not even rotate.
  // If we increase the PWM value then it rotates faster and our robot is not controlled in that speed and goes out of line.
  // For that we need to increase the frequency of analogWrite.
  // Below line is important to change the frequency of PWM signal on pin D5 and D6
  // Because of this, motor runs in controlled manner (lower speed) at high PWM value.
  // This sets frequency as 7812.5 hz.
  TCCR0B = TCCR0B & B11111000 | B00000010;
  
  // Initialize SPI and RFID
  SPI.begin();
  rfid.PCD_Init();
  
  // Motor pins setup
  pinMode(enableRightMotor, OUTPUT);
  pinMode(rightMotorPin1, OUTPUT);
  pinMode(rightMotorPin2, OUTPUT);
  
  pinMode(enableLeftMotor, OUTPUT);
  pinMode(leftMotorPin1, OUTPUT);
  pinMode(leftMotorPin2, OUTPUT);

  pinMode(IR_SENSOR_RIGHT, INPUT);
  pinMode(IR_SENSOR_LEFT, INPUT);
  
  rotateMotor(0, 0);
  
  Serial.println("Master Arduino - Line Follower Ready");
  Serial.println("Waiting for book selection from Slave via Serial...");
}


void loop()
{
  // Check for incoming data from Slave
  checkSlaveCommand();
  
  // Only start line following if a book has been selected
  if (selectedBook == 0) {
    // No book selected, keep motors stopped
    rotateMotor(0, 0);
    delay(100);
    return;
  }
  
  // Check for RFID tag (non-blocking)
  checkRFIDTag();
  
  // If stopped due to RFID detection, wait for duration
  if (isStopped) {
    if (millis() - stopStartTime < stopDuration) {
      rotateMotor(0, 0); // Keep motors stopped
      return; // Skip line following logic
    } else {
      isStopped = false; // Resume movement - keep selectedBook active!
      Serial.println("Stop duration complete. Continuing line following...");
      
      // Do NOT send reset or change selectedBook
      // Robot continues following the line indefinitely
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
  else if (rightIRSensorValue == LOW && leftIRSensorValue == HIGH)
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

void checkSlaveCommand()
{
  if (slaveSerial.available() > 0) {
    Serial.println("DEBUG: Data available from Slave!");
    String command = slaveSerial.readStringUntil('\n');
    command.trim();
    
    Serial.print("DEBUG: Received from Slave: '");
    Serial.print(command);
    Serial.println("'");
    
    if (command.startsWith("S")) { // 'S' for Select
      // Format: "S1" to "S6"
      int bookNum = command.substring(1).toInt();
      if (bookNum >= 1 && bookNum <= 6) {
        selectedBook = bookNum;
        bookFoundAndProcessed = false; // Reset flag for new search
        Serial.print("Received book selection from Slave: Book ");
        Serial.println(selectedBook);
        Serial.println("Starting line follower...");
        
        // Send acknowledgment to Slave
        Serial.println("DEBUG: Sending ACK to Slave...");
        slaveSerial.print("A");
        slaveSerial.println(selectedBook); // 'A' for Acknowledge
        Serial.print("DEBUG: Sent 'A");
        Serial.print(selectedBook);
        Serial.println("' to Slave");
      } else {
        Serial.print("ERROR: Invalid book number: ");
        Serial.println(bookNum);
      }
    }
    else if (command == "R") { // 'R' for Reset from Slave
      selectedBook = 0;
      isStopped = false;
      bookFoundAndProcessed = false;
      Serial.println("Selection reset by Slave");
    } else {
      Serial.print("ERROR: Unknown command: ");
      Serial.println(command);
    }
  }
}

void checkRFIDTag()
{
  // Skip RFID checking if book already found in this session
  if (bookFoundAndProcessed) {
    return;
  }
  
  // Check for new card
  if (!rfid.PICC_IsNewCardPresent())
    return;
  
  // Verify if the card can be read
  if (!rfid.PICC_ReadCardSerial()) {
    return; // Silent fail for speed
  }
  
  // Check if detected tag matches the selected book
  bool isSelectedBook = false;
  byte* selectedBookUID = nullptr;
  
  switch (selectedBook) {
    case 1:
      selectedBookUID = book1;
      break;
    case 2:
      selectedBookUID = book2;
      break;
    case 3:
      selectedBookUID = book3;
      break;
    case 4:
      selectedBookUID = book4;
      break;
    case 5:
      selectedBookUID = book5;
      break;
    case 6:
      selectedBookUID = book6;
      break;
  }
  
  // Compare UID
  if (selectedBookUID != nullptr && rfid.uid.size == 4) {
    isSelectedBook = true;
    for (byte i = 0; i < 4; i++) {
      if (rfid.uid.uidByte[i] != selectedBookUID[i]) {
        isSelectedBook = false;
        break;
      }
    }
  }
  
  // Only stop if it's the selected book AND not already processed
  if (isSelectedBook && !bookFoundAndProcessed) {
    rotateMotor(0, 0);
    isStopped = true;
    bookFoundAndProcessed = true; // Mark as processed
    stopStartTime = millis();
    
    Serial.print("FOUND! Book ");
    Serial.print(selectedBook);
    Serial.println(" detected. Stopping for 10 seconds...");
    
    // Send confirmation to slave Arduino
    slaveSerial.print("F");
    slaveSerial.println(selectedBook); // 'F' for Found
  }
  
  rfid.PICC_HaltA(); // Halt the card
}

void rotateMotor(int rightMotorSpeed, int leftMotorSpeed)
{
  if (rightMotorSpeed < 0)
  {
    digitalWrite(rightMotorPin1, LOW);
    digitalWrite(rightMotorPin2, HIGH);    
  }
  else if (rightMotorSpeed > 0)
  {
    digitalWrite(rightMotorPin1, HIGH);
    digitalWrite(rightMotorPin2, LOW);      
  }
  else
  {
    digitalWrite(rightMotorPin1, LOW);
    digitalWrite(rightMotorPin2, LOW);      
  }

  if (leftMotorSpeed < 0)
  {
    digitalWrite(leftMotorPin1, LOW);
    digitalWrite(leftMotorPin2, HIGH);    
  }
  else if (leftMotorSpeed > 0)
  {
    digitalWrite(leftMotorPin1, HIGH);
    digitalWrite(leftMotorPin2, LOW);      
  }
  else 
  {
    digitalWrite(leftMotorPin1, LOW);
    digitalWrite(leftMotorPin2, LOW);      
  }
  analogWrite(enableRightMotor, abs(rightMotorSpeed));
  analogWrite(enableLeftMotor, abs(leftMotorSpeed));    
}
