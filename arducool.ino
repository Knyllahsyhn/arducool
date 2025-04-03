#include <Arduino.h>
#include "Timers.h"
#include "Sensor.h"
#include "Pump.h"
#include "Fan.h"

const int sensor1Pin = A0;
const int sensor2Pin = A1;
const int pump1Pin   = 9;
const int pump2Pin   = 10;
const int fanPin     = 3;
const int benchmarkButtonPin = 4;
const int ledPin             = 13;

// Hysteresen
ActuatorHysteresis pump1NormalHyst = {0.0f, 0.0f};
ActuatorHysteresis pump1BenchHyst  = {0.0f, 0.0f};
ActuatorHysteresis pump2NormalHyst = {20.0f, 15.0f};
ActuatorHysteresis pump2BenchHyst = {10.0f,5.0f};

ActuatorCurve pump1NormalCurve = {25.0f, 35.0f, 85, 128};
ActuatorCurve pump1BenchCurve  = {25.0f, 30.0f, 128, 230};
// Sensoren
Sensor sensor1(A0, 10000.0f, 3950.0f, 10000.0f, 298.15f, 0.1f);
Sensor sensor2(A1, 10000.0f, 3950.0f, 10000.0f, 298.15f, 0.1f);

// Pumpe1
Pump pump1(
  sensor1, 9,
  pump1NormalHyst, pump1NormalCurve,
  pump1BenchHyst,  pump1BenchCurve,
  2000,            // Kickstart-Dauer
  255              // Kickstart-PWM
);

// Pumpe1
Pump pump2(
  sensor2, 10,
  pump2NormalHyst, pump1NormalCurve,
  pump2BenchHyst,  pump1BenchCurve,
  2000,            // Kickstart-Dauer
  255              // Kickstart-PWM
);

// Lsüfter, der ab Normal 30..25°C off, Benchmark 20..15°C off, etc.
ActuatorHysteresis fanNormalHyst = {25.0f, 22.0f}; // EIN>30, AUS<25
ActuatorHysteresis fanBenchHyst  = {20.0f, 10.0f}; // ...
ActuatorCurve fanNormalCurve     = {25.0f, 40.0f, 114, 255};
ActuatorCurve fanBenchCurve      = {20.0f, 30.0f, 128, 255};

Fan fan1(sensor2, 3,
         fanNormalHyst, fanNormalCurve,
         fanBenchHyst,  fanBenchCurve);

bool benchmarkMode = false;
static bool lastButtonState = HIGH;
static unsigned long lastDebounceTime = 0;
static const unsigned long debounceDelay = 50;

unsigned long lastControlTime = 0;
const unsigned long controlInterval = 500;

void setup() {
  Serial.begin(9600);
  pinMode(benchmarkButtonPin, INPUT_PULLUP);
  pinMode(ledPin, OUTPUT);

  initTimers25kHz();

  // Alle Kanäle 0
  setPWM_25kHz(pump1Pin, 0);
  setPWM_25kHz(pump2Pin, 0);
  setPWM_25kHz(fanPin,   0);
}

// Taster
void handleBenchmarkButton() {
  bool reading = digitalRead(benchmarkButtonPin);
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (lastButtonState == HIGH && reading == LOW) {
      benchmarkMode = !benchmarkMode;
      digitalWrite(ledPin, benchmarkMode ? HIGH : LOW);
      Serial.print("Benchmark: ");
      Serial.println(benchmarkMode ? "ON" : "OFF");
    }
  }
  lastButtonState = reading;
}

void loop() {
  handleBenchmarkButton();

  unsigned long now = millis();
  if ((now - lastControlTime) >= controlInterval) {
    lastControlTime = now;


  sensor1.update();
  sensor2.update();

  pump1.update(benchmarkMode);
  pump2.update(benchmarkMode);
  fan1.update(benchmarkMode);

    Serial.print("Pump1 PWM=");
  Serial.print(pump1.getCurrentPWM());
  Serial.print("  Pump2 PWM=");
  Serial.print(pump2.getCurrentPWM());
  Serial.print("  Fan1 PWM=");
  Serial.print(fan1.getCurrentPWM());
  Serial.print(" T1 = ");
  Serial.print(sensor1.getTemperature());
   Serial.print(" T2 = ");
  Serial.println(sensor2.getTemperature());
  }
}
