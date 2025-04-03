#include "Sensor.h"
#include <math.h>  // log()

Sensor::Sensor(int analogPin,
               float rFixed,
               float beta,
               float r0,
               float t0Kelvin,
               float alpha)
: pin(analogPin),
  R_FIXED(rFixed),
  B(beta),
  R0(r0),
  T0(t0Kelvin),
  alphaFilter(alpha),
  firstRun(true),
  filteredTemp(0.0f)
{
}

void Sensor::update() {
  float rawTemp = readNTCTemp();
  if (firstRun) {
    filteredTemp = rawTemp;
    firstRun = false;
  } else {
    // Exponential Moving Average
    filteredTemp = alphaFilter * rawTemp + (1 - alphaFilter) * filteredTemp;
  }
}

float Sensor::getTemperature() const {
  return filteredTemp;
}

// Beta-Formel: ADC -> Spannung -> R_NTC -> Kelvin -> °C
float Sensor::readNTCTemp() {
  int raw = analogRead(pin);

  const float VCC = 5.0f;
  float vPin = (raw / 1023.0f) * VCC;
  if (vPin <= 0.0f || vPin >= (VCC - 0.0001f)) {
    // Fehler, unplausible Messung
    return -100.0f;
  }

  float rNtc = R_FIXED * (vPin / (VCC - vPin));
  float invT = (1.0f / T0) + (1.0f / B) * log(rNtc / R0);
  float tKelvin = 1.0f / invT;
  return (tKelvin - 273.15f);
}
