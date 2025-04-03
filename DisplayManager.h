#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Forward-Declarations deiner Klassen, damit wir sie hier als Referenzen nutzen können.
// Du kannst alternativ #include "Sensor.h", "Pump.h" und "Fan.h" verwenden,
// wenn du Zugriff auf deren Definition brauchst.
class Sensor;
class Pump;
class Fan;

// Passt du für dein Display an
#define OLED_SCREEN_WIDTH  128
#define OLED_SCREEN_HEIGHT 40

/**
 * DisplayManager:
 *  - verwaltet ein I2C-OLED (SSD1306)
 *  - zeigt z.B. zwei Sensor-Temperaturen und
 *    die aktuellen PWM-Werte von 2 Pumpen + 1 Lüfter an
 */
class DisplayManager {
public:
  /**
   * Konstruktor:
   *  - Referenzen auf Sensoren, Pumpen, Lüfter, damit wir deren Daten anzeigen können.
   *  - I2C-Adresse (Standard oft 0x3C)
   */
  DisplayManager(Sensor& sensor1Ref,
                 Sensor& sensor2Ref,
                 Pump&   pump1Ref,
                 Pump&   pump2Ref,
                 Fan&    fanRef,
                 uint8_t i2cAddress = 0x3D);

  /**
   * Display initialisieren
   *  - gibt true zurück, wenn das Display erfolgreich erkannt wurde
   */
  bool begin();

  /**
   * In jedem Zyklus (oder alle X ms) aufrufen, um die Anzeige zu aktualisieren
   * Hier kannst du Frequenz, Layout usw. anpassen
   */
  void update();

private:
  // SSD1306-Objekt
  Adafruit_SSD1306 display;

  // Referenzen auf deine Objekte
  Sensor& s1;
  Sensor& s2;
  Pump&   p1;
  Pump&   p2;
  Fan&    fan;

  // I2C-Adresse
  uint8_t oledAddress;
};
