#include "DisplayManager.h"

// Wenn du Forward-Declarations benutzt, brauchst du hier
// die eigentlichen Includes, damit wir die Klassen implementieren können.
#include "Sensor.h"
#include "Pump.h"
#include "Fan.h"
extern bool benchmarkMode;

// Konstruktor
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

// begin()
bool DisplayManager::begin() {
  // Display starten
  // SSD1306_SWITCHCAPVCC => interne Ladungspumpe für OLED
  Wire.begin();
  if(!display.begin(SSD1306_SWITCHCAPVCC, oledAddress)) {
    // Fehlschlag
    return false;
  }
  display.clearDisplay();
  display.setTextSize(1);         // Schriftgröße
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println(F("Display OK!"));
  display.display();

  return true; // Erfolg
}

// update()
void DisplayManager::update() {
  // 1) Display puffern löschen
  display.clearDisplay();

  // 2) Werte einlesen
  float temp1 = s1.getTemperature();
  float temp2 = s2.getTemperature();
  int   p1PWM = p1.getCurrentPWM();
  int   p2PWM = p2.getCurrentPWM();
  int   fanPWM= fan.getCurrentPWM();

  // 3) Darstellung
  // Zeile 1: T1
  display.setCursor(0, 0);
  display.print(F("T1="));
  display.print(temp1, 1); // 1 Nachkommastelle
  display.print(F("C"));

  // Zeile 2: T2
  display.setCursor(64, 0);
  display.print(F("T2="));
  display.print(temp2, 1);
  display.print(F("C"));

  // Zeile 3: Pump1 PWM
  display.setCursor(0, 10);
  display.print(F("P1="));
  display.print(p1PWM);

  // Zeile 4: Pump2 PWM
  display.setCursor(64, 10);
  display.print(F("P2="));
  display.print(p2PWM);

  // Zeile 5: Fan PWM
  display.setCursor(0, 20);
  display.print(F("Fan="));
  display.print(fanPWM);

   display.setCursor(64, 20);
  if (benchmarkMode) {
    display.print(F("BM: ON"));
  } else {
    display.print(F("BM: OFF"));
  }


  // (Optional) Mehr Zeilen, Benchmark Mode etc.

  // 4) Anzeigen
  display.display();
}
