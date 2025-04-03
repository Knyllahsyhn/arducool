# Arduino-basierte Pumpen- und Lüftersteuerung (OOP, 25 kHz PWM, separater Hysterese‐ und Kennlinienmodus)

Dieses Projekt steuert mithilfe eines Arduino Nano zwei Pumpen und einen Lüfter auf Basis von Temperaturmessungen. Dabei werden **Hysterese**‐Schaltpunkte, **Kennlinien** (Normal-/Benchmarkmodus) und ein **Kickstart** für Pumpen implementiert. Die PWM-Ausgänge laufen auf **25 kHz**, um störende Pfeifgeräusche zu vermeiden. Der Code ist **objektorientiert** aufgebaut, wodurch Pumpen- und Lüfterlogik modular und leicht erweiterbar sind.

---

## Merkmale

1. **Zwei Betriebsmodi**:  
   - **Normalmodus** mit eigenen Temperaturgrenzen & Kennlinien  
   - **Benchmarkmodus**: steilere Kennlinien, andere Hysterese für Ein-/Ausschalten, höhere PWM-Werte  
   - Umschaltbar per Taster (mit LED-Anzeige)

2. **Objektorientierter Aufbau**  
   - **Basisklasse** `Actuator` verwaltet:  
     - Zwei Sätze **Hysterese**‐Grenzen (onTemp/offTemp)  
     - Zwei **Kennlinien** (Normal / Benchmark)  
     - Ein-/Ausschalten per Hysterese + lineares Mapping von Temperatur zu PWM  
   - **Pump** erbt von `Actuator` und ergänzt einen **Kickstart** (hoher PWM-Wert für einige Sekunden beim Einschalten)  
   - **Fan** erbt von `Actuator` (Hysterese & Kennlinien), **ohne** Kickstart

3. **NTC-Temperatursensoren** (Beta-Formel + Exponential‐Glättung)  
   - Genaue Temperaturberechnung mithilfe der Beta‐Gleichung  
   - Rauscharm dank gleitendem Mittelwert (EMA)

4. **Nicht-blockierende** Zeitsteuerung  
   - **Kein** `delay()`, sondern `millis()`-basiertes Abfragen in regelmäßigen Intervallen

5. **25 kHz PWM** auf Pins 9, 10 und 3  
   - Vermeidet hörbare Pfeifgeräusche  
   - Implementiert via direkter Timer-Register‐Konfiguration und einer Hilfsfunktion `setPWM_25kHz()`

6. **Modulare Dateien**  
   - `Timers.*` → 25 kHz PWM  
   - `Sensor.*` → NTC‐Beta‐Formel & Glättung  
   - `Actuator.*` → Basisklasse (Hysterese, Kennlinien)  
   - `Pump.*` und `Fan.*` → Spezialisierungen mit Kickstart (Pump) oder ohne (Fan)

---

## Projektstruktur

Ein mögliches Layout (Arduino IDE / PlatformIO):

//kommt noch

---

## Hardware‐Setup

1. **Arduino Nano** (5V, ATmega328p).  
2. **NTC-Sensoren** (z.B. 10 kΩ, Beta ~3950) in Spannungsteilern.  
   - Analogeingänge `A0`, `A1` messen die Spannung.  
3. **PWM-Ausgänge**:  
   - **Pumpe1** → Pin 9 (OC1A)  
   - **Pumpe2** → Pin 10 (OC1B)  
   - **Lüfter** → Pin 3 (OC2B)  
   - Alle auf ~25 kHz (kein Pfeifen).  
4. **Hauptschalter** an Pin 2 (digital, `INPUT_PULLUP`).  
5. **Benchmark‐Taster** an Pin 4 (digital, `INPUT_PULLUP`), toggelt Modus.  
6. **Onboard‐LED** (Pin 13) als Aktivitätsanzeige für den Benchmarkmodus.  
7. **MOSFETs** oder Treiber‐Stufe zum Ansteuern von Pumpen und Lüfter.

