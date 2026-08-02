#include "DisplayManager.h"

#include "Sensor.h"
#include "Pump.h"
#include "Fan.h"

DisplayManager::DisplayManager(Sensor& sensor1Ref,
                               Sensor& sensor2Ref,
                               Pump&   pump1Ref,
                               Pump&   pump2Ref,
                               Fan&    fanRef,
                               uint8_t i2cAddress)
: display(OLED_SCREEN_WIDTH, OLED_SCREEN_HEIGHT, &Wire),
  s1(sensor1Ref),
  s2(sensor2Ref),
  p1(pump1Ref),
  p2(pump2Ref),
  fan(fanRef),
  oledAddress(i2cAddress)
{
}

bool DisplayManager::begin() {
  Wire.begin();
  if (!display.begin(SSD1306_SWITCHCAPVCC, oledAddress)) {
    return false;
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println(F("Display OK!"));
  display.display();

  return true;
}

// Print a sensor's temperature, or "ERR" if the sensor is faulted.
static void printTemperature(Adafruit_SSD1306& display, Sensor& sensor) {
  if (sensor.isFaulted()) {
    display.print(F("ERR"));
  } else {
    display.print(sensor.getTemperature(), 1);
    display.print(F("C"));
  }
}

void DisplayManager::update(bool benchmarkMode) {
  display.clearDisplay();

  int p1PWM  = p1.getCurrentPWM();
  int p2PWM  = p2.getCurrentPWM();
  int fanPWM = fan.getCurrentPWM();

  display.setCursor(0, 0);
  display.print(F("T1="));
  printTemperature(display, s1);

  display.setCursor(64, 0);
  display.print(F("T2="));
  printTemperature(display, s2);

  display.setCursor(0, 10);
  display.print(F("P1="));
  display.print(p1PWM);

  display.setCursor(64, 10);
  display.print(F("P2="));
  display.print(p2PWM);

  display.setCursor(0, 20);
  display.print(F("Fan="));
  display.print(fanPWM);

  display.setCursor(64, 20);
  display.print(benchmarkMode ? F("BM: ON") : F("BM: OFF"));

  display.display();
}
