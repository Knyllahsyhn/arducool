#include <Arduino.h>
#include "Pins.h"
#include "Timers.h"
#include "Sensor.h"
#include "Pump.h"
#include "Fan.h"
#include "DisplayManager.h"

// Hysteresis: on/off temperature for each actuator, normal and benchmark mode.
ActuatorHysteresis pump1NormalHyst = { 0.0f, 0.0f };
ActuatorHysteresis pump1BenchHyst  = { 0.0f, 0.0f };
ActuatorHysteresis pump2NormalHyst = { 20.0f, 15.0f };
ActuatorHysteresis pump2BenchHyst  = { 10.0f, 5.0f };

// Curve: {TLow, THigh, pwmLow, pwmHigh}. PWM range is 0-100 percent.
ActuatorCurve pump1NormalCurve = { 20.0f, 35.0f, 38, 50 };
ActuatorCurve pump1BenchCurve  = { 20.0f, 30.0f, 50, 75 };

// Sensors
Sensor sensor1(PIN_SENSOR1, 10000.0f, 3950.0f, 298.15f, 0.1f);
Sensor sensor2(PIN_SENSOR2, 10000.0f, 3950.0f, 298.15f, 0.1f);

// Pump 1
Pump pump1(
  sensor1, PIN_PUMP1,
  pump1NormalHyst, pump1NormalCurve,
  pump1BenchHyst, pump1BenchCurve,
  2000,  // Kickstart duration, ms
  100    // Kickstart PWM
);

// Pump 2 uses the same curve as pump 1 by design: both pumps must
// run the same PWM profile.
Pump pump2(
  sensor2, PIN_PUMP2,
  pump2NormalHyst, pump1NormalCurve,
  pump2BenchHyst, pump1BenchCurve,
  2000,  // Kickstart duration, ms
  100    // Kickstart PWM
);

// Fan: normal mode off below 25 C, on above; benchmark mode off below 20 C.
ActuatorHysteresis fanNormalHyst = { 25.0f, 22.0f };
ActuatorHysteresis fanBenchHyst  = { 20.0f, 10.0f };
ActuatorCurve fanNormalCurve = { 25.0f, 40.0f, 45, 100 };
ActuatorCurve fanBenchCurve  = { 20.0f, 30.0f, 50, 100 };

Fan fan1(sensor2, PIN_FAN1,
         fanNormalHyst, fanNormalCurve,
         fanBenchHyst, fanBenchCurve);

DisplayManager displayMgr(sensor1, sensor2, pump1, pump2, fan1, 0x3C);

unsigned long lastDisplayTime = 0;
const unsigned long displayInterval = 1000;

bool benchmarkMode = false;
unsigned long lastControlTime = 0;
const unsigned long controlInterval = 500;

// Read the benchmark button with debounce and toggle benchmarkMode
// on a press. The pin uses INPUT_PULLUP, so a press reads LOW.
void handleBenchmarkButton() {
  static int lastReading = HIGH;
  static int stableState = HIGH;
  static unsigned long lastDebounceTime = 0;
  const unsigned long debounceDelay = 50;

  int reading = digitalRead(PIN_BENCHMARK_BUTTON);

  if (reading != lastReading) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay && reading != stableState) {
    stableState = reading;
    if (stableState == LOW) {
      benchmarkMode = !benchmarkMode;
      digitalWrite(PIN_LED, benchmarkMode ? HIGH : LOW);
      Serial.print(F("Benchmark toggled: "));
      Serial.println(benchmarkMode ? F("ON") : F("OFF"));
    }
  }

  lastReading = reading;
}

void setup() {
  Serial.begin(9600);
  pinMode(PIN_PUMP1, OUTPUT);
  pinMode(PIN_PUMP2, OUTPUT);
  pinMode(PIN_FAN1, OUTPUT);
  pinMode(PIN_BENCHMARK_BUTTON, INPUT_PULLUP);
  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, LOW);

  if (!displayMgr.begin()) {
    Serial.println(F("Display not found!"));
  }

  initTimers25kHz();
}

void loop() {
  unsigned long now = millis();
  if ((now - lastControlTime) >= controlInterval) {
    lastControlTime = now;
    handleBenchmarkButton();

    if (now - lastDisplayTime >= displayInterval) {
      lastDisplayTime = now;
      displayMgr.update(benchmarkMode);
    }

    sensor1.update();
    sensor2.update();

    pump1.update(benchmarkMode);
    pump2.update(benchmarkMode);
    fan1.update(benchmarkMode);

    Serial.print(F("S1;"));
    Serial.print(sensor1.getTemperature());
    Serial.print(F("  |S2;"));
    Serial.print(sensor2.getTemperature());
    Serial.print(F("  |P1;"));
    Serial.print(pump1.getCurrentPWM());
    Serial.print(F("  |P2;"));
    Serial.print(pump2.getCurrentPWM());
    Serial.print(F("  |Fan;"));
    Serial.println(fan1.getCurrentPWM());
  }
}
