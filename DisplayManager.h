#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

class Sensor;
class Pump;
class Fan;

// Adjust these to match your OLED module.
#define OLED_SCREEN_WIDTH  128
#define OLED_SCREEN_HEIGHT 64

// DisplayManager shows sensor temperatures and actuator PWM values
// on an I2C OLED (SSD1306).
class DisplayManager {
public:
  // sensor1Ref, sensor2Ref, pump1Ref, pump2Ref, fanRef: objects to
  // read data from.
  // i2cAddress: I2C address of the OLED module (0x3C is common).
  DisplayManager(Sensor& sensor1Ref,
                 Sensor& sensor2Ref,
                 Pump&   pump1Ref,
                 Pump&   pump2Ref,
                 Fan&    fanRef,
                 uint8_t i2cAddress = 0x3C);

  // Start the display. Returns true if the display responds on the
  // I2C bus.
  bool begin();

  // Redraw the display with the current sensor and actuator values.
  // Call this at a fixed interval, for example every 1000 ms.
  void update(bool benchmarkMode);

private:
  Adafruit_SSD1306 display;

  Sensor& s1;
  Sensor& s2;
  Pump&   p1;
  Pump&   p2;
  Fan&    fan;

  uint8_t oledAddress;
};
