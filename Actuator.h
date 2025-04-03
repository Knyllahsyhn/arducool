#pragma once
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

class Actuator {
public:
  Actuator(const Sensor& sensorRef,
           uint8_t pwmPin,
           // Normalmodus
           const ActuatorHysteresis& normalHyst,
           const ActuatorCurve& normalCurve,
           // Benchmarkmodus
           const ActuatorHysteresis& benchHyst,
           const ActuatorCurve& benchCurve);

  virtual void update(bool benchmarkMode);

  // Getter
  int  getCurrentPWM() const { return currentPWM; }
  bool isOn()         const { return stateOn;    }

protected:
  const Sensor& sensor;
  uint8_t pin;

  // Zwei Hysterese-Paare
  ActuatorHysteresis normalHyst;
  ActuatorHysteresis benchHyst;

  // Zwei Kennlinien
  ActuatorCurve normalCurve;
  ActuatorCurve benchCurve;

  bool stateOn;     // Ein/Aus
  int  currentPWM;  // 0..255

  // Zeit, wann zuletzt eingeschaltet
  unsigned long lastOnTime;

  // Basismethode für lineares Mapping
  int mapTemperature(float T, const ActuatorCurve& curve);

  // Virtuelle Methode für Post-Processing (z.B. Kickstart)
  virtual void postProcessPWM(int& pwmVal, unsigned long elapsedSinceOn) {}

private:
  void setOff();
};
