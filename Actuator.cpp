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
  float T = sensor.getTemperature();

  // 1) Wähle, welches Hysterese-Paar wir nehmen
  float onTemp  = benchmarkMode ? benchHyst.onTemp  : normalHyst.onTemp;
  float offTemp = benchmarkMode ? benchHyst.offTemp : normalHyst.offTemp;

  // 2) An-/Aus-Logik
  if (!stateOn) {
    // War aus, evtl. einschalten?
    if (T > onTemp) {
      stateOn = true;
      lastOnTime = millis();
    }
  } else {
    // War an, evtl. ausschalten?
    if (T < offTemp) {
      setOff();
      return; 
    }
  }

  if (!stateOn) {
    // Wenn jetzt immer noch aus, PWM=0, fertig
    currentPWM = 0;
    return;
  }

  // 3) Wähle Kennlinie
  const ActuatorCurve& usedCurve = benchmarkMode ? benchCurve : normalCurve;
  int pwmVal = mapTemperature(T, usedCurve);

  // 4) Virtuelle Methode => abgeleitete Klasse kann Kickstart etc.
  unsigned long elapsed = millis() - lastOnTime;
  postProcessPWM(pwmVal, elapsed);

  // Begrenzen
  if (pwmVal < 0)   pwmVal = 0;
  if (pwmVal > 255) pwmVal = 255;

  // 5) Speichern & ausgeben
  currentPWM = pwmVal;
  setPWM_25kHz(pin, pwmVal);
}

uint8_t Actuator::mapTemperature(float T, const ActuatorCurve& curve) {
  if (T <= curve.TLow)  return curve.pwmLow;
  if (T >= curve.THigh) return curve.pwmHigh;
  float ratio = (T - curve.TLow) / (curve.THigh - curve.TLow);
  return curve.pwmLow + (int)(ratio * (curve.pwmHigh - curve.pwmLow));
}

void Actuator::setOff() {
  stateOn = false;
  currentPWM = 0;
  setPWM_25kHz(pin, 0);
}
