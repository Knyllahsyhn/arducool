#include <stdint.h>
#pragma once
#include "Actuator.h"

/**
 * Pump erbt von Actuator
 *  - Hysterese (normal / benchmark)
 *  - Zwei Kennlinien
 *  - Kickstart (Dauer + PWM-Wert)
 */
class Pump : public Actuator {
public:
  /**
   * Konstruktor:
   *  - sensorRef:   Welcher Sensor?
   *  - pwmPin:      Pin (9,10 ...)
   *  - normalHyst:  on/off-Temp für Normalmodus
   *  - normalCurve: Kennlinie (TLow, THigh, pwmLow, pwmHigh) für Normalmodus
   *  - benchHyst:   on/off-Temp für Benchmark
   *  - benchCurve:  Kennlinie für Benchmark
   *  - kickstartDuration: ms
   *  - kickstartPWM: welcher Wert (0..255) während Kickstart?
   */
  Pump(const Sensor& sensorRef,
       uint8_t pwmPin,
       const ActuatorHysteresis& normalHyst,
       const ActuatorCurve& normalCurve,
       const ActuatorHysteresis& benchHyst,
       const ActuatorCurve& benchCurve,
       unsigned long kickstartDuration,
       uint8_t kickstartPWM);

protected:
  /**
   * postProcessPWM wird von Actuator::update() aufgerufen,
   * nachdem der lineare PWM-Wert berechnet wurde.
   * Hier implementieren wir den Kickstart.
   */
  void postProcessPWM(int& pwmVal, unsigned long elapsedSinceOn) override;

private:
  unsigned long kickTime;  // Dauer
  uint8_t kickValue;           // PWM während Kickstart
};
