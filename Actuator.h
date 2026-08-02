#pragma once
#include <stdint.h>
#include <Arduino.h>
#include "Sensor.h"
#include "Timers.h"

struct ActuatorCurve {
  float TLow;
  float THigh;
  int pwmLow;
  int pwmHigh;
};

struct ActuatorHysteresis {
  float onTemp;
  float offTemp;
};

// Actuator drives a pump or a fan from a temperature reading.
// It selects between a normal-mode and a benchmark-mode hysteresis
// pair and curve, and fails safe to 100% PWM if its sensor is faulted.
class Actuator {
public:
  Actuator(const Sensor& sensorRef,
           uint8_t pwmPin,
           const ActuatorHysteresis& normalHyst,
           const ActuatorCurve& normalCurve,
           const ActuatorHysteresis& benchHyst,
           const ActuatorCurve& benchCurve);

  virtual void update(bool benchmarkMode);
  virtual void setOff();

  int getCurrentPWM() const { return currentPWM; }
  bool isOn()         const { return stateOn;    }

protected:
  const Sensor& sensor;
  uint8_t pin;

  ActuatorHysteresis normalHyst;
  ActuatorHysteresis benchHyst;

  ActuatorCurve normalCurve;
  ActuatorCurve benchCurve;

  bool stateOn;
  int  currentPWM;    // 0-100

  unsigned long lastOnTime;

  // Map a temperature to a PWM value (0-100) with the given curve.
  int mapTemperature(float T, const ActuatorCurve& curve);

  // Called after update() computes a PWM value from the curve.
  // A subclass overrides this to change the value (for example,
  // Pump uses it for a kickstart period).
  virtual void postProcessPWM(int& pwmVal, unsigned long elapsedSinceOn) {}
};
