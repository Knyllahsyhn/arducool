#include <Arduino.h>
#include "Timers.h"
#include "Sensor.h"
#include "Pump.h"
#include "Fan.h"

// Pins
const int sensor1Pin = A0;
const int sensor2Pin = A1;
const int pump1Pin   = 9;
const int pump2Pin   = 10;
const int fanPin     = 3;

const int benchmarkButtonPin = 4;
const int ledPin             = 13;

// Erzeuge zwei Sensor-Objekte (NTC-Beta)
Sensor sensor1(sensor1Pin, 10000.0f, 3950.0f, 10000.0f, 298.15f, 0.1f);
Sensor sensor2(sensor2Pin, 10000.0f, 3950.0f, 10000.0f, 298.15f, 0.1f);

// Pumpen anlegen
// Pump1: EIN>25°C, AUS<20°C, Kickstart=1.5s
Pump pump1(sensor1,
           pump1Pin,
           0.0f,   // onThreshold
           0.0f,   // offThreshold
           85,      // pwmMinNormal
           128,     // pwmMaxNormal
           230,     // pwmMaxBenchmark
           2000,    // kickstartDuration
           255);    // kickstartPWM

// Pump2: EIN>20°C, AUS<15°C, Kickstart=2s
Pump pump2(sensor2,
           pump2Pin,
           20.0f,
           15.0f,
           85,
           128,
           230,
           2000,
           255);

// Lüfter, einfach 25..60°C => 0..255
Fan fan(sensor2, fanPin);

// Benchmark-Mode
static bool benchmarkMode = false;

// Debounce
static bool lastButtonState = HIGH;
static unsigned long lastDebounceTime = 0;
static const unsigned long debounceDelay = 50;

unsigned long lastControlTime = 0;
const unsigned long controlInterval = 500;

void setup() {
  Serial.begin(9600);

  pinMode(benchmarkButtonPin, INPUT_PULLUP);
  pinMode(ledPin, OUTPUT);

  // 25 kHz PWM
  initTimers25kHz();
  // Alle Kanäle 0
  setPWM_25kHz(pump1Pin, 0);
  setPWM_25kHz(pump2Pin, 0);
  setPWM_25kHz(fanPin,   0);
}

// Taster, um Benchmark zu togglen
void handleBenchmarkButton() {
  bool reading = digitalRead(benchmarkButtonPin);
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (lastButtonState == HIGH && reading == LOW) {
      benchmarkMode = !benchmarkMode;
      digitalWrite(ledPin, benchmarkMode ? HIGH : LOW);
      Serial.print("Benchmarkmode: ");
      Serial.println(benchmarkMode ? "ON" : "OFF");
    }
  }
  lastButtonState = reading;
}

void loop() {
  // 1) Taster checken
  handleBenchmarkButton();

  // 2) Zeitgesteuerter Regelzyklus
  unsigned long now = millis();
  if (now - lastControlTime >= controlInterval) {
    lastControlTime = now;
    // Sensoren updaten
    sensor1.update();
    sensor2.update();

    // Debug
    Serial.print("T1 = ");
    Serial.print(sensor1.getTemperature());
    Serial.print(" | T2 = ");
    Serial.print(sensor2.getTemperature());
    Serial.print(" | BM = ");
    Serial.println(benchmarkMode);

    // Pumpen & Lüfter updaten
    pump1.update(benchmarkMode);
    pump2.update(benchmarkMode);
    fan.update(benchmarkMode);
  }

  // Hier weitere nicht-blockierende Aktionen, falls nötig ...
}
