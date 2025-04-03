# Arduino-basierte Pumpen- und Lüftersteuerung mit 25 kHz PWM

Dieses Projekt steuert mithilfe eines Arduino Nano und MOSFETs zwei Pumpen und eine Lüftergruppe basierend auf Temperaturmessungen. Die Temperaturmessung erfolgt über zwei 10k-NTC-Sensor (im Spannungsteiler) und die Umrechnung auf °C über die Beta-Formel. Die PWM-Ausgänge sind auf eine Frequenz von etwa 25 kHz eingestellt, um störende Pfeifgeräusche zu minimieren. Außerdem wird auf blockierendes `delay()` verzichtet und stattdessen eine nicht-blockierende Zeitsteuerung mittels `millis()` verwendet.

---

## Features

- Mosfets für beispielsweise **Zwei Pumpen** (Pumpe1 & Pumpe2) und **eine Lüftergruppe**, jeweils mit PWM geregelt.
- **25 kHz PWM** auf den Ausgängen (Pins 9, 10 und 3), umgesetzt über direkte Register-Manipulation (Timer1 & Timer2).
- **Temperaturmessung** mittels 10k NTC und Beta-Formel zur Umrechnung in °C.
- **Geglättete Messwerte** (Exponential Moving Average) für stabileres Regelverhalten.
- **Nicht-blockierende** Zeitsteuerung mithilfe von `millis()`.
- **Benchmark-Modus** (via Schalter): steilere PWM-Kennlinien und höheres Maximum.
- **Hysterese** beim Ein- und Ausschalten (z.B. Pumpe2: EIN >20 °C, AUS <15 °C).
- **Einfach anpassbare** Temperaturgrenzen und PWM-Bereiche im Code.

---

## Hardware

1. **Arduino Nano** oder kompatibles Board (ATmega328p).
2. **NTC-Sensor** (10 kΩ, Beta z.B. 3950) in einem Spannungsteiler mit einem festen 10-kΩ-Widerstand.
3. **MOSFETs** oder geeignete Treiberstufe (z.B. IRLZ44N), um spannungsgeregelte Pumpen und Lüfter mit PWM anzusteuern.  
4. **Pumpen** (2x) und **Lüfter** (1x) entsprechend deiner Anwendung.
5.**Benchmark-Schalter** (einfacher Taster/Schalter gegen GND mit Pullup).

### Verschaltung (Beispiel)

- **Sensor1** an A0 (NTC-Spannungsteiler).
- **Sensor2** an A1 (NTC-Spannungsteiler).
- **Pumpe1** (MOSFET-Gate) an Pin 9 (OC1A).
- **Pumpe2** (MOSFET-Gate) an Pin 10 (OC1B).
- **Lüfter** (MOSFET-Gate) an Pin 3 (OC2B).
- **Benchmark-Schalter** an Pin 4 (digital, INPUT_PULLUP).
- Gemeinsame Masse aller Komponenten.

---

## Software / Code

### Installation

1. **Arduino IDE** oder PlatformIO installieren.  
2. Projektdateien (z.B. `Pumpen_Luefter_Steuerung.ino`) in ein Verzeichnis legen.  
3. Gegebenenfalls die **NTC-Konstante** (Beta, R0) und den festen Widerstandwert in der `readNTCTemp_Beta()`-Funktion anpassen.  
4. Sicherstellen, dass der Code für einen **Arduino Nano** (ATmega328p) kompiliert wird (Board-Einstellungen).

### Wichtige Code-Bereiche

