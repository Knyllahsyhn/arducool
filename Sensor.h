#pragma once
#include <Arduino.h>

// Sensor reads a temperature from an NTC thermistor with the Beta
// formula, and smooths it with an exponential moving average (EMA).
class Sensor {
public:
  // analogPin: analog input pin (for example A0)
  // rFixed: fixed resistor value in the voltage divider, in ohm
  // beta: NTC beta coefficient
  // t0Kelvin: reference temperature T0, in kelvin (25 C = 298.15 K)
  // alpha: EMA smoothing factor, 0 to 1
  Sensor(int analogPin,
         float rFixed,
         float beta,
         float t0Kelvin,
         float alpha = 0.1f);

  // Read the ADC and update the smoothed temperature.
  // On an invalid raw reading, this function keeps the last good
  // value and sets the fault flag instead of computing a new value.
  void update();

  // Return the smoothed temperature in degrees Celsius.
  float getTemperature() const;

  // Return true if the last update() call found an invalid raw reading.
  bool isFaulted() const;

private:
  int pin;
  float R_FIXED;
  float B;
  float T0;
  float alphaFilter;

  bool firstRun;
  bool faulted;
  float filteredTemp;

  // Convert one raw ADC reading (1-1022) to a temperature in Celsius.
  float computeTempFromRaw(int raw) const;
};
