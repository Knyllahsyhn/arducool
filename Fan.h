#pragma once
#include "Actuator.h"

// Fan is an Actuator with no kickstart. It uses the base class
// default behavior (no post-processing of the PWM value).
class Fan : public Actuator {
public:
  Fan(const Sensor& sensorRef,
      uint8_t pwmPin,
      const ActuatorHysteresis& normalHyst,
      const ActuatorCurve& normalC,
      const ActuatorHysteresis& benchHyst,
      const ActuatorCurve& benchC)
  : Actuator(sensorRef, pwmPin, normalHyst, normalC, benchHyst, benchC)
  {}
};
