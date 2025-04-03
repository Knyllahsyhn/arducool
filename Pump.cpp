#include "Pump.h"
#include "Timers.h"  // setPWM_25kHz() verwenden
#include <math.h>     // log() falls gebraucht

Pump::Pump(const Sensor& sensorRef,
           uint8_t pwmPin,
           float onThreshold,
           float offThreshold,
           int pwmMinNormal,
           int pwmMaxNormal,
           int pwmMaxBenchmark,
           unsigned long kickstartDuration,
           int kickstartPWM)
: sensor(sensorRef),
  pin(pwmPin),
  onTemp(onThreshold),
  offTemp(offThreshold),
  pwmMin(pwmMinNormal),
  pwmMax(pwmMaxNormal),
  pwmMaxBench(pwmMaxBenchmark),
  isOn(false),
  kickstartActive(false),
  kickstartTime(kickstartDuration),
  kickstartValue(kickstartPWM),
  kickstartBegin(0),
  pwmVal(0)
{
}

void Pump::update(bool benchmarkMode) {
  float T = sensor.getTemperature();

  // Hysterese
  if (!isOn) {
    if (T > onTemp) {
      isOn = true;
      kickstartActive = true;
      kickstartBegin = millis();
    }
  } else {
    if (T < offTemp) {
      isOn = false;
      kickstartActive = false;
    }
  }

  if (isOn) {
    // PWM-Kurve
    int maxVal = benchmarkMode ? pwmMaxBench : pwmMax;
    pwmVal = mapTemperature(T, 25.0f, 60.0f, pwmMin, maxVal);

    // Kickstart
    if (kickstartActive) {
      unsigned long elapsed = millis() - kickstartBegin;
      if (elapsed < kickstartTime) {
        pwmVal = kickstartValue;
      } else {
        kickstartActive = false;
      }
    }
  } else {
    pwmVal = 0;
  }

  // Output
  setPWM_25kHz(pin, pwmVal);
}

int Pump::mapTemperature(float temp, float tLow, float tHigh, int pwmLow, int pwmHigh) {
  if (temp <= tLow) return pwmLow;
  if (temp >= tHigh) return pwmHigh;
  float ratio = (temp - tLow) / (tHigh - tLow);
  return pwmLow + (int)(ratio * (pwmHigh - pwmLow));
}

int Pump::getCurrentPWM() const {
  return pwmVal;
}
