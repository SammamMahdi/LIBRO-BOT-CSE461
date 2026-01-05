// SLAVE ARDUINO - Book Selection Interface
// This Arduino handles the LCD, Rotary Encoder, and book selection
// Sends selected book number to Master Arduino via Serial (TX/RX)

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>
#include <Servo.h>

// Software Serial for communication with Master
// RX on Slave = TX on Master, TX on Slave = RX on Master
#define SLAVE_RX 2  // Connect to Master TX
#define SLAVE_TX 3  // Connect to Master RX

// Rotary Encoder Pins (moved to avoid conflict)
#define CLK_PIN 4
#define DT_PIN 5
#define SW_PIN 6

// Servo Motor Pin
#define SERVO_PIN 9  // PWM pin for SG92R servo

// LCD Instance (Address 0x27 for 16x2 display, change to 0x3F if needed)
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Software Serial Instance
SoftwareSerial masterSerial(SLAVE_RX, SLAVE_TX);

// Servo Instance
Servo bookServo;

// Rotary encoder variables
int currentStateCLK;
int lastStateCLK;
int selectedBook = 1; // Currently selected book (1-6)
int sentBook = 0;     // Last book sent to master
bool bookSent = false;

// Button debounce variables
unsigned long lastButtonPress = 0;
const unsigned long debounceDelay = 200;

// Status tracking
bool waitingForConfirmation = false;
unsigned long confirmationTimeout = 0;
const unsigned long CONFIRMATION_TIMEOUT = 60000; // 60 seconds timeout
bool bookCompleted = false; // Track if book was found and servo completed

void setup() {
  Serial.begin(9600); // For debugging
  masterSerial.begin(9600); // For communication with Master
  
  // Initialize LCD
  lcd.init();
  lcd.backlight();
  
  // Initialize Servo
  bookServo.attach(SERVO_PIN);
  bookServo.write(180); // Set to initial position (180 degrees)
  
  // Initialize Rotary Encoder
  pinMode(CLK_PIN, INPUT_PULLUP);  // Changed to INPUT_PULLUP
  pinMode(DT_PIN, INPUT_PULLUP);   // Changed to INPUT_PULLUP
  pinMode(SW_PIN, INPUT_PULLUP);
  
  // Small delay for pin stabilization
  delay(50);
  
  // Read the initial state of CLK
  lastStateCLK = digitalRead(CLK_PIN);
  
  // Display initial message
  lcd.setCursor(0, 0);
  lcd.print("Select Book:");
  lcd.setCursor(0, 1);
  lcd.print("Book ");
  lcd.print(selectedBook);
  
  Serial.println("Slave Arduino - Book Selector Ready!");
  Serial.println("Use rotary encoder to select a book, press button to confirm.");
}

void loop() {
  // Check for incoming data from Master
  checkMasterResponse();
  
  // Check for manual reset button press (after book completed)
  checkResetButton();
  
  // Debug output - print status every 2 seconds
  static unsigned long lastDebug = 0;
  if (millis() - lastDebug > 2000) {
    Serial.print("Status - bookSent: ");
    Serial.print(bookSent);
    Serial.print(" | waitingForConfirmation: ");
    Serial.print(waitingForConfirmation);
    Serial.print(" | selectedBook: ");
    Serial.println(selectedBook);
    lastDebug = millis();
  }
  
  // If waiting for confirmation from master (after book found)
  if (waitingForConfirmation) {
    if (millis() - confirmationTimeout > CONFIRMATION_TIMEOUT) {
      // Timeout - reset
      resetSelection();
    }
    return; // Don't process encoder/button while waiting
  }
  
  // If book has been sent, wait for it to be processed
  if (bookSent) {
    // Could add a timeout here if needed
    delay(100);
    return;
  }
  
  // Normal operation - handle encoder and button
  handleRotaryEncoder();
  handleButtonPress();
}

void handleRotaryEncoder() {
  // Read the current state of CLK
  currentStateCLK = digitalRead(CLK_PIN);
  
  // Debug: Print raw pin states occasionally
  static unsigned long lastPinDebug = 0;
  if (millis() - lastPinDebug > 500) {
    Serial.print("CLK: ");
    Serial.print(currentStateCLK);
    Serial.print(" | DT: ");
    Serial.print(digitalRead(DT_PIN));
    Serial.print(" | lastCLK: ");
    Serial.println(lastStateCLK);
    lastPinDebug = millis();
  }
  
  // Only detect on rising edge (LOW to HIGH transition) - FIXED!
  if (currentStateCLK != lastStateCLK && currentStateCLK == HIGH) {
    Serial.println(">>> CLK RISING EDGE DETECTED! <<<");
    
    // Small delay for debouncing
    delay(2);
    
    // If DT state is different from CLK state - rotating clockwise
    if (digitalRead(DT_PIN) != currentStateCLK) {
      selectedBook++;
      if (selectedBook > 6) {
        selectedBook = 1; // Wrap around to 1
      }
      Serial.print("Clockwise → ");
    } else {
      // Rotating counter-clockwise
      selectedBook--;
      if (selectedBook < 1) {
        selectedBook = 6; // Wrap around to 6
      }
      Serial.print("Counter-clockwise → ");
    }
    
    // Update LCD
    updateLCD();
    
    Serial.print("Selected Book: ");
    Serial.println(selectedBook);
  }
  
  // Save the last CLK state
  lastStateCLK = currentStateCLK;
}

