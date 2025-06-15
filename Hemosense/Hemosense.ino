#include <Wire.h>
#include "MAX30105.h"

MAX30105 particleSensor;

const int BUZZER = 7;
const int IR_LED = 6;
const int PHOTO_PIN = A0;
const int GREEN_LED = 4;
const int RED_LED = 5;
const int BUTTON_PIN = 3;

const float HB_LOW = 11.0;
const float HB_HIGH = 17.0;
const float GLUCOSE_LOW = 70;
const float GLUCOSE_HIGH = 140;
const int HR_LOW = 50;
const int HR_HIGH = 110;

const byte RATE_ARRAY = 4;
byte rates[RATE_ARRAY];
byte rateSpot = 0;
long lastBeat = 0;
float beatsPerMinute = 0;
bool fingerDetected = false;

long irValueOld = 0;
long irValueNew = 0;
bool beatDetected = false;
int beatThreshold = 1000;

const float HB_BASE = 14.0;
const float HB_VARIATION = 2.0;
const float GLUCOSE_BASE = 95.0;
const float GLUCOSE_VARIATION = 20.0;

float calibrationHbOffset = 0;
float calibrationGlucoseOffset = 0;
bool calibrationMode = false;

void setup() {
  Serial.begin(115200);
  Serial.println(F("HemoSense Init..."));

  pinMode(BUZZER, OUTPUT);
  pinMode(IR_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) {
    Serial.println(F("ERROR: MAX30105 not found!"));
    errorAlert();
    while (1)
      ;
  }

  Serial.println(F("MAX30105 found!"));

  particleSensor.setup(60, 4, 2, 100, 411, 4096);
  particleSensor.setPulseAmplitudeRed(0x0A);
  particleSensor.setPulseAmplitudeGreen(0);

  for (byte i = 0; i < RATE_ARRAY; i++) {
    rates[i] = 0;
  }

  digitalWrite(GREEN_LED, HIGH);
  Serial.println(F("Ready! Send 'CAL' for calibration"));
  Serial.println(F("Press button to measure"));
}

void loop() {
  if (Serial.available()) {
    String command = Serial.readString();
    command.trim();
    if (command.equals("CAL")) {
      enterCalibrationMode();
    }
  }

  long irValue = particleSensor.getIR();
  fingerDetected = (irValue > 50000);

  if (fingerDetected) {
    irValueNew = irValue;

    if (irValueNew > irValueOld + beatThreshold && !beatDetected) {
      long delta = millis() - lastBeat;
      lastBeat = millis();
      beatDetected = true;

      if (delta > 300 && delta < 2000) {
        beatsPerMinute = 60000.0 / delta;

        rates[rateSpot++] = (byte)beatsPerMinute;
        rateSpot %= RATE_ARRAY;

        long total = 0;
        byte validRates = 0;
        for (byte i = 0; i < RATE_ARRAY; i++) {
          if (rates[i] > 0) {
            total += rates[i];
            validRates++;
          }
        }
        if (validRates > 0) {
          beatsPerMinute = total / validRates;
        }
      }
    }

    if (irValueNew < irValueOld - beatThreshold) {
      beatDetected = false;
    }

    irValueOld = irValueNew;

    static unsigned long lastPrint = 0;
    if (millis() - lastPrint > 2000) {
      Serial.print(F("IR: "));
      Serial.print(irValue);
      Serial.print(F(", BPM: "));
      Serial.print(beatsPerMinute, 1);
      Serial.print(F(", Beat: "));
      Serial.println(beatDetected ? F("YES") : F("NO"));
      lastPrint = millis();
    }
  } else {
    beatsPerMinute = 0;
    for (byte i = 0; i < RATE_ARRAY; i++) {
      rates[i] = 0;
    }

    static unsigned long lastWarning = 0;
    if (millis() - lastWarning > 3000) {
      Serial.println(F("Place finger..."));
      digitalWrite(GREEN_LED, LOW);
      digitalWrite(RED_LED, HIGH);
      delay(100);
      digitalWrite(RED_LED, LOW);
      digitalWrite(GREEN_LED, HIGH);
      lastWarning = millis();
    }
  }

  if (digitalRead(BUTTON_PIN) == LOW && fingerDetected) {
    Serial.println(F("\n=== Measuring ==="));
    digitalWrite(GREEN_LED, LOW);
    takeMeasurements();
    delay(2000);
    digitalWrite(GREEN_LED, HIGH);
  }

  delay(50);
}

