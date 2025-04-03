# Arduino-basierte Pumpen- und Lüftersteuerung

Dieses Projekt bietet eine flexible, objektorientierte Lösung zur Steuerung von zwei Pumpen und einem Lüfter basierend auf Temperaturmessungen. Mithilfe eines Arduino Nano und 25 kHz PWM werden die Aktoren (Pumpen und Lüfter) präzise geregelt – inklusive separater Hysterese- und Kennlinienparameter für den Normal- und Benchmarkmodus. Zusätzlich zeigt ein I²C-OLED-Display den aktuellen Systemstatus (Temperaturen, PWM-Werte und Benchmarkmodus) an, und ein Taster ermöglicht das Umschalten in den Benchmarkmodus.

---

## Features

- **Zwei Betriebsmodi:**  
  - **Normalmodus:** Definierte Ein-/Ausschaltgrenzen und eine lineare Temperatur-zu-PWM-Kennlinie.  
  - **Benchmarkmodus:** Steilere Kennlinien, geänderte Hysteresegrenzen und höhere PWM-Werte zur Leistungssteigerung.

- **Objektorientierte Architektur:**  
  - Eine gemeinsame Basisklasse `Actuator` implementiert die grundlegende Logik (Hysterese, Temperatur-Mapping, PWM-Ausgabe).  
  - Die abgeleiteten Klassen `Pump` und `Fan` fügen spezifische Funktionen hinzu, wie z.B. einen Kickstart für die Pumpen.

- **NTC-Temperatursensoren:**  
  - Exakte Temperaturmessung über die Beta-Gleichung.  
  - Exponentielle Glättung (EMA) reduziert Rauschen und sorgt für stabile Werte.

- **25 kHz PWM:**  
  - Direkt per Timer-Konfiguration (Timer1 für Pins 9/10 und Timer2 für Pin 3) realisiert – verhindert hörbare Pfeifgeräusche.

- **I²C-OLED-Display:**  
  - Zeigt aktuelle Temperaturen, PWM-Werte und den Benchmarkmodus an.  
  - Separate Display-Klasse (`DisplayManager`) übernimmt die OLED-Ansteuerung.

- **Taster für Benchmarkumschaltung:**  
  - Ein Push-Button ermöglicht das Umschalten zwischen Normal- und Benchm
---

## Hardware

- **Arduino Nano** (ATmega328p, 5V)
- **NTC-Sensoren:**  
  - 10 kΩ NTC-Sensoren (z.B. Beta ≈ 3950) in einem Spannungsteiler (mit 10 kΩ Festwiderstand) an den analogen Eingängen (A0, A1)
- **PWM-Ausgänge:**  
  - **Pumpe1:** Pin 9 (Timer1, OC1A)  
  - **Pumpe2:** Pin 10 (Timer1, OC1B)  
  - **Lüfter:** Pin 3 (Timer2, OC2B)
- **I²C-OLED-Display:**  
  - Typischerweise 128×64 Pixel, angeschlossen an SDA (A4) und SCL (A5)
- **Taster:**  
  - Verbunden mit Pin 4 (INPUT_PULLUP) zum Umschalten des Benchmarkmodus
- **Hauptschalter:**  
  - Verbunden mit Pin 2 (INPUT_PULLUP) zur Steuerung des Gesamtsystems
- **Onboard-LED:**  
  - Pin 13 als Indikator für den Benchmarkmodus
- **MOSFETs/Treiber:**  
  - Zur Ansteuerung der Pumpen und des Lüfters

---

## Funktionsweise

1. **Timer-Konfiguration:**  
   `initTimers25kHz()` setzt Timer1 (für Pins 9 und 10) und Timer2 (für Pin 3) auf eine PWM-Frequenz von ca. 25 kHz. Dadurch werden die Pumpen und der Lüfter mit einer Frequenz angesteuert, die oberhalb des menschlichen Hörbereichs liegt.

2. **Temperaturmessung:**  
   Die `Sensor`-Klasse liest den ADC-Wert ein, rechnet ihn mit der Beta-Gleichung in °C um und glättet das Ergebnis mittels EMA.

3. **Aktorensteuerung:**  
   - Die Basisklasse `Actuator` verwaltet zwei Sätze von Hysteresegrenzen und Temperatur-zu-PWM-Kennlinien für Normal- und Benchmarkmodus.  
   - Abgeleitete Klassen wie `Pump` fügen einen Kickstart hinzu, der beim Einschalten der Pumpe einen kurzzeitigen hohen PWM-Wert erzwingt.  
   - Die `Fan`-Klasse übernimmt die Steuerung des Lüfters ohne Kickstart, verwendet aber ebenfalls unterschiedliche Parameter im Benchmarkmodus.

4. **Display:**  
   Die `DisplayManager`-Klasse steuert das I²C-OLED-Display, zeigt die aktuellen Temperaturen, die PWM-Werte der Pumpen und des Lüfters sowie den Benchmarkmodus an.

