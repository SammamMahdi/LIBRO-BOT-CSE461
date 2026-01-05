# Pin Configuration for Master-Slave Arduino System (SERIAL UART)

## MASTER ARDUINO (Line Follower & Motor Control)
**Responsibilities:** Line following, motor control, RFID detection on shelves

### Pin Assignments:
- **A0** - IR Sensor Right (Line following)
- **A1** - IR Sensor Left (Line following)
- **D2** - RFID RST Pin
- **D3** - Serial RX (from Slave TX) - SoftwareSerial
- **D4** - RFID SS Pin (Chip Select)
- **D5** - Left Motor Enable (PWM)
- **D6** - Right Motor Enable (PWM)
- **D7** - Right Motor Pin 1
- **D8** - Right Motor Pin 2
- **D9** - Left Motor Pin 1
- **D10** - Left Motor Pin 2
- **D11** - Serial TX (to Slave RX) - SoftwareSerial
- **D12** - SPI MISO (for RFID)
- **D13** - SPI SCK (for RFID)
- **GND** - Common Ground (connect to Slave)

### Serial Communication:
- **Hardware Serial (D0/D1)** - USB programming & debugging
- **Software Serial (D3/D11)** - Communication with Slave
- Baud Rate: 9600

### Power Requirements:
- Motors require external power supply (motor driver)
- Arduino can be powered via USB or Vin (7-12V)

---

## SLAVE ARDUINO (Book Selection Interface)
**Responsibilities:** LCD display, rotary encoder, user interface

### Pin Assignments:
- **D2** - Serial RX (from Master TX) - SoftwareSerial
- **D3** - Serial TX (to Master RX) - SoftwareSerial
- **D4** - Rotary Encoder CLK
- **D5** - Rotary Encoder DT
- **D6** - Rotary Encoder SW (Button)
- **D9** - Servo Motor Signal (SG92R)
- **A4** - I2C SDA (LCD only)
- **A5** - I2C SCL (LCD only)
- **GND** - Common Ground (connect to Master)

### Serial Communication:
- **Hardware Serial (D0/D1)** - USB programming & debugging
- **Software Serial (D2/D3)** - Communication with Master
- Baud Rate: 9600

### I2C Addresses:
- LCD Display: 0x27 (or 0x3F - check your LCD module)

### Power Requirements:
- Can be powered via USB or Vin (7-12V)
- LCD powered through I2C module (5V from Arduino)
- Servo (SG92R) powered from 5V pin (ensure adequate power supply)

---

## SERIAL UART Communication Protocol

### Commands from Slave to Master:
- **"S1" to "S6"** - Select book (Book 1 to Book 6)
- **"R"** - Reset selection

### Commands from Master to Slave:
- **"F1" to "F6"** - Book found confirmation (Book 1 to Book 6)
- **"A1" to "A6"** - Acknowledge book selection received
- **"R"** - Reset/Ready for next selection

### Message Format:
- All messages are newline-terminated strings
- Command letter followed by book number (if applicable)
- Examples: "S3\n", "F2\n", "A1\n", "R\n"

---

## Hardware Connections

### Master Arduino Connections:
1. **IR Sensors** → A0, A1 (with appropriate resistors)
2. **Motor Driver** → D5, D6 (enable), D7, D8, D9, D10 (direction)
3. **RFID Module** → D2 (RST), D4 (SS), D12 (MISO), D13 (SCK), 3.3V (VCC)
4. **Serial to Slave**:
   - D3 (Master RX) → D3 (Slave TX)
   - D11 (Master TX) → D2 (Slave RX)
   - GND → GND (MUST CONNECT!)

### Slave Arduino Connections:
1. **Rotary Encoder** → D4 (CLK), D5 (DT), D6 (SW), GND, 5V
2. **LCD (I2C)** → A4 (SDA), A5 (SCL), GND, 5V
3. **Servo Motor (SG92R)** → D9 (Signal/Orange wire), 5V (Red wire), GND (Brown wire)
4. **Serial to Master**:
   - D2 (Slave RX) → D11 (Master TX)
   - D3 (Slave TX) → D3 (Master RX)
   - GND → GND (MUST CONNECT!)

### Important Notes:
- **CRITICAL:** Connect GND between both Arduinos
- Cross-connect Serial lines: Master RX → Slave TX, Master TX → Slave RX
- SoftwareSerial on Master uses D3 (RX) and D11 (TX)
- SoftwareSerial on Slave uses D2 (RX) and D3 (TX)
- Both use 9600 baud rate
- No additional resistors needed for UART (unlike I2C)

