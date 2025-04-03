#pragma once
#include <Arduino.h>
#include "Sensor.h"

/**
 * Kapselt eine Pumpe, die anhand eines Sensors gesteuert wird.
 *  - Hysterese (on/off)
 *  - Kickstart
 *  - Temperatur->PWM-Kurve
 */
class Pump {
public:
  /**
   * Konstruktor:
   *  - sensorRef: Referenz auf den Sensor, der diese Pumpe steuern soll
   *  - pwmPin: Pin (9 oder 10), wo die PWM anliegt
   *  - onThreshold / offThreshold: Hysteresegrenzen
   *  - pwmMinNormal / pwmMaxNormal: z.B. 33%..50% (85..128)
   *  - pwmMaxBenchmark: z.B. 230 (~90%)
   *  - kickstartDuration: wie lange (ms) soll Kickstart anliegen
   *  - kickstartPWM: PWM-Wert während Kickstart (0..255)
   */
  Pump(const Sensor& sensorRef,
       uint8_t pwmPin,
       float onThreshold,
       float offThreshold,
       int pwmMinNormal,
       int pwmMaxNormal,
       int pwmMaxBenchmark,
       unsigned long kickstartDuration,
       int kickstartPWM);

  // In jedem Zyklus aufrufen:
  //  - benchmarkMode: ob wir die steile Kurve verwenden
  void update(bool benchmarkMode);
  int getCurrentPWM() const;

private:
  const Sensor& sensor;
  uint8_t pin;

  float onTemp;
  float offTemp;

  int pwmMin;
  int pwmMax;
  int pwmMaxBench;
  int pwmVal;

  bool isOn;

  // Kickstart
  bool kickstartActive;
  unsigned long kickstartTime;
  int kickstartValue;
  unsigned long kickstartBegin;

  // Hilfsfunktion, um Temperatur linear in PWM (0..255) zu mappen
  int mapTemperature(float temp, float tLow, float tHigh, int pwmLow, int pwmHigh);
};
