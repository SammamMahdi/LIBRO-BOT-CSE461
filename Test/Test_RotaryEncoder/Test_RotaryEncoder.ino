// ROTARY ENCODER TEST SKETCH
// Upload this to your Slave Arduino to diagnose the rotary encoder issue

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Rotary Encoder Pins
#define CLK_PIN 4
#define DT_PIN 5
#define SW_PIN 6

// LCD Instance (try 0x27 first, if not working try 0x3F)
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Rotary encoder variables
int currentStateCLK;
int lastStateCLK;
int counter = 0;
String rotation = "";

void setup() {
  Serial.begin(9600);
  Serial.println("===== ROTARY ENCODER TEST =====");
  Serial.println("Rotate the encoder and watch the output...");
  Serial.println();
  
  // Initialize LCD
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Encoder Test");
  
  // Initialize Rotary Encoder pins
  pinMode(CLK_PIN, INPUT);
  pinMode(DT_PIN, INPUT);
  pinMode(SW_PIN, INPUT_PULLUP);
  
  // Read the initial state of CLK
  lastStateCLK = digitalRead(CLK_PIN);
  
  Serial.print("Initial CLK State: ");
  Serial.println(lastStateCLK);
  Serial.print("Initial DT State: ");
  Serial.println(digitalRead(DT_PIN));
  Serial.print("Initial SW State: ");
  Serial.println(digitalRead(SW_PIN));
  Serial.println("----------------------------");
}

void loop() {
  // Read the current state of CLK
  currentStateCLK = digitalRead(CLK_PIN);
  
  // If the state of CLK has changed, then rotation occurred
  if (currentStateCLK != lastStateCLK) {
    
    // If DT state is different from CLK state - rotating clockwise
    if (digitalRead(DT_PIN) != currentStateCLK) {
      counter++;
      rotation = "CW (Clockwise)";
      Serial.print("Rotation: CLOCKWISE   | Counter: ");
    } else {
      // Rotating counter-clockwise
      counter--;
      rotation = "CCW (Counter-CW)";
      Serial.print("Rotation: COUNTER-CW  | Counter: ");
    }
    
    Serial.print(counter);
    Serial.print(" | CLK: ");
    Serial.print(currentStateCLK);
    Serial.print(" | DT: ");
    Serial.println(digitalRead(DT_PIN));
    
    // Update LCD
    lcd.setCursor(0, 1);
    lcd.print("                "); // Clear line
    lcd.setCursor(0, 1);
    lcd.print(rotation);
    lcd.print(" ");
    lcd.print(counter);
  }
  
  // Save the last CLK state
  lastStateCLK = currentStateCLK;
  
  // Check button press
  if (digitalRead(SW_PIN) == LOW) {
    Serial.println("*** BUTTON PRESSED ***");
    lcd.setCursor(0, 0);
    lcd.print("BUTTON PRESSED! ");
    delay(500);
    lcd.setCursor(0, 0);
    lcd.print("Encoder Test    ");
  }
  
  delay(1); // Small delay for stability
}
