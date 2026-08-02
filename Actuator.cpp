#include "Actuator.h"

Actuator::Actuator(const Sensor& sensorRef,
                   uint8_t pwmPin,
                   const ActuatorHysteresis& normalHyst,
                   const ActuatorCurve& normalC,
                   const ActuatorHysteresis& benchHyst,
                   const ActuatorCurve& benchC)
: sensor(sensorRef),
  pin(pwmPin),
  normalHyst(normalHyst),
  benchHyst(benchHyst),
  normalCurve(normalC),
  benchCurve(benchC),
  stateOn(false),
  currentPWM(0),
  lastOnTime(0)
{
}

void Actuator::update(bool benchmarkMode) {
  // Fail safe: run at full power when the sensor reading is not
  // trustworthy. This favors overcooling over undercooling. Leave
  // stateOn and lastOnTime untouched, so the hysteresis and kickstart
  // state machine resumes correctly once the fault clears.
  if (sensor.isFaulted()) {
    currentPWM = 100;
    setPWM_25kHz(pin, 100);
    return;
  }

  float T = sensor.getTemperature();

  // 1) Select the active hysteresis pair.
  float onTemp  = benchmarkMode ? benchHyst.onTemp  : normalHyst.onTemp;
  float offTemp = benchmarkMode ? benchHyst.offTemp : normalHyst.offTemp;

  // 2) On/off logic.
  if (!stateOn) {
    if (T > onTemp) {
      stateOn = true;
      lastOnTime = millis();
    }
  } else {
    if (T < offTemp) {
      setOff();
      return;
    }
  }

  if (!stateOn) {
    currentPWM = 0;
    return;
  }

  // 3) Select the active curve and map the temperature to a PWM value.
  const ActuatorCurve& usedCurve = benchmarkMode ? benchCurve : normalCurve;
  int pwmVal = mapTemperature(T, usedCurve);

  // 4) Let a subclass adjust the value (for example, pump kickstart).
  unsigned long elapsed = millis() - lastOnTime;
  postProcessPWM(pwmVal, elapsed);

  if (pwmVal < 0)   pwmVal = 0;
  if (pwmVal > 100) pwmVal = 100;

  // 5) Store and output the result.
  currentPWM = pwmVal;
  setPWM_25kHz(pin, pwmVal);
}

int Actuator::mapTemperature(float T, const ActuatorCurve& curve) {
  if (T <= curve.TLow)  return curve.pwmLow;
  if (T >= curve.THigh) return curve.pwmHigh;
  float ratio = (T - curve.TLow) / (curve.THigh - curve.TLow);
  float pwm = curve.pwmLow + ratio * (curve.pwmHigh - curve.pwmLow);
  return (int)(pwm + 0.5f);  // Round to the nearest value, do not truncate.
}

void Actuator::setOff() {
  stateOn = false;
  currentPWM = 0;
  setPWM_25kHz(pin, 0);
}
