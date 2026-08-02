#pragma once
#include <stdint.h>
#include "Actuator.h"

// Pump is an Actuator with an added kickstart period. A kickstart
// forces a high PWM value for a short time after startup, so the
// pump motor starts reliably at a low duty cycle.
class Pump : public Actuator {
public:
  // sensorRef: which sensor to read
  // pwmPin: PWM output pin (9, 10, ...)
  // normalHyst: on/off temperature for normal mode
  // normalCurve: temperature-to-PWM curve for normal mode
  // benchHyst: on/off temperature for benchmark mode
  // benchCurve: temperature-to-PWM curve for benchmark mode
  // kickstartDuration: kickstart length, in milliseconds
  // kickstartPWM: PWM value (0-100) during kickstart
  Pump(const Sensor& sensorRef,
       uint8_t pwmPin,
       const ActuatorHysteresis& normalHyst,
       const ActuatorCurve& normalCurve,
       const ActuatorHysteresis& benchHyst,
       const ActuatorCurve& benchCurve,
       unsigned long kickstartDuration,
       uint8_t kickstartPWM);

protected:
  // Actuator::update() calls this after it computes the curve's PWM
  // value. This override forces kickValue for the kickstart duration.
  void postProcessPWM(int& pwmVal, unsigned long elapsedSinceOn) override;

private:
  unsigned long kickTime;   // Kickstart duration, in milliseconds
  uint8_t kickValue;        // PWM value during kickstart
};