- **`initTimers25kHz()`**: Setzt Timer1 und Timer2 so, dass die PWM-Ausgänge mit ~25 kHz laufen.  
- **`readNTCTemp_Beta()`** (oder ähnlich): Implementiert die Beta-Formel für den NTC.  
- **`mapTemperatureToPWM()`**: Wandelt die gemessene Temperatur in einen PWM-Wert um (0..255), der später auf 0..79 skaliert wird, da der Timer im Fast-PWM-Modus mit `ICR1=79` bzw. `OCR2A=79` arbeitet.  
- **`loop()`**: Enthält die nicht-blockierende Zeitsteuerung mit `millis()`. Alle `controlInterval` Millisekunden wird die Temperatur gemessen, gefiltert und anschließend die PWM-Werte für Pumpen und Lüfter berechnet.  
- **Hysterese**: Für Pumpe2 wird beispielsweise `pump2IsOn` genutzt, um nur bei Überschreiten einer bestimmten Temperatur ein- und bei Unterschreiten wieder auszuschalten.

---

## Anpassungen

1. **Temperaturschwellen** (`T_PUMP_START_NORMAL`, `T_FAN_START`, etc.) lassen sich frei wählen.  
2. **PWM-Min/Max** kann man im Code (`PUMP_MIN_PWM_NORMAL`, `PUMP_MAX_PWM_NORMAL`, etc.) anpassen, je nachdem, welche Leistung benötigt wird.  
3. **Beta-Parameter** des NTC (z.B. 3950) und der Widerstandswert `R_fixed` (z.B. 10 kΩ) müssen übereinstimmen.  
4. **Glättungsfaktor** `alpha` bestimmt, wie stark die Messwerte geglättet werden. Niedriger Wert = sehr träge, hoher Wert = weniger Filterung.

---

## Bekannte Einschränkungen

- Auflösung der PWM ist auf **0..79** (entspricht ~6,3 Bit) reduziert, da wir für 25 kHz einen kleinen TOP-Wert gewählt haben. Für die meisten Anwendungen ist das ausreichend.  
- Die Beta-Formel ist genauer als eine simple Linear-Skalierung, aber für perfekte Genauigkeit wäre eine **Steinhart-Hart**-Gleichung noch präziser.  
- Nur **Pin 3** (OC2B) kann für Timer2 bei 25 kHz frei geregelt werden. **Pin 11** ist in diesem Modus belegt als „TOP“-Register-Ausgang und steht nicht für Duty-Cycle zur Verfügung.

---

## Verwendung / Workflow

1. **Sketch** per Arduino IDE / PlatformIO kompilieren und auf den Nano hochladen.  
2. **Hardware** korrekt verdrahten (NTC-Spannungsteiler, MOSFETs, Schalter, Pumpe, Lüfter).  
3. **Hauptschalter** einschalten → Arduino beginnt zu messen und regeln.  
4. **Benchmarkmodus** testen, indem du den entsprechenden Schalter betätigst. Dies erhöht die PWM-Obergrenze für beide Pumpen und kann die Lüfterkennlinie anpassen (je nach Code).  
5. **Temperatur simulieren** (z.B. Fühler erwärmen), um die Regelung zu testen. Mit dem seriellen Monitor kannst du Debug-Meldungen über `Serial.print()` auslesen (Temperature, PWM-Werte etc.).

---

##To do

1. Integration eines I2C-Displays und Ausgabe relevanter Werte
2. Vernünftige Branching-Strategie anlegen
3. Code aufräumen
4. Schönen Schaltplan basteln

## Lizenz und Haftungsausschluss

GNU GPLv3


Diese Schaltung und Software ist ein Beispielprojekt. Für Schäden durch unsachgemäßen Gebrauch oder abweichende Hardwarekonfigurationen wird keine Haftung übernommen. Bitte teste die Schaltung sorgfältig und stelle sicher, dass die MOSFETs, Spannungsversorgung und Verdrahtung für deine Lasten geeignet sind.

---

## Kontakt

Fragen, Vorschläge oder Probleme? Erstelle bitte ein [GitHub Issue](https://github.com/Knyllahsyhn/arducool/issues) oder melde dich direkt per E-Mail.  

Viel Spaß beim Bauen und Experimentieren!