![image](https://github.com/user-attachments/assets/fc299ae5-d2fb-4789-9afa-03acfe1a1893)

---

## Ablauf / Funktionsweise

1. **Timer‐Init**  
   - `initTimers25kHz()` (in `Timers.cpp`) stellt Timer1 (Pins 9,10) und Timer2 (Pin 3) auf eine ~25 kHz PWM-Frequenz.

2. **Sensor**  
   - `Sensor`‐Klasse (`Sensor.cpp`) liest den ADC‐Wert, rechnet ihn via Beta-Formel in °C um und glättet das Ergebnis.

3. **Basisklasse `Actuator`**  
   - Hält zwei Paare **Hysterese** (normal / benchmark) und zwei **Kennlinien** (normal / benchmark).  
   - `update(bool benchmarkMode)` prüft den Ein-/Aus‐Zustand via Hysterese, mappt Temperatur linear auf PWM und ruft `postProcessPWM()` auf.

4. **`Pump`** (abgeleitet von `Actuator`)  
   - Implementiert **Kickstart** in `postProcessPWM()`: Bei Einschalten wird für eine definierte Zeit (z.B. 1–2 s) ein hoher PWM‐Wert erzwungen, um ein Stehenbleiben zu vermeiden.

5. **`Fan`** (abgeleitet von `Actuator`)  
   - Braucht ggf. **keinen** Kickstart.  
   - Kann trotzdem andere Hysterese- und Kennlinieneinstellungen im Benchmarkmodus erhalten.

6. **Benchmarkmodus**  
   - Taster toggelt ein globales `benchmarkMode`‐Flag.  
   - Aktoren wählen dann die **Benchmark‐Hysterese** (z.B. tiefere Ein-/Ausschalt‐Temperaturen) und die **steilere Kennlinie** (höhere pwmHigh).

7. **Nicht-blockierend**  
   - `loop()` nutzt ein Zeitintervall (`millis()`), um alle X ms (`sensor.update()`, `pump.update()`, `fan.update()`) auszuführen. Keine `delay()`‐Aufrufe.
---

## Installation und Verwendung

1. **Arduino IDE** oder PlatformIO installieren.  
2. Alle `.h`/`.cpp`‐Dateien in einen Projektordner kopieren.  
3. In der **Hauptdatei** `Pumpensteuerung.ino` sicherstellen, dass Board/Port korrekt gewählt sind.  
4. Kompilieren und hochladen.  
5. **Hardware** gemäß obiger Pinbelegung anschließen.  
6. **Hauptschalter** auf ON → System misst Temperatur & regelt Pumpen/Lüfter.  
7. **Benchmark‐Taster** drücken → Modus wechselt, LED geht an/aus; steilere Kennlinien und andere Hysterese greifen.

---

## Beispielcode (Ausschnitt)

```cpp
// Hauptsketch (Pumpensteuerung.ino)
#include "Timers.h"
#include "Sensor.h"
#include "Pump.h"
#include "Fan.h"

// Taster-Flag
bool benchmarkMode = false;

// Hysteresen & Kennlinien definieren
ActuatorHysteresis pumpNormalHyst = {25.0f, 20.0f};
ActuatorHysteresis pumpBenchHyst  = {15.0f, 10.0f};

ActuatorCurve pumpNormalCurve     = {25.0f, 60.0f, 85, 128};
ActuatorCurve pumpBenchCurve      = {25.0f, 60.0f, 85, 230};

// Pumpe1
Pump pump1(sensor1, 9,
           pumpNormalHyst, pumpNormalCurve,
           pumpBenchHyst,  pumpBenchCurve,
           2000, 255); // Kickstart=2s, PWM=100%

// Fan
ActuatorHysteresis fanNormalHyst  = {25.0f, 20.0f};
ActuatorHysteresis fanBenchHyst   = {20.0f, 15.0f};

ActuatorCurve fanNormalCurve      = {25.0f, 60.0f, 0, 255};
ActuatorCurve fanBenchCurve       = {20.0f, 60.0f, 0, 255};

Fan fan1(sensor2, 3,
         fanNormalHyst, fanNormalCurve,
         fanBenchHyst,  fanBenchCurve);

void setup() {
  Serial.begin(9600);
  pinMode(mainSwitchPin, INPUT_PULLUP);
  pinMode(benchmarkButtonPin, INPUT_PULLUP);
  pinMode(ledPin, OUTPUT);

  initTimers25kHz();
}

void loop() {
  // Taster abfragen, benchmarkMode toggeln
  handleBenchmarkButton();

  // Zeitgesteuert (z.B. alle 500 ms):
  if (millis() - lastCheck >= 500) {
    lastCheck = millis();

    sensor1.update();
    sensor2.update();

    pump1.update(benchmarkMode);
    pump2.update(benchmarkMode);
    fan1.update(benchmarkMode);

    Serial.print(F("Pump1 PWM="));
    Serial.print(pump1.getCurrentPWM());
    Serial.print(F("  Fan1 PWM="));
    Serial.println(fan1.getCurrentPWM());
  }
}
```
---
##Lizenz

GNU GPLv3


---
##Kontakt

## Kontakt

Bei Fragen oder Anmerkungen:

- **Issues** erstellen

Viel Erfolg und Spaß beim Experimentieren!