5. **Taster & Benchmarkmodus:**  
   Ein Push-Button schaltet per Toggle den Benchmarkmodus um. Der aktuelle Zustand wird über die Onboard-LED (Pin 13) und auf dem Display angezeigt.

6. **Nicht-blockierende Steuerung:**  
   Alle Regelzyklen werden mithilfe von `millis()` gesteuert, sodass das System ohne blockierende `delay()`-Aufrufe arbeitet und stets ansprechbar bleibt.

---

## Installation & Nutzung

1. **Voraussetzungen:**  
   - Arduino IDE oder PlatformIO  
   - Installierte Bibliotheken:  
     - [Adafruit SSD1306](https://github.com/adafruit/Adafruit_SSD1306)  
     - [Adafruit GFX](https://github.com/adafruit/Adafruit-GFX-Library)

2. **Projekt herunterladen:**  
   Klone oder lade dieses Repository herunter und öffne die `Pumpensteuerung.ino` in der Arduino IDE.

3. **Hardware anschließen:**  
   - Verbinde die NTC-Sensoren mit den analogen Eingängen A0 und A1.  
   - Schließe die Pumpen an die PWM-Pins 9 und 10 sowie den Lüfter an Pin 3 an.  
   - Verbinde das OLED-Display mit SDA (A4) und SCL (A5).  
   - Schließe den Benchmark-Taster an Pin 4 und den Hauptschalter an Pin 2 (beide mit internem Pullup) an.  
   - Achte auf eine korrekte Spannungsversorgung und Masseverbindungen.

4. **Code kompilieren & hochladen:**  
   Wähle das richtige Board (Arduino Nano) und den passenden Port, kompilieren und laden den Code hoch.

5. **Betrieb:**  
   - Schalte das System über den Hauptschalter ein.  
   - Beobachte die Messwerte und PWM-Werte auf dem OLED-Display.  
   - Drücke den Benchmark-Taster, um zwischen Normal- und Benchmarkmodus zu wechseln (der Status wird auf der LED und dem Display angezeigt).

---

## Anpassungsmöglichkeiten

- **Temperaturgrenzen und Kennlinien:**  
  Ändere die Werte in den Strukturen `ActuatorHysteresis` und `ActuatorCurve` je nach den gewünschten Schaltpunkten und der PWM-Kurve.

- **Kickstart-Parameter für Pumpen:**  
  Passe die Dauer und den PWM-Wert des Kickstarts in der `Pump`-Klasse an, um den Startvorgang zu optimieren.

- **Displaylayout:**  
  Passe das Layout in der `DisplayManager`-Klasse in `update()` an, um weitere Informationen anzuzeigen oder die Darstellung zu verändern.

- **Speicheroptimierung:**  
  Reduziere Debug-Ausgaben (verwende das `F()`-Makro) und überprüfe, ob die Floating-Point-Operationen (Beta-Gleichung) oder virtuelle Methoden weiter optimiert werden können, falls Flash/RAM knapp werden.

---

## Lizenz

*(Bitte füge hier deine gewünschte Lizenz ein, z.B. MIT, GPL, etc.)*

---

## Kontakt

Falls du Fragen, Vorschläge oder Probleme hast, eröffne bitte ein Issue in diesem Repository oder kontaktiere mich direkt per E-Mail.

---

Viel Erfolg und Spaß beim Experimentieren!
arkmodus.  
  - Eine Onboard-LED (Pin 13) zeigt den aktuellen Benchmarkstatus an.

- **Nicht-blockierende Steuerung:**  
  - Zeitsteuerung mittels `millis()` statt `delay()`, sodass das System responsiv bleibt und weitere Aufgaben parallel ausgeführt werden können.

---

## Projektstruktur

Das Projekt ist modular aufgebaut und in mehrere Dateien unterteilt:


---

## Hardware

- **Arduino Nano** (ATmega328p, 5V)
- **NTC-Sensoren:**  
  - 10 kΩ NTC-Sensoren (z.B. Beta ≈ 3950) in einem Spannungsteiler (mit 10 kΩ Festwiderstand) an den analogen Eingängen (A0, A1)
- **PWM-Ausgänge:**  
  - **Pumpe1:** Pin 9 (Timer1, OC1A)  
  - **Pumpe2:** Pin 10 (Timer1, OC1B)  
  - **Lüfter:** Pin 3 (Timer2, OC2B)
- **I²C-OLED-Display:**  
  - Typischerweise 128×64 Pixel, angeschlossen an SDA (A4) und SCL (A5)
- **Taster:**  
  - Verbunden mit Pin 4 (INPUT_PULLUP) zum Umschalten des Benchmarkmodus
- **Hauptschalter:**  
  - Verbunden mit Pin 2 (INPUT_PULLUP) zur Steuerung des Gesamtsystems
- **Onboard-LED:**  
  - Pin 13 als Indikator für den Benchmarkmodus
- **MOSFETs/Treiber:**  
  - Zur Ansteuerung der Pumpen und des Lüfters
 
![Untitled Sketch_Schaltplan](https://github.com/user-attachments/assets/f8880831-9631-4281-a42a-58a17b2641b9)

  

---

## Funktionsweise

1. **Timer-Konfiguration:**  
   `initTimers25kHz()` setzt Timer1 (für Pins 9 und 10) und Timer2 (für Pin 3) auf eine PWM-Frequenz von ca. 25 kHz. Dadurch werden die Pumpen und der Lüfter mit einer Frequenz angesteuert, die oberhalb des menschlichen Hörbereichs liegt.

2. **Temperaturmessung:**  
   Die `Sensor`-Klasse liest den ADC-Wert ein, rechnet ihn mit der Beta-Gleichung in °C um und glättet das Ergebnis mittels EMA.

3. **Aktorensteuerung:**  
   - Die Basisklasse `Actuator` verwaltet zwei Sätze von Hysteresegrenzen und Temperatur-zu-PWM-Kennlinien für Normal- und Benchmarkmodus.  
   - Abgeleitete Klassen wie `Pump` fügen einen Kickstart hinzu, der beim Einschalten der Pumpe einen kurzzeitigen hohen PWM-Wert erzwingt.  
   - Die `Fan`-Klasse übernimmt die Steuerung des Lüfters ohne Kickstart, verwendet aber ebenfalls unterschiedliche Parameter im Benchmarkmodus.

4. **Display:**  
   Die `DisplayManager`-Klasse steuert das I²C-OLED-Display, zeigt die aktuellen Temperaturen, die PWM-Werte der Pumpen und des Lüfters sowie den Benchmarkmodus an.

5. **Taster & Benchmarkmodus:**  
   Ein Push-Button schaltet per Toggle den Benchmarkmodus um. Der aktuelle Zustand wird über die Onboard-LED (Pin 13) und auf dem Display angezeigt.

6. **Nicht-blockierende Steuerung:**  
   Alle Regelzyklen werden mithilfe von `millis()` gesteuert, sodass das System ohne blockierende `delay()`-Aufrufe arbeitet und stets ansprechbar bleibt.

---

## Installation & Nutzung

1. **Voraussetzungen:**  
   - Arduino IDE oder PlatformIO  
   - Installierte Bibliotheken:  
     - [Adafruit SSD1306](https://github.com/adafruit/Adafruit_SSD1306)  
     - [Adafruit GFX](https://github.com/adafruit/Adafruit-GFX-Library)

2. **Projekt herunterladen:**  
   Klone oder lade dieses Repository herunter und öffne die `Pumpensteuerung.ino` in der Arduino IDE.

3. **Hardware anschließen:**  
   - Verbinde die NTC-Sensoren mit den analogen Eingängen A0 und A1.  
   - Schließe die Pumpen an die PWM-Pins 9 und 10 sowie den Lüfter an Pin 3 an.  
   - Verbinde das OLED-Display mit SDA (A4) und SCL (A5).  
   - Schließe den Benchmark-Taster an Pin 4 und den Hauptschalter an Pin 2 (beide mit internem Pullup) an.  
   - Achte auf eine korrekte Spannungsversorgung und Masseverbindungen.

4. **Code kompilieren & hochladen:**  
   Wähle das richtige Board (Arduino Nano) und den passenden Port, kompilieren und laden den Code hoch.

5. **Betrieb:**  
   - Schalte das System über den Hauptschalter ein.  
   - Beobachte die Messwerte und PWM-Werte auf dem OLED-Display.  
   - Drücke den Benchmark-Taster, um zwischen Normal- und Benchmarkmodus zu wechseln (der Status wird auf der LED und dem Display angezeigt).

---

## Anpassungsmöglichkeiten

- **Temperaturgrenzen und Kennlinien:**  
  Ändere die Werte in den Strukturen `ActuatorHysteresis` und `ActuatorCurve` je nach den gewünschten Schaltpunkten und der PWM-Kurve.

- **Kickstart-Parameter für Pumpen:**  
  Passe die Dauer und den PWM-Wert des Kickstarts in der `Pump`-Klasse an, um den Startvorgang zu optimieren.

- **Displaylayout:**  
  Passe das Layout in der `DisplayManager`-Klasse in `update()` an, um weitere Informationen anzuzeigen oder die Darstellung zu verändern.

- **Speicheroptimierung:**  
  Reduziere Debug-Ausgaben (verwende das `F()`-Makro) und überprüfe, ob die Floating-Point-Operationen (Beta-Gleichung) oder virtuelle Methoden weiter optimiert werden können, falls Flash/RAM knapp werden.

---

## Lizenz

*(Bitte füge hier deine gewünschte Lizenz ein, z.B. MIT, GPL, etc.)*

---

## Kontakt

Falls du Fragen, Vorschläge oder Probleme hast, eröffne bitte ein Issue in diesem Repository oder kontaktiere mich direkt per E-Mail.

---

Viel Erfolg und Spaß beim Experimentieren!
