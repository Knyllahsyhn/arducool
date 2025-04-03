#pragma once
#include "Actuator.h"

class Fan : public Actuator {
public:
  Fan(const Sensor& sensorRef,
      uint8_t pwmPin,
      // Normal + Benchmark-Hysterese
      const ActuatorHysteresis& normalHyst,
      const ActuatorCurve& normalC,
      const ActuatorHysteresis& benchHyst,
      const ActuatorCurve& benchC)
  : Actuator(sensorRef, pwmPin, normalHyst, normalC, benchHyst, benchC)
  {}

protected:
  void postProcessPWM(int& pwmVal, unsigned long elapsedSinceOn) override {};

};
 