---

## Advantages of Serial UART over I2C

✅ **Simpler wiring** - Only 3 wires (TX, RX, GND) vs I2C (SDA, SCL, GND)
✅ **No pull-up resistors needed** - UART doesn't require external resistors
✅ **No address conflicts** - Direct point-to-point communication
✅ **Better for longer distances** - UART is more robust than I2C
✅ **Easier debugging** - Can monitor with Serial Monitor
✅ **Frees up I2C** - Master's I2C (A4/A5) is now available for expansion

---

## System Operation Flow

1. **User selects book** on Slave Arduino using rotary encoder
2. **User presses button** to confirm selection
3. **Slave sends "S3"** (for example) to Master via Serial
4. **Master receives and acknowledges** with "A3"
5. **Master starts line following** and searches for Book 3
6. **Master detects RFID tag** and checks if it matches Book 3
7. **If match:** Master stops for 10 seconds and sends "F3" to Slave
8. **Slave displays "Book 3 FOUND!"** on LCD
9. **Servo activates:** Rotates from 180° → 0° → 180° (takes ~2 seconds)
10. **After 10 seconds:** Master resumes line following automatically
11. **Robot continues** following the line indefinitely
12. **Manual reset:** Press rotary encoder button to restart system and select new book

---

## Troubleshooting

### Serial Communication Issues:
- Verify baud rate is 9600 on both sides
- Check TX/RX crossover: Master RX connects to Slave TX and vice versa
- Ensure common ground connection
- Don't connect anything to D0/D1 (Hardware Serial for USB)
- Test with Serial Monitor (open both Master and Slave monitors)

### No Communication Between Arduinos:
- Check if SoftwareSerial is properly initialized
- Verify pin connections (D3↔D3 and D11↔D2)
- Use Serial.print() to debug what's being sent/received
- Make sure GND is connected!

### LCD Not Displaying:
- Try alternate address 0x3F instead of 0x27
- Check I2C connections (SDA, SCL, VCC, GND)
- Adjust contrast potentiometer on I2C backpack

### RFID Not Detecting:
- Verify RFID module is 3.3V or 5V compatible
- Check SPI connections (note: MOSI removed for Serial TX)
- Ensure proper power supply to RFID module
- Test RFID UIDs match the defined arrays

### Motors Not Running Properly:
- Verify motor driver has adequate power supply
- Check PWM frequency setting (TCCR0B register)
- Test motor connections and polarity
- Ensure enable pins receive PWM signal

---

## Libraries Required

### Master Arduino:
- SPI (built-in)
- MFRC522 (install via Library Manager)
- SoftwareSerial (built-in)

### Slave Arduino:
- Wire (built-in)
- LiquidCrystal_I2C (install via Library Manager)
- SoftwareSerial (built-in)
- Servo (built-in)

---

## Testing Serial Communication

### Test 1: Slave to Master
Upload both sketches, open Serial Monitor for Master at 9600 baud:
- Select a book on Slave
- Press button
- You should see: "Received book selection from Slave: Book X"

### Test 2: Master to Slave
With both running:
- Select a book and press button
- When Master finds the book, Slave LCD should show "Book X FOUND!"

### Test 3: Debugging
Add this to Master's `checkSlaveCommand()`:
```cpp
Serial.print("Received: ");
Serial.println(command);
```

Add this to Slave's `checkMasterResponse()`:
```cpp
Serial.print("Received: ");
Serial.println(response);
```

---

## Book RFID Tags Reference

All 6 books are defined with their RFID UIDs:
- Book 1: 0x00, 0xAD, 0xA9, 0x2B
- Book 2: 0x30, 0xE7, 0xA8, 0x2B
- Book 3: 0xA0, 0x9D, 0xB5, 0x2B
- Book 4: 0x10, 0x2C, 0xAC, 0x2B
- Book 5: 0x90, 0xFA, 0xBA, 0x2B
- Book 6: 0xC9, 0x7D, 0x6A, 0x05

Make sure your physical RFID tags match these UIDs, or update the arrays accordingly.

---

## Pin Summary Comparison

### I2C Version vs Serial Version:

**MASTER:**
- I2C: Uses A4/A5 for communication
- Serial: Uses D3/D11 for communication (frees up A4/A5)

**SLAVE:**
- I2C: Encoder on D2/D3/D4
- Serial: Encoder on D4/D5/D6 (D2/D3 used for Serial)

**Winner:** Serial version is cleaner and more reliable for this application!
