#include "Fan.h"
#include "Timers.h"

Fan::Fan(const Sensor& sensorRef, uint8_t pwmPin)
: sensor(sensorRef),
  pin(pwmPin),
  pwmVal(0)
{
}

void Fan::update(bool benchmarkMode) {
  float T = sensor.getTemperature();
  // Einfach 25..40°C => 0..255, egal ob Benchmark oder nicht
  pwmVal = mapTemperature(T, 25.0f, 40.0f, 0, 255);


  setPWM_25kHz(pin, pwmVal);
}

int Fan::mapTemperature(float temp, float tLow, float tHigh, int pwmLow, int pwmHigh) {
  if (temp <= tLow) return pwmLow;
  if (temp >= tHigh) return pwmHigh;
  float ratio = (temp - tLow) / (tHigh - tLow);
  return pwmLow + (int)(ratio * (pwmHigh - pwmLow));
}

int Fan::getCurrentPWM() const {
  return pwmVal;
} 