#include "Sensor.h"
#include <math.h>  // log()

Sensor::Sensor(int analogPin,
               float rFixed,
               float beta,
               float t0Kelvin,
               float alpha)
: pin(analogPin),
  R_FIXED(rFixed),
  B(beta),
  T0(t0Kelvin),
  alphaFilter(alpha),
  firstRun(true),
  faulted(false),
  filteredTemp(0.0f)
{
}

void Sensor::update() {
  int raw = analogRead(pin);

  // A raw value of 0 or 1023 means an open or a shorted sensor circuit.
  // Both values make the Beta formula divide by zero or take log(0).
  if (raw <= 0 || raw >= 1023) {
    faulted = true;
    return;  // Keep the last good filteredTemp value.
  }
  faulted = false;

  float rawTemp = computeTempFromRaw(raw);
  if (firstRun) {
    filteredTemp = rawTemp;
    firstRun = false;
  } else {
    // Exponential moving average (EMA)
    filteredTemp = alphaFilter * rawTemp + (1 - alphaFilter) * filteredTemp;
  }
}

float Sensor::getTemperature() const {
  return filteredTemp;
}

bool Sensor::isFaulted() const {
  return faulted;
}

// Beta formula: ADC value -> voltage -> NTC resistance -> kelvin -> Celsius
float Sensor::computeTempFromRaw(int raw) const {
  float rNtc = R_FIXED * (1023.0f / (float)raw - 1.0f);
  float invT = (1.0f / T0) + (1.0f / B) * log(rNtc / R_FIXED);
  float tKelvin = 1.0f / invT;
  return tKelvin - 273.15f;
}
