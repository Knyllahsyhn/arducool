#include "Pump.h"

Pump::Pump(const Sensor& sensorRef,
           uint8_t pwmPin,
           const ActuatorHysteresis& normalHyst,
           const ActuatorCurve& normalCurve,
           const ActuatorHysteresis& benchHyst,
           const ActuatorCurve& benchCurve,
           unsigned long kickstartDuration,
           int kickstartPWM)
: Actuator(sensorRef,  // Basisklassen-Konstruktor
           pwmPin,
           normalHyst,
           normalCurve,
           benchHyst,
           benchCurve),
  kickTime(kickstartDuration),
  kickValue(kickstartPWM)
{
}

/**
 * postProcessPWM() wird von Actuator::update() aufgerufen,
 * nachdem die Hysterese geprüft und ein "linearer" PWM-Wert
 * (je nach Kennlinie) berechnet wurde. Hier können wir
 * den Kickstart übersteuern.
 * 
 * @param pwmVal          [in/out] der bereits berechnete PWM-Wert (0..255)
 * @param elapsedSinceOn  Zeit in ms seit dem letzten Einschalten
 */
void Pump::postProcessPWM(int& pwmVal, unsigned long elapsedSinceOn) {
  // Solange die Zeit seit dem Einschalten (elapsedSinceOn) 
  // kleiner ist als kickTime, erzwingen wir den kickValue
  if (elapsedSinceOn < kickTime) {
    pwmVal = kickValue; 
  }
  // Danach bleibt pwmVal so, wie Actuator::mapTemperature() ihn ermittelt hat.
}
