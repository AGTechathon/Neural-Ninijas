# Hemosense Arduino - Required Libraries & Components

## 📚 Required Libraries

### Core Libraries
1. *Wire.h* - Built-in Arduino library for I2C communication
2. *MAX30105.h* - SparkFun MAX3010x library for heart rate and pulse oximetry sensor

### Library Installation

#### Method 1: Arduino IDE Library Manager
1. Open Arduino IDE
2. Go to *Sketch* → *Include Library* → *Manage Libraries*
3. Search for "MAX3010x" or "SparkFun MAX30105"
4. Install *"SparkFun MAX3010x Particle Sensor Library"*

#### Method 2: Manual Installation
bash
# Download from GitHub
git clone https://github.com/sparkfun/SparkFun_MAX3010x_Sensor_Library.git

Then add to Arduino/libraries folder

#### Method 3: ZIP Install
- Download: https://github.com/sparkfun/SparkFun_MAX3010x_Sensor_Library/archive/master.zip
- Arduino IDE → *Sketch* → *Include Library* → *Add .ZIP Library*

## 🔧 Hardware Components

### Main Sensor
- *MAX30105* - Heart rate, pulse oximetry, and particle sensor
  - I2C communication
  - 3.3V or 5V compatible
  - Built-in LEDs (Red, IR, Green)

### Additional Components
- *Buzzer* (Pin 7) - Audio feedback
- *IR LED* (Pin 6) - Additional infrared LED
- *Photoresistor/LDR* (Pin A0) - Light sensor
- *Green LED* (Pin 4) - Status indicator
- *Red LED* (Pin 5) - Alert indicator  
- *Push Button* (Pin 3) - User input

### Resistors & Connections
- *10kΩ resistor* - Button pullup (if not using internal)
- *220Ω resistors* - LED current limiting
- *10kΩ resistor* - Photoresistor voltage divider
- *Breadboard* and *jumper wires*

## 🔌 Wiring Diagram


Arduino Uno/Nano ↔ MAX30105
---------------------------
5V/3.3V         ↔ VIN
GND             ↔ GND
A4 (SDA)        ↔ SDA
A5 (SCL)        ↔ SCL

Other Connections:
-----------------
Pin 7  ↔ Buzzer (+)
Pin 6  ↔ IR LED (+) → 220Ω → GND
Pin 5  ↔ Red LED (+) → 220Ω → GND  
Pin 4  ↔ Green LED (+) → 220Ω → GND
Pin 3  ↔ Button → 10kΩ → GND
Pin A0 ↔ Photoresistor → 10kΩ → GND


## 📦 Shopping List

| Component | Quantity | Purpose |
|-----------|----------|---------|
| Arduino Uno/Nano | 1 | Microcontroller |
| MAX30105 Sensor | 1 | Heart rate detection |
| LEDs (Red, Green) | 2 | Status indicators |
| IR LED | 1 | Additional sensing |
| Buzzer | 1 | Audio alerts |
| Push Button | 1 | User input |
| Photoresistor | 1 | Light sensing |
| 220Ω Resistors | 3 | LED protection |
| 10kΩ Resistors | 2 | Pullup/voltage divider |
| Breadboard | 1 | Prototyping |
| Jumper Wires | 1 set | Connections |

## 🛠 Quick Setup Code

cpp
// Test if libraries are working
#include <Wire.h>
#include "MAX30105.h"

MAX30105 particleSensor;

void setup() {
  Serial.begin(115200);
  
  if (!particleSensor.begin()) {
    Serial.println("MAX30105 not found!");
    while(1);
  }
  
  Serial.println("Libraries working!");
  particleSensor.setup(); // Basic setup
}

void loop() {
  long irValue = particleSensor.getIR();
  Serial.print("IR Value: ");
  Serial.println(irValue);
  delay(1000);
}


## 🚨 Common Issues & Solutions

### Library Not Found
- Make sure MAX30105 library is installed
- Restart Arduino IDE after installation
- Check library folder: Arduino/libraries/

### Sensor Not Detected
- Check I2C connections (SDA/SCL)
- Verify power connections (VIN/GND)
- Try different I2C address scanning

### Poor Signal Quality
- Ensure good finger contact with sensor
- Adjust LED power settings
- Check ambient light conditions

## 📋 Library Dependencies

The MAX30105 library requires:
- *Wire.h* (built-in)
- *Arduino.h* (built-in)

No additional dependencies needed!

## 🔗 Useful Links

- [SparkFun MAX30105 Library](https://github.com/sparkfun/SparkFun_MAX3010x_Sensor_Library)
- [MAX30105 Datasheet](https://datasheets.maximintegrated.com/en/ds/MAX30105.pdf)
- [Arduino Wire Library](https://www.arduino.cc/en/reference/wire)
- [Sensor Hookup Guide](https://learn.sparkfun.com/tutorials/max30105-particle-and-pulse-ox-sensor-hookup-guide)