//Arduino Code - RFID Book Selector with Rotary Encoder and LCD

#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// RFID Pins
#define SS_PIN 10
#define RST_PIN 7

// Rotary Encoder Pins
#define CLK_PIN 2
#define DT_PIN 3
#define SW_PIN 4

// RFID Instance
MFRC522 rfid(SS_PIN, RST_PIN);

// LCD Instance (Address 0x27 for 16x2 display, change to 0x3F if needed)
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Define the RFID tags for each book
byte book1[] = {0x00, 0xAD, 0xA9, 0x2B};
byte book2[] = {0x30, 0xE7, 0xA8, 0x2B};
byte book3[] = {0xA0, 0x9D, 0xB5, 0x2B};
byte book4[] = {0x10, 0x2C, 0xAC, 0x2B};
byte book5[] = {0x90, 0xFA, 0xBA, 0x2B};
byte book6[] = {0xC9, 0x7D, 0x6A, 0x05};

// Rotary encoder variables
int currentStateCLK;
int lastStateCLK;
int selectedBook = 1; // Currently selected book (1-6)
int lockedBook = 0;   // Locked book selection (0 = not locked)
bool bookLocked = false;

// Button debounce variables
unsigned long lastButtonPress = 0;
const unsigned long debounceDelay = 200;

void setup() {
  Serial.begin(9600);
  
  // Initialize SPI and RFID
  SPI.begin();
  rfid.PCD_Init();
  
  // Initialize LCD
  lcd.init();
  lcd.backlight();
  
  // Initialize Rotary Encoder
  pinMode(CLK_PIN, INPUT);
  pinMode(DT_PIN, INPUT);
  pinMode(SW_PIN, INPUT_PULLUP);
  
  // Read the initial state of CLK
  lastStateCLK = digitalRead(CLK_PIN);
  
  // Display initial message
  lcd.setCursor(0, 0);
  lcd.print("Select Book:");
  lcd.setCursor(0, 1);
  lcd.print("Book ");
  lcd.print(selectedBook);
  
  Serial.println("RFID Book Scanner Ready!");
  Serial.println("Use rotary encoder to select a book, press button to lock.");
}

void loop() {
  // Check if book is not locked - allow selection
  if (!bookLocked) {
    handleRotaryEncoder();
    handleButtonPress();
  }
  // If book is locked - check for RFID scan
  else {
    checkRFID();
  }
}

void handleRotaryEncoder() {
  // Read the current state of CLK
  currentStateCLK = digitalRead(CLK_PIN);
  
  // If the state of CLK has changed
  if (currentStateCLK != lastStateCLK && currentStateCLK == 1) {
    // If DT state is different from CLK state - rotating clockwise
    if (digitalRead(DT_PIN) != currentStateCLK) {
      selectedBook++;
      if (selectedBook > 6) {
        selectedBook = 1; // Wrap around to 1
      }
    } else {
      // Rotating counter-clockwise
      selectedBook--;
      if (selectedBook < 1) {
        selectedBook = 6; // Wrap around to 6
      }
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
    
    // Lock the book selection
    lockedBook = selectedBook;
    bookLocked = true;
    
    // Update LCD
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Book Selected:");
    lcd.setCursor(0, 1);
    lcd.print("Book ");
    lcd.print(lockedBook);
    
    Serial.print("Book Selected: Book ");
    Serial.println(lockedBook);
    Serial.println("Now scan the book...");
    
    delay(1000);
    
    // Display scanning message
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Scan Book ");
    lcd.print(lockedBook);
    lcd.setCursor(0, 1);
    lcd.print("Now...");
  }
}

void checkRFID() {
  // Check for new card
  if (!rfid.PICC_IsNewCardPresent())
    return;
  
  // Verify if the card can be read
  if (!rfid.PICC_ReadCardSerial())
    return;
  
  Serial.print(F("RFID Tag UID:"));
  printHex(rfid.uid.uidByte, rfid.uid.size);
  Serial.println("");
  
  // Check if scanned book matches the selected book
  bool isCorrectBook = false;
  
  switch (lockedBook) {
    case 1:
      isCorrectBook = compareUID(rfid.uid.uidByte, book1, rfid.uid.size);
      break;
    case 2:
      isCorrectBook = compareUID(rfid.uid.uidByte, book2, rfid.uid.size);
      break;
    case 3:
      isCorrectBook = compareUID(rfid.uid.uidByte, book3, rfid.uid.size);
      break;
    case 4:
      isCorrectBook = compareUID(rfid.uid.uidByte, book4, rfid.uid.size);
      break;
    case 5:
      isCorrectBook = compareUID(rfid.uid.uidByte, book5, rfid.uid.size);
      break;
    case 6:
      isCorrectBook = compareUID(rfid.uid.uidByte, book6, rfid.uid.size);
      break;
  }
  
  // Display result
  lcd.clear();
  if (isCorrectBook) {
    lcd.setCursor(0, 0);
    lcd.print("Result: YES");
    lcd.setCursor(0, 1);
    lcd.print("Correct Book!");
    Serial.println("YES - Correct Book!");
  } else {
    lcd.setCursor(0, 0);
    lcd.print("Result: NO");
    lcd.setCursor(0, 1);
    lcd.print("Wrong Book!");
    Serial.println("NO - Wrong Book!");
  }
  
  delay(3000);
  
  // Reset for next selection
  bookLocked = false;
  lockedBook = 0;
  selectedBook = 1;
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Select Book:");
  lcd.setCursor(0, 1);
  lcd.print("Book ");
  lcd.print(selectedBook);
  
  Serial.println("----------------------------");
  Serial.println("Select next book...");
  
  rfid.PICC_HaltA();
}

void updateLCD() {
  lcd.setCursor(0, 1);
  lcd.print("Book ");
  lcd.print(selectedBook);
  lcd.print("  "); // Clear any extra characters
}

// Function to compare two UID arrays
bool compareUID(byte *buffer1, byte *buffer2, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    if (buffer1[i] != buffer2[i]) {
      return false;
    }
  }
  return true;
}

// Routine to dump a byte array as hex values to Serial
void printHex(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}