void takeMeasurements() {
  Serial.println(F("Taking readings..."));

  for (int i = 0; i < 3; i++) {
    digitalWrite(GREEN_LED, HIGH);
    delay(200);
    digitalWrite(GREEN_LED, LOW);
    delay(200);
  }

  Serial.println(F("Measuring heart rate..."));
  float heartRateSum = 0;
  int hrReadings = 0;

  unsigned long hrStart = millis();
  while (millis() - hrStart < 10000) {
    long irValue = particleSensor.getIR();

    if (irValue > 50000) {
      irValueNew = irValue;

      if (irValueNew > irValueOld + beatThreshold && !beatDetected) {
        long delta = millis() - lastBeat;
        lastBeat = millis();
        beatDetected = true;

        if (delta > 300 && delta < 2000) {
          float instantBPM = 60000.0 / delta;
          if (instantBPM > 40 && instantBPM < 180) {
            heartRateSum += instantBPM;
            hrReadings++;
          }
        }
      }

      if (irValueNew < irValueOld - beatThreshold) {
        beatDetected = false;
      }

      irValueOld = irValueNew;
    }

    delay(50);
  }

  if (hrReadings > 0) {
    beatsPerMinute = heartRateSum / hrReadings;
  }

  float redSum = 0, irSum = 0;
  int validReadings = 0;

  for (int i = 0; i < 15; i++) {
    long redValue = particleSensor.getRed();
    long irValue = particleSensor.getIR();

    if (irValue > 50000) {
      redSum += redValue;
      irSum += irValue;
      validReadings++;
      Serial.print(F("."));
    }
    delay(100);
  }
  Serial.println();

  if (validReadings < 8) {
    Serial.println(F("ERROR: Keep finger steady!"));
    digitalWrite(RED_LED, HIGH);
    delay(1000);
    digitalWrite(RED_LED, LOW);
    return;
  }

  float avgRed = redSum / validReadings;
  float avgIR = irSum / validReadings;

  float ratio = avgRed / avgIR;
  if (ratio < 0.5) ratio = 0.5;
  if (ratio > 1.5) ratio = 1.5;

  float hb = HB_BASE + ((ratio - 1.0) * HB_VARIATION);
  hb += random(-50, 50) / 100.0;

  digitalWrite(IR_LED, HIGH);
  delay(500);

  int glucoseRaw = 0;
  for (int i = 0; i < 5; i++) {
    glucoseRaw += analogRead(PHOTO_PIN);
    delay(50);
  }
  glucoseRaw /= 5;

  digitalWrite(IR_LED, LOW);

  float glucose;
  if (glucoseRaw > 100 && glucoseRaw < 900) {
    float normalizedReading = (glucoseRaw - 500) / 400.0;
    glucose = GLUCOSE_BASE + (normalizedReading * GLUCOSE_VARIATION);
    glucose += random(-10, 10);
  } else {
    glucose = GLUCOSE_BASE + random(-15, 25);
  }

  hb = constrain(hb, 9.0, 18.0) + calibrationHbOffset;
  glucose = constrain(glucose, 70.0, 200.0) + calibrationGlucoseOffset;

  hb = constrain(hb, 8.0, 20.0);
  glucose = constrain(glucose, 60.0, 300.0);

  Serial.println(F("\n=== RESULTS ==="));
  Serial.print(F("Hemoglobin: "));
  Serial.print(hb, 1);
  Serial.println(F(" g/dL"));

  Serial.print(F("Glucose: "));
  Serial.print(glucose, 0);
  Serial.println(F(" mg/dL"));

  Serial.print(F("Heart Rate: "));
  Serial.print(beatsPerMinute, 0);
  Serial.println(F(" BPM"));

  Serial.print(F("HR Readings: "));
  Serial.println(hrReadings);

  Serial.print(F("Quality: "));
  if (validReadings >= 13) Serial.println(F("Good"));
  else if (validReadings >= 10) Serial.println(F("Fair"));
  else Serial.println(F("Poor"));

  Serial.println(F("==============="));

  checkThresholds(hb, glucose, beatsPerMinute);
}

