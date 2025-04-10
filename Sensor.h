#pragma once
#include <Arduino.h>

/**
 * Klasse, die die Temperaturmessung per NTC (Beta-Formel)
 * und Glättung (Exponential Moving Average) kapselt.
 */
class Sensor {
public:
  /**
   * Konstruktor:
   *  - analogPin: Welchen Pin lesen wir aus? (z.B. A0)
   *  - rFixed: Widerstand des Festwiderstands im Teiler (z.B. 10000.0)
   *  - beta: Beta-Koeffizient des NTC (z.B. 3950)
   *  - r0: NTC-Widerstand bei T0 (z.B. 10000 bei 25°C)
   *  - t0Kelvin: T0 in Kelvin (z.B. 298.15)
   *  - alpha: Glättungsfaktor (0..1)
   */
  Sensor(int analogPin,
         float rFixed,
         float beta,
         float t0Kelvin,
         float alpha = 0.1f);

  // Aufrufen, um neue Messung durchzuführen und zu glätten
  void update();

  // Liefert die geglättete Temperatur (°C)
  float getTemperature() const;

private:
  int pin;
  float R_FIXED;
  float B;
  float T0;
  float alphaFilter;

  bool firstRun;
  float filteredTemp;

  // Interne Funktion, die einmal den ADC liest, NTC berechnet und °C zurückgibt
  float readNTCTemp();
};
