# Arduino-Pumpen- und Lüftersteuerung für ein 24-V-Netzteil

Dieses Projekt steuert zwei Pumpen und einen Lüfter mit einem Arduino Nano.
Der Controller liest zwei NTC-Temperatursensoren aus und setzt den PWM-Wert
für jede Pumpe und den Lüfter. Das PWM-Signal läuft mit 25 kHz, dadurch
erzeugen Pumpen und Lüfter kein hörbares Pfeifen. Der Code kennt zwei
Einstellungssätze: Normalmodus und Benchmarkmodus. Ein Taster an Pin 4
schaltet zwischen beiden Modi um.

Probiere die [Wokwi-Simulation](https://wokwi.com/projects/427233338419995649) aus.
Hinweis: Die PWM-Ausgabe funktioniert im Simulator nicht korrekt. Der Code
funktioniert korrekt auf echter Arduino-Nano-Hardware.

---

## Funktionen

1. **Zwei Betriebsmodi**
   - Normalmodus: benutzerdefinierte Ein-/Ausschalttemperaturen und PWM-Kurven.
   - Benchmarkmodus: steilere Kurven, andere Ein-/Ausschalttemperaturen und
     ein höherer maximaler PWM-Wert.
   - Ein Taster schaltet den Modus um. Die LED an Pin 13 zeigt den aktuellen
     Modus: an im Benchmarkmodus, aus im Normalmodus.

2. **Objektorientiertes Design**
   - Die Basisklasse `Actuator` verwaltet zwei Hysterese-Sätze
     (Ein-/Ausschalttemperatur) und zwei Temperatur-zu-PWM-Kurven.
   - `Pump` erweitert `Actuator` um eine Kickstart-Phase, damit der
     Pumpenmotor bei niedrigem PWM-Wert zuverlässig anläuft.
   - `Fan` erweitert `Actuator` ohne Kickstart.

3. **NTC-Temperatursensoren und Fail-Safe-Verhalten**
   - Jedes `Sensor`-Objekt rechnet einen rohen ADC-Wert mit der
     Beta-Formel in eine Temperatur um.
   - Ein exponentiell gleitender Mittelwert (EMA) glättet den Messwert.
   - Meldet ein Sensor einen ungültigen Rohwert (offener oder
     kurzgeschlossener Stromkreis), behält der Sensor seine letzte
     gültige Temperatur und setzt ein Fehler-Flag.
   - Die zugehörige Pumpe oder der Lüfter läuft dann mit 100% PWM, bis
     der Sensor wieder einen gültigen Messwert liefert. Dieses Fail-Safe
     geht im Fehlerfall vom schlimmsten Fall aus (Überhitzung), da der
     Controller dem Sensor nicht mehr vertrauen kann.

4. **Nicht blockierende Hauptschleife**
   - Die Schleife nutzt `millis()`, um jede Aufgabe zeitlich zu steuern.
     Der Code ruft kein `delay()` auf, dadurch bleiben alle Aufgaben
     reaktionsfähig.

5. **25-kHz-PWM an den Pins 9, 10 und 3**
   - `Timers.cpp` stellt Timer1 und Timer2 per direktem Registerzugriff
     auf 25 kHz ein.
   - `setPWM_25kHz()` setzt das Tastverhältnis für einen bestimmten Pin.

6. **Zentrale Pin-Konfiguration**
   - `Pins.h` listet jeden vom Sketch verwendeten Pin auf. Ändere deine
     Verkabelung nur in dieser einen Datei.

7. **Modulare Dateien**
   - `Pins.h`: Pin-Zuordnung.
   - `Timers.*`: Einrichtung der 25-kHz-PWM.
   - `Sensor.*`: NTC-Auslesung, EMA-Glättung und Fehlererkennung.
   - `Actuator.*`: Basisklasse für Hysterese, Kurven und Fail-Safe-Verhalten.
   - `Pump.*` und `Fan.*`: Pumpen-Kickstart und lüfterspezifisches Verhalten.
   - `DisplayManager.*`: OLED-Ausgabe.

8. **128x64-SSD1306-OLED-Ausgabe**
   - Das Display zeigt die Temperatur für Sensor 1 und Sensor 2, oder
     `ERR` bei einem Sensorfehler.
   - Das Display zeigt den PWM-Wert für Pumpe 1, Pumpe 2 und den Lüfter.
   - Das Display zeigt den aktuellen Modus (`BM: ON` oder `BM: OFF`).

---

## Hardware-Aufbau

1. Arduino Nano (5 V, ATmega328P).
2. 24-V-Netzteil. Hinweis: Wir haben zuerst 12 V getestet und für unsere
   Pumpen als zu schwach befunden, deshalb sind wir auf 24 V gewechselt.
3. Zwei 10-kΩ-NTC-Sensoren (Beta ≈ 3950) in einem Spannungsteiler, an den
   analogen Pins `A0` und `A1`.
4. PWM-Ausgänge mit 25 kHz:
   - Pumpe 1 → Pin 9 (OC1A)
   - Pumpe 2 → Pin 10 (OC1B)
   - Lüfter → Pin 3 (OC2B)
5. Benchmark-Taster an Pin 4 (`INPUT_PULLUP`). Der Taster schaltet den
   Hysterese- und Kurvensatz um.
6. Onboard-LED (Pin 13) als Benchmarkmodus-Anzeige.
7. N-Kanal-MOSFET-Treiber für die Pumpen und den Lüfter.
8. 128x64-SSD1306-OLED-Display (SCL → A5, SDA → A4).

![Schematic](https://github.com/user-attachments/assets/a0198f37-dbb2-4c98-975d-91f54a14b9a9)
[![Build photo](https://i.postimg.cc/4xChdyvJ/IMG-4341.jpg)](https://postimg.cc/JGqhY1dg)

Die MOSFETs bestimmen die Leistungsgrenze für jeden Kanal. Wir haben
IRLZ44N-MOSFETs verwendet, die mehr Reserve bieten, als diese Pumpen
brauchen.

---

## Arbeitsweise

1. **Timer-Konfiguration**: `initTimers25kHz()` stellt Timer1 (Pins 9,
   10) und Timer2 (Pin 3) auf 25 kHz ein.
2. **Sensorauslesung**: `Sensor::update()` liest den ADC aus, rechnet
   den Wert mit der Beta-Formel in Grad Celsius um und glättet das
   Ergebnis mit einem EMA-Filter. Ist der rohe ADC-Wert 0 oder 1023,
   meldet der Sensor einen Fehler, statt eine neue Temperatur zu
   berechnen.
3. **Basisklasse `Actuator`**: `update(bool benchmarkMode)` prüft
   zuerst das Sensor-Fehler-Flag. Bei einem Fehler läuft der Aktor mit
   100% PWM. Andernfalls prüft die Funktion die Hysterese, bildet die
   Temperatur mit der aktiven Kurve auf einen PWM-Wert ab und ruft
   `postProcessPWM()` auf.
4. **`Pump`**: `postProcessPWM()` erzwingt für die Kickstart-Dauer
   einen hohen PWM-Wert, damit der Motor zuverlässig anläuft.
5. **`Fan`**: nutzt das Verhalten der Basisklasse ohne Kickstart.
6. **Benchmarkmodus**: Der Taster schaltet `benchmarkMode` um. Das
   wechselt den aktiven Hysterese- und Kurvensatz und setzt die LED an
   Pin 13.
7. **Nicht blockierende Schleife**: `loop()` prüft `millis()` gegen
   feste Intervalle, um jede Aufgabe auszuführen. Der Code ruft kein
   `delay()` auf.

---

## Kurzbeispiel (Ausschnitt aus dem Haupt-Sketch)

```cpp
void loop() {
  unsigned long now = millis();
  if ((now - lastControlTime) >= controlInterval) {
    lastControlTime = now;
    handleBenchmarkButton();

    if (now - lastDisplayTime >= displayInterval) {
      lastDisplayTime = now;
      displayMgr.update(benchmarkMode);
    }

    sensor1.update();
    sensor2.update();

    pump1.update(benchmarkMode);
    pump2.update(benchmarkMode);
    fan1.update(benchmarkMode);
  }
}
```

---

## Installation

1. Klone dieses Repository oder lade es mit allen `.h`- und
   `.cpp`-Dateien herunter.
2. Öffne `arducool.ino` in der Arduino IDE, oder nutze PlatformIO.
3. Wähle "Arduino Nano" und den passenden COM-Port aus.
4. Kompiliere den Sketch und lade ihn hoch.
5. Verdrahte die Hardware wie oben beschrieben (Sensoren,
   MOSFET-Treiber und so weiter).
6. Schalte das Board ein. Das System steuert die Pumpen und den
   Lüfter jetzt anhand der Sensortemperaturen.

---

## Hinweise zum Speicher

Die Beta-Formel nutzt `log()` aus `<math.h>`, das Flash und RAM
belegt. Brauchst du mehr freien RAM, ersetze die Beta-Formel durch
eine Lookup-Tabelle. Die Debug-Ausgabe nutzt bereits das `F()`-Makro,
um Zeichenketten im Flash statt im RAM abzulegen.

---

## Lizenz

GNU GPLv3

---

## Kontakt und Mitwirkung

Eröffne ein Issue oder einen Pull Request bei Fragen oder
Verbesserungsvorschlägen.

Passe die Beta-Konstante an das Datenblatt deines NTC an, für genaue
Messwerte.