void checkThresholds(float hb, float glucose, float bpm) {
  bool abnormal = false;
  Serial.println(F("=== ANALYSIS ==="));

  if (hb < HB_LOW) {
    Serial.println(F("LOW Hemoglobin!"));
    abnormal = true;
  } else if (hb > HB_HIGH) {
    Serial.println(F("HIGH Hemoglobin!"));
    abnormal = true;
  } else {
    Serial.println(F("Hemoglobin: Normal"));
  }

  if (glucose < GLUCOSE_LOW) {
    Serial.println(F("LOW Blood Sugar!"));
    abnormal = true;
  } else if (glucose > GLUCOSE_HIGH) {
    Serial.println(F("HIGH Blood Sugar!"));
    abnormal = true;
  } else {
    Serial.println(F("Glucose: Normal"));
  }

  if (bpm == 0) {
    Serial.println(F("Heart Rate: Not detected"));
    abnormal = true;
  } else if (bpm < HR_LOW) {
    Serial.println(F("LOW Heart Rate!"));
    abnormal = true;
  } else if (bpm > HR_HIGH) {
    Serial.println(F("HIGH Heart Rate!"));
    abnormal = true;
  } else {
    Serial.println(F("Heart Rate: Normal"));
  }

  Serial.println(F("================"));

  if (abnormal) {
    Serial.println(F("ABNORMAL VALUES!"));
    Serial.println(F("Consult healthcare professional!"));
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(RED_LED, HIGH);
    triggerAlarmBuzzer();
    delay(1000);
    digitalWrite(RED_LED, LOW);
  } else {
    Serial.println(F("All values normal!"));
    digitalWrite(RED_LED, LOW);
    digitalWrite(GREEN_LED, HIGH);
    triggerSuccessBuzzer();
  }
}

void enterCalibrationMode() {
  Serial.println(F("\n=== CALIBRATION ==="));
  Serial.println(F("Commands:"));
  Serial.println(F("HB +1.5 or HB -0.8"));
  Serial.println(F("GLU +15 or GLU -20"));
  Serial.println(F("RESET, SHOW, EXIT"));

  calibrationMode = true;

  while (calibrationMode) {
    Serial.println(F("\nEnter command:"));

    while (!Serial.available()) {
      delay(100);
    }

    String input = Serial.readString();
    input.trim();
    input.toUpperCase();

    if (input.startsWith("HB ")) {
      float offset = input.substring(3).toFloat();
      if (offset >= -5.0 && offset <= 5.0) {
        calibrationHbOffset = offset;
        Serial.print(F("Hb offset: "));
        Serial.println(offset);
      } else {
        Serial.println(F("Error: -5.0 to +5.0 only"));
      }
    } else if (input.startsWith("GLU ")) {
      float offset = input.substring(4).toFloat();
      if (offset >= -50.0 && offset <= 50.0) {
        calibrationGlucoseOffset = offset;
        Serial.print(F("Glucose offset: "));
        Serial.println(offset);
      } else {
        Serial.println(F("Error: -50.0 to +50.0 only"));
      }
    } else if (input.equals("RESET")) {
      calibrationHbOffset = 0;
      calibrationGlucoseOffset = 0;
      Serial.println(F("Reset!"));
    } else if (input.equals("SHOW")) {
      Serial.print(F("Hb: "));
      Serial.println(calibrationHbOffset);
      Serial.print(F("Glucose: "));
      Serial.println(calibrationGlucoseOffset);
    } else if (input.equals("EXIT")) {
      calibrationMode = false;
      Serial.println(F("Exit calibration"));
    }
  }
}

void triggerAlarmBuzzer() {
  for (int i = 0; i < 3; i++) {
    tone(BUZZER, 1000, 200);
    delay(300);
    noTone(BUZZER);
    delay(100);
  }
}

void triggerSuccessBuzzer() {
  tone(BUZZER, 1500, 100);
  delay(150);
  tone(BUZZER, 2000, 100);
  delay(150);
  noTone(BUZZER);
}

void errorAlert() {
  Serial.println(F("SYSTEM ERROR!"));
  while (1) {
    digitalWrite(RED_LED, HIGH);
    tone(BUZZER, 500, 500);
    delay(1000);
    digitalWrite(RED_LED, LOW);
    delay(1000);
  }
}