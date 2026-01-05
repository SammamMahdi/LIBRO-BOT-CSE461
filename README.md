# Library Robot Project - Folder Structure

All Arduino sketches are now properly organized in individual folders as required by Arduino IDE.

## 📁 Project Structure

```
librobot/
│
├── 📂 book_select/
│   └── book_select.ino                    [Original RFID book selector with rotary encoder]
│
├── 📂 LineFollower_8Sensor/
│   └── LineFollower_8Sensor.ino           [Your original line follower with RFID]
│
├── 📂 Master_LineFollower/
│   └── Master_LineFollower.ino            [I2C Version - Master Arduino]
│
├── 📂 Slave_BookSelector/
│   └── Slave_BookSelector.ino             [I2C Version - Slave Arduino]
│
├── 📂 Master_LineFollower_Serial/
│   └── Master_LineFollower_Serial.ino     [Serial UART Version - Master Arduino] ⭐ RECOMMENDED
│
├── 📂 Slave_BookSelector_Serial/
│   └── Slave_BookSelector_Serial.ino      [Serial UART Version - Slave Arduino] ⭐ RECOMMENDED
│
├── 📂 Test_RotaryEncoder/
│   └── Test_RotaryEncoder.ino             [Diagnostic test for rotary encoder]
│
└── 📄 Documentation Files
    ├── PIN_CONFIGURATION.md                [Pin assignments for Serial version]
    ├── PIN_CONFIGURATION_SERIAL.md         [Detailed Serial UART pin config]
    ├── PIN_CHANGES_COMPARISON.txt          [What changed from I2C to Serial]
    ├── WIRING_DIAGRAM.txt                  [I2C version wiring]
    ├── WIRING_DIAGRAM_SERIAL.txt           [Serial version wiring] ⭐ USE THIS
    └── ROTARY_ENCODER_TROUBLESHOOTING.md   [Rotary encoder help guide]
```

---

## 🎯 Which Sketches to Use?

### ⭐ **RECOMMENDED: Serial UART Version**

**For Master Arduino (Line Follower Robot):**
```
Master_LineFollower_Serial/Master_LineFollower_Serial.ino
```

**For Slave Arduino (Book Selector Interface):**
```
Slave_BookSelector_Serial/Slave_BookSelector_Serial.ino
```

**Why Serial Version?**
- ✅ Simpler wiring (3 wires: TX, RX, GND)
- ✅ More reliable communication
- ✅ No pull-up resistors needed
- ✅ Easier to debug
- ✅ Frees up I2C pins on Master for expansion
- ✅ **Already has the rotary encoder fix applied!**

---

## 📝 Upload Instructions

### Step 1: Master Arduino
1. Open Arduino IDE
2. File → Open → Navigate to:
   ```
   librobot/Master_LineFollower_Serial/Master_LineFollower_Serial.ino
   ```
3. Select your Master Arduino board and port
4. Click Upload ⬆️

### Step 2: Slave Arduino
1. Open Arduino IDE (can open second window)
2. File → Open → Navigate to:
   ```
   librobot/Slave_BookSelector_Serial/Slave_BookSelector_Serial.ino
   ```
3. Select your Slave Arduino board and port
4. Click Upload ⬆️

---

## 🔌 Wiring Reference

### Master Arduino (Line Follower):
- **A0, A1** - IR Sensors (unchanged from your original)
- **D2** - RFID RST
- **D3** - Serial RX (from Slave)
- **D4** - RFID SS
- **D5-D10** - Motors (unchanged from your original)
- **D11** - Serial TX (to Slave)
- **D12, D13** - RFID MISO, SCK

### Slave Arduino (Book Selector):
- **D2** - Serial RX (from Master)
- **D3** - Serial TX (to Master)
- **D4** - Rotary Encoder CLK
- **D5** - Rotary Encoder DT
- **D6** - Rotary Encoder SW (button)
- **D9** - Servo Motor Signal (SG92R)
- **A4, A5** - LCD I2C (SDA, SCL)

### Connection Between Arduinos:
```
Master D3 (RX)  ──────► Slave D3 (TX)
Master D11 (TX) ──────► Slave D2 (RX)
Master GND      ──────► Slave GND  [CRITICAL!]
```

---

## 📚 Alternative Versions

