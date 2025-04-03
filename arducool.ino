#include <Arduino.h>
#include "Timers.h"
#include "Sensor.h"
#include "Pump.h"
#include "Fan.h"
#include "DisplayManager.h"


const byte benchmarkButtonPin = 4;
const byte ledPin = 13;

// Hysteresen
ActuatorHysteresis pump1NormalHyst = { 0.0f, 0.0f };
ActuatorHysteresis pump1BenchHyst = { 0.0f, 0.0f };
ActuatorHysteresis pump2NormalHyst = { 20.0f, 15.0f };
ActuatorHysteresis pump2BenchHyst = { 10.0f, 5.0f };

ActuatorCurve pump1NormalCurve = { 25.0f, 35.0f, 85, 128 };
ActuatorCurve pump1BenchCurve = { 25.0f, 30.0f, 128, 230 };
// Sensoren
Sensor sensor1(A0, 10000.0f, 3950.0f, 10000.0f, 298.15f, 0.1f);
Sensor sensor2(A1, 10000.0f, 3950.0f, 10000.0f, 298.15f, 0.1f);

// Pumpe1
Pump pump1(
  sensor1, 9,
  pump1NormalHyst, pump1NormalCurve,
  pump1BenchHyst, pump1BenchCurve,
  2000,  // Kickstart-Dauer
  255    // Kickstart-PWM
);

// Pumpe1
Pump pump2(
  sensor2, 10,
  pump2NormalHyst, pump1NormalCurve,
  pump2BenchHyst, pump1BenchCurve,
  2000,  // Kickstart-Dauer
  255    // Kickstart-PWM
);

// Lsüfter, der ab Normal 30..25°C off, Benchmark 20..15°C off, etc.
ActuatorHysteresis fanNormalHyst = { 25.0f, 22.0f };  // EIN>30, AUS<25
ActuatorHysteresis fanBenchHyst = { 20.0f, 10.0f };   // ...
ActuatorCurve fanNormalCurve = { 25.0f, 40.0f, 114, 255 };
ActuatorCurve fanBenchCurve = { 20.0f, 30.0f, 128, 255 };

Fan fan1(sensor2, 3,
         fanNormalHyst, fanNormalCurve,
         fanBenchHyst, fanBenchCurve);

// DisplayManager anlegen, mit Verweisen auf Sensoren/Pumpen/Fan
DisplayManager displayMgr(sensor1, sensor2, pump1, pump2, fan1, 0x3D);

unsigned long lastDisplayTime = 0;
const unsigned long displayInterval = 1000;

bool benchmarkMode = false;
unsigned long lastControlTime = 0;
const unsigned long controlInterval = 500;

void setup() {
  Serial.begin(9600);
  pinMode(benchmarkButtonPin, INPUT_PULLUP);
  pinMode(ledPin, OUTPUT);
    if(!displayMgr.begin()) {
    Serial.println(F("Display not found!"));
  }

  initTimers25kHz();

  // Alle Kanäle 0
  pump1.setOff();
  pump2.setOff();
  fan1.setOff();
}



void loop() {
  
  unsigned long now = millis();
  if ((now - lastControlTime) >= controlInterval) {
    lastControlTime = now;
    handleBenchmarkButton();
    if(now - lastDisplayTime >= displayInterval) {
    lastDisplayTime = now;
    displayMgr.update(); // Schreibt Sensor/Pump/Fan-Werte aufs OLED
  }


    sensor1.update();
    sensor2.update();

    pump1.update(benchmarkMode);
    pump2.update(benchmarkMode);
    fan1.update(benchmarkMode);

    Serial.print(F("Pump1 PWM="));
    Serial.print(pump1.getCurrentPWM());
    Serial.print(F("  Pump2 PWM="));
    Serial.print(pump2.getCurrentPWM());
    Serial.print(F("  Fan1 PWM="));
    Serial.print(fan1.getCurrentPWM());
    Serial.print(F(" T1 = "));
    Serial.print(sensor1.getTemperature());
    Serial.print(" BM= ");
    Serial.print(benchmarkMode);
    Serial.print(F(" T2 = "));
    Serial.println(sensor2.getTemperature());
  }
}

// Taster
void handleBenchmarkButton() {
  static int lastButtonState = HIGH;
  static unsigned long lastDebounceTime = 0;
  const unsigned long debounceDelay = 50;
  static int buttonState; // Adjust as needed

  // Read the current state; with INPUT_PULLUP the unpressed state is HIGH.
  int reading = digitalRead(benchmarkButtonPin);

  // If the button state has changed, reset the debounce timer.
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }

  // If the state has been stable for longer than debounceDelay,
  // then check if we have a falling edge (HIGH -> LOW).
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState){
      buttonState=reading;
      if (buttonState == LOW){
      benchmarkMode = !benchmarkMode;
      Serial.print("Benchmark toggled: ");
      Serial.println(benchmarkMode ? "ON" : "OFF");
    }
    }
  }

  lastButtonState = reading;
}