void handleButtonPress() {
  // Read button state (active LOW with pull-up)
  int buttonState = digitalRead(SW_PIN);
  
  // Check if button is pressed and debounce
  if (buttonState == LOW && (millis() - lastButtonPress > debounceDelay)) {
    lastButtonPress = millis();
    
    // Send book selection to master
    sentBook = selectedBook;
    bookSent = true;
    
    // Update LCD
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Book ");
    lcd.print(sentBook);
    lcd.print(" Selected");
    lcd.setCursor(0, 1);
    lcd.print("Robot searching..");
    
    Serial.print("Sending Book ");
    Serial.print(sentBook);
    Serial.println(" to Master Arduino...");
    
    // Send to Master via Serial
    Serial.println("DEBUG: Sending via SoftwareSerial...");
    masterSerial.print("S");
    masterSerial.println(sentBook); // Format: "S1" to "S6"
    Serial.print("DEBUG: Sent 'S");
    Serial.print(sentBook);
    Serial.println("' to Master");
    
    waitingForConfirmation = true;
    confirmationTimeout = millis();
  }
}

void checkMasterResponse()
{
  if (masterSerial.available() > 0) {
    Serial.println("DEBUG: Data available from Master!");
    String response = masterSerial.readStringUntil('\n');
    response.trim();
    
    Serial.print("DEBUG: Received from Master: '");
    Serial.print(response);
    Serial.println("'");
    
    if (response.startsWith("F")) { // 'F' for Found
      // Format: "F1" to "F6"
      int bookNum = response.substring(1).toInt();
      
      // Display success message
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Book ");
      lcd.print(bookNum);
      lcd.print(" FOUND!");
      lcd.setCursor(0, 1);
      lcd.print("Robot stopped.");
      
      Serial.print("Master confirmed: Book ");
      Serial.print(bookNum);
      Serial.println(" found!");
      
      delay(2000);
      
      // Activate servo - rotate from 180 to 0 and back
      lcd.setCursor(0, 1);
      lcd.print("Servo rotating..");
      Serial.println("Activating servo: rotating to 0 degrees");
      
      bookServo.write(0); // Rotate to 0 degrees
      delay(1000); // Wait for servo to complete movement
      
      Serial.println("Servo: rotating back to 180 degrees");
      bookServo.write(180); // Rotate back to 180 degrees
      delay(1000); // Wait for servo to complete movement
      
      lcd.setCursor(0, 1);
      lcd.print("Servo complete! ");
      Serial.println("Servo operation complete");
      delay(1000);
      
      // Mark as completed - system can now be reset by button press
      bookCompleted = true;
      waitingForConfirmation = false; // Allow button checking
      
      lcd.setCursor(0, 1);
      lcd.print("Press to reset  ");
      
      // Wait for Master to send reset signal
      // (Master will send 'R' after 30 second stop)
    }
    else if (response.startsWith("A")) { // 'A' for Acknowledge
      // Format: "A1" to "A6"
      int bookNum = response.substring(1).toInt();
      
      Serial.print("Master acknowledged Book ");
      Serial.println(bookNum);
      
      // Update LCD to show robot is searching
      lcd.setCursor(0, 1);
      lcd.print("Searching...    ");
    }
    else if (response == "R") { // 'R' for Reset/Ready
      Serial.println("Master ready for next selection");
      resetSelection();
    }
  }
}

void resetSelection() {
  bookSent = false;
  waitingForConfirmation = false;
  bookCompleted = false;
  sentBook = 0;
  selectedBook = 1;
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Select Book:");
  lcd.setCursor(0, 1);
  lcd.print("Book ");
  lcd.print(selectedBook);
  
  Serial.println("----------------------------");
  Serial.println("Ready for next selection...");
}

void checkResetButton() {
  // Only check for reset if book has been completed
  if (!bookCompleted) {
    return;
  }
  
  // Read button state (active LOW with pull-up)
  int buttonState = digitalRead(SW_PIN);
  
  // Check if button is pressed and debounce
  if (buttonState == LOW && (millis() - lastButtonPress > debounceDelay)) {
    lastButtonPress = millis();
    
    Serial.println("Reset button pressed - restarting system");
    
    // Send reset command to Master
    masterSerial.println("R");
    Serial.println("Sent reset command to Master");
    
    // Reset local state
    resetSelection();
  }
}

void updateLCD() {
  lcd.setCursor(0, 1);
  lcd.print("Book ");
  lcd.print(selectedBook);
  lcd.print("  "); // Clear any extra characters
}
