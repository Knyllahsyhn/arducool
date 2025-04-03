#pragma once
#include <Arduino.h>
#include "Sensor.h"

/**
 * Lüfter-Klasse:
 *  - lineare Kennlinie 25..60°C => 0..255 PWM (Beispiel)
 *  - optional Benchmark-Mode
 */
class Fan {
public:
  Fan(const Sensor& sensorRef, uint8_t pwmPin);

  // update:
  //  - benchmarkMode könnte eine andere Kennlinie schalten,
  //    oder du ignorierst es und fährst immer dieselbe Kurve.
  void update(bool benchmarkMode);
  int getCurrentPWM() const;

private:
  const Sensor& sensor;
  uint8_t pin;
  int pwmVal;

  int mapTemperature(float temp, float tLow, float tHigh, int pwmLow, int pwmHigh);
};
