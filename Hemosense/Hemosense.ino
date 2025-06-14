#include <Wire.h>
#include "MAX30105.h"

MAX30105 particleSensor;

// Pins
const int BUZZER = 7;
const int IR_LED = 6;
const int PHOTO_PIN = A0;
const int GREEN_LED = 4;
const int RED_LED = 5;
const int BUTTON_PIN = 3;

// Thresholds stored in PROGMEM to save RAM
const float HB_LOW = 11.0;
const float HB_HIGH = 17.0;
const float GLUCOSE_LOW = 70;
const float GLUCOSE_HIGH = 140;
const int HR_LOW = 50;
const int HR_HIGH = 110;

// Reduced heart rate array size
const byte RATE_ARRAY = 4;
byte rates[RATE_ARRAY];
byte rateSpot = 0;
long lastBeat = 0;
float beatsPerMinute = 0;
bool fingerDetected = false;

// Heart rate detection variables
long irValueOld = 0;
long irValueNew = 0;
bool beatDetected = false;
int beatThreshold = 1000;  // Minimum change for beat detection

// Calibration constants
const float HB_BASE = 14.0;
const float HB_VARIATION = 2.0;
const float GLUCOSE_BASE = 95.0;
const float GLUCOSE_VARIATION = 20.0;

// Calibration offsets
float calibrationHbOffset = 0;
float calibrationGlucoseOffset = 0;
bool calibrationMode = false;

void setup() {
  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:

}
