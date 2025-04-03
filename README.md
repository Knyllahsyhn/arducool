# Arduino-basierte Pumpen- und Lüftersteuerung (OOP & 25 kHz PWM)

In diesem Projekt wird ein Arduino Nano (oder ein vergleichbarer ATmega328p‐basiertes Board) verwendet, um zwei Pumpen und einen Lüfter auf Basis von Temperaturmessungen zu steuern. Die Steuerung ist **objektorientiert** aufgebaut, verwendet **25 kHz PWM** (um Pfeifgeräusche zu vermeiden) und arbeitet nicht‐blockierend über `millis()` statt `delay()`.

**Highlights**:

- **Zwei Pumpen** mit Ein/Aus‐Hysterese und Kickstart (damit sie sicher anlaufen)  
- **Lüfter** mit linearer Temperatur‐PWM‐Kennlinie  
- **NTC-Temperatursensoren** (10k) mit **Beta‐Formel** und Exponential‐Glättung  
- **Benchmarkmodus** per Taster, um steilere Kennlinien und höhere PWM‐Maxima zu aktivieren  
- **Objektorientierte** Struktur (Klassen `Pump`, `Fan`, `Sensor`, plus `Timers`‐Modul)  
- **Aufteilung** in mehrere `.h/.cpp`‐Dateien für bessere Wartbarkeit  
- Keine blockierenden `delay()`‐Aufrufe; stattdessen Zeitsteuerung per `millis()`  

---

## Inhaltsverzeichnis

1. [Projektstruktur](#projektstruktur)  
2. [Hardware-Setup](#hardware-setup)  
3. [Funktionsweise](#funktionsweise)  
4. [Installation / Kompilierung](#installation--kompilierung)  
5. [Anpassungen](#anpassungen)  
6. [Lizenz](#lizenz)

---

## Projektstruktur

//kommt noch



> **Hinweis**: Im Sketch werden die Klassen **`Sensor`**, **`Pump`** und **`Fan`** instanziert und im Haupt‐Loop via `update()` aufgerufen.

---

## Hardware-Setup

- **Arduino Nano** (ATmega328p), 5V.  
- **Temperatursensor**: 10k NTC in einem Spannungsteiler mit 10k Festwiderstand. Die analogen Eingänge (`A0`, `A1`) messen die Spannung über dem NTC.  
- **MOSFET‐Treiber** (oder geeignete Schaltstufe) an den PWM-Pins:  
  - **Pumpe1** an **Pin 9** (OC1A)  
  - **Pumpe2** an **Pin 10** (OC1B)  
  - **Lüfter** an **Pin 3** (OC2B)  
  - Alle drei Pins liefern ~25 kHz PWM.  
- **Hauptschalter** an **Pin 2** (digital, `INPUT_PULLUP`), schaltet das System ein/aus.  
- **Benchmark‐Taster** an **Pin 4** (digital, `INPUT_PULLUP`), toggelt den Modus bei jedem Druck.  
- **Onboard-LED** (Pin 13) als Indikator für den Benchmarkmodus (einfaches `digitalWrite(13, HIGH/LOW)`).

![image](https://github.com/user-attachments/assets/fc299ae5-d2fb-4789-9afa-03acfe1a1893)


---

## Funktionsweise

1. **Timer-Konfiguration**  
   - In `Timers.cpp` wird **Timer1** (für Pins 9 & 10) und **Timer2** (für Pin 3) auf ~25 kHz eingestellt, anstatt der Standard-Arduino-Frequenz (490/980 Hz).  
   - Eine Hilfsfunktion `setPWM_25kHz(pin, value)` nimmt 0..255 entgegen und skaliert intern auf 0..79, um in den Compare-Registern die Duty Cycle einzustellen.

2. **NTC-Sensor (Beta‐Formel + Glättung)**  
   - Klasse `Sensor` in `Sensor.cpp`.  
   - Konvertiert `analogRead()` mithilfe der Beta‐Gleichung in °C.  
   - Exponential Moving Average verhindert Sprünge und Rauschen.

3. **Pumpen (Hysterese & Kickstart)**  
   - Klasse `Pump` in `Pump.cpp`.  
   - Jede Pumpe hat:  
     - **Ein-/Ausschalt-Grenzen** (`onThreshold`, `offThreshold`).  
     - **Kickstart**: Bei Einschalten wird kurzzeitig ein hoher PWM-Wert (z.B. 100%) angelegt, um sicher anzulaufen.  
     - Danach eine **lineare Temperatur‐zu‐PWM‐Kurve** (z.B. 25..60 °C → 33..50% oder bis 90% im Benchmarkmodus).

4. **Lüfter**  
   - Klasse `Fan` in `Fan.cpp`.  
   - Einfacher: 25..60 °C → 0..255 PWM (oder beliebige andere Kennlinie).

5. **Benchmarkmodus**  
   - Per Taster toggelbar.  
   - Erhöht z.B. bei den Pumpen das PWM‐Maximum (z.B. von 50% auf 90%).  
   - Onboard-LED (Pin 13) zeigt an, ob Benchmarkmodus **aktiv** ist.

---

## Installation / Kompilierung

1. **Arduino IDE** (oder PlatformIO) installieren.  
2. Dieses Repository (mit allen `.h`/`.cpp`‐Dateien) herunterladen.  
3. Im Arduino IDE die **Datei `Pumpensteuerung.ino`** öffnen.  
4. **Board** auf "Arduino Nano" einstellen (ATmega328P).  
5. **Kompliliere** und **lade hoch**.  

**Wichtig**:  
- Code erwartet **5V** Betrieb und `analogRead()` als Spannungsmessung für den NTC. Bei 3.3V‐Systemen müssen ggf. die Formeln und Parameter angepasst werden.  
-  In `Sensor.cpp` die Beta‐Konstante und Widerstandswerte (`R_FIXED`, `R0`) fü konkreten NTC anpassen. 

---

## Anpassungen

1. **Temperaturschwellen**  
   - `onThreshold` / `offThreshold` pro Pumpe in deren Konstruktor.  
   - `mapTemperature(...)` in `Pump.cpp` oder `Fan.cpp` kann Start/End‐Temperaturen und PWM-Min/Max anpassen.

2. **Kickstart**  
   - Dauer und Kickstart‐PWM sind Konstruktor‐Parameter der `Pump`.Für eigene Hardware anpassen. 

3. **Benchmarkmodus**  
   - Aktuell werden in `Pump` nur andere Obergrenzen (z.B. 230 statt 128) verwendet. Andere Kennlinien möglich.

4. **Frequenz**  
   - Für 25 kHz verwenden wir `TOP=79` und Prescaler=8. Bei höherer PWM‐Auflösung (z.B. TOP=255) sinkt die Frequenz entsprechend.

5. **Mehr Pumpen**  
   - Einfach weitere `Pump`‐Objekte (z.B. `pump3`) anlegen, Sensor zuweisen, Pin angeben, im Loop aufrufen.

6. **Struktur**  
   - Bei Bedarf jede Klasse (`Sensor`, `Pump`, `Fan`) weiter verfeinern (z.B. Vererbung), falls z.B. gemeinsame Basisklassen für Aktoren gewünscht. 

---

## Lizenz

GNU GPLv3


---

## Kontakt

Bei Fragen oder Anmerkungen:  
- **Issues** erstellen 

Viel Erfolg und Spaß beim Experimentieren!  


