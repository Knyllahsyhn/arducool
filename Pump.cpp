#include "Pump.h"

Pump::Pump(const Sensor& sensorRef,
           uint8_t pwmPin,
           const ActuatorHysteresis& normalHyst,
           const ActuatorCurve& normalCurve,
           const ActuatorHysteresis& benchHyst,
           const ActuatorCurve& benchCurve,
           unsigned long kickstartDuration,
           uint8_t kickstartPWM)
: Actuator(sensorRef,
           pwmPin,
           normalHyst,
           normalCurve,
           benchHyst,
           benchCurve),
  kickTime(kickstartDuration),
  kickValue(kickstartPWM)
{
}

// postProcessPWM() runs after Actuator::update() checks the
// hysteresis and computes a PWM value from the active curve. While
// elapsedSinceOn is less than kickTime, this function overrides that
// value with kickValue. After the kickstart period, pwmVal keeps the
// value from Actuator::mapTemperature().
void Pump::postProcessPWM(int& pwmVal, unsigned long elapsedSinceOn) {
  if (elapsedSinceOn < kickTime) {
    pwmVal = kickValue;
  }
}