### I2C Version (Original master-slave design)
- **Master_LineFollower/** - I2C Master
- **Slave_BookSelector/** - I2C Slave
- Uses A4/A5 for communication
- Requires pull-up resistors on I2C lines

### Original Single Arduino
- **LineFollower_8Sensor/** - Your original implementation
- Single Arduino, no book selection interface
- Stops at Book 1 only

### Standalone Book Scanner
- **book_select/** - The original book selection code
- Tests RFID scanning with rotary encoder
- Good for testing RFID tags

---

## 🧪 Testing & Diagnostics

### Test Rotary Encoder
If your rotary encoder isn't working:
```
Test_RotaryEncoder/Test_RotaryEncoder.ino
```
Upload to Slave Arduino and open Serial Monitor to diagnose.

### Test Serial Communication
1. Upload both Serial sketches
2. Open two Serial Monitors (one for each Arduino)
3. Rotate encoder on Slave
4. Press button
5. Watch for messages on both monitors

---

## 🔧 Libraries Required

### For Both Arduinos:
- **SoftwareSerial** (built-in)

### For Master Arduino:
- **SPI** (built-in)
- **MFRC522** (install from Library Manager)

### For Slave Arduino:
- **Wire** (built-in)
- **LiquidCrystal_I2C** (install from Library Manager)
- **Servo** (built-in)

---

## 📖 Book RFID Tags

All sketches use the same 6 book UIDs:
```cpp
Book 1: {0x00, 0xAD, 0xA9, 0x2B}
Book 2: {0x30, 0xE7, 0xA8, 0x2B}
Book 3: {0xA0, 0x9D, 0xB5, 0x2B}
Book 4: {0x10, 0x2C, 0xAC, 0x2B}
Book 5: {0x90, 0xFA, 0xBA, 0x2B}
Book 6: {0xC9, 0x7D, 0x6A, 0x05}
```

Update these arrays if your physical RFID tags have different UIDs.

---

## ✅ Current Status

- ✅ All sketches organized in proper folders
- ✅ Serial UART version implemented and tested
- ✅ Rotary encoder fix applied to Slave_BookSelector_Serial
- ✅ Pin configurations documented
- ✅ Wiring diagrams created
- ✅ Troubleshooting guides included

---

## 🚀 Quick Start Guide

1. **Wire up both Arduinos** following WIRING_DIAGRAM_SERIAL.txt
2. **Install required libraries** (MFRC522, LiquidCrystal_I2C)
3. **Upload Master_LineFollower_Serial.ino** to Master Arduino
4. **Upload Slave_BookSelector_Serial.ino** to Slave Arduino
5. **Test:**
   - Rotate encoder → LCD should show Book 1-6
   - Press button → LCD shows "Robot searching..."
   - Place robot on line → Should start following
   - Present correct book RFID → Robot stops for 10 seconds
   - LCD shows "Book X FOUND!" → Servo rotates 180°→0°→180°
   - Robot resumes line following automatically
   - Press encoder button to reset and select new book

---

## 📞 Need Help?

Check these files:
- **PIN_CONFIGURATION.md** - Pin assignments
- **WIRING_DIAGRAM_SERIAL.txt** - Complete wiring guide
- **ROTARY_ENCODER_TROUBLESHOOTING.md** - Encoder issues
- **PIN_CHANGES_COMPARISON.txt** - What changed from original

---

## 🎆 Current Features

### ✅ Master Arduino (Line Follower):
- 2-sensor line following with adjustable speed
- RFID book detection with 6 book support
- Stops for 10 seconds when correct book is found
- Continues line following after book detection
- Serial communication with Slave for commands
- Prevents duplicate detections of same book

### ✅ Slave Arduino (User Interface):
- 16x2 LCD display with I2C
- Rotary encoder book selection (1-6)
- Button confirmation for selection
- **SG92R Servo motor** for book retrieval mechanism
- Servo action: 180° → 0° → 180° when book found
- Manual reset via encoder button press
- Real-time status updates on LCD

### ✅ System Workflow:
1. Select book (1-6) using rotary encoder
2. Press button to start search
3. Robot follows line to find book
4. Stops when correct RFID detected
5. Servo activates to retrieve/mark book
6. Robot continues following line
7. Press button to reset for new search

---

**Project Status:** ✅ Ready to Deploy
**Recommended Version:** Serial UART (Master_LineFollower_Serial + Slave_BookSelector_Serial)
**Last Updated:** January 5, 2026
