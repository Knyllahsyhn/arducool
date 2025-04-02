/****************************************************
 * Beispiel-Sketch (25 kHz PWM, non-blocking, geglättete Messwerte)
 * mit vereinfachter Pumpe2-Logik:
 *  - Ein wenn T2 > 20°C
 *  - Aus wenn T2 < 15°C
 *  - Wenn an, läuft Pumpe2 über eine Temperaturkurve ähnlich wie Pumpe1
 ****************************************************/

// Pins für Sensoren
const int sensor1Pin = A0; 
const int sensor2Pin = A1; 

// PWM-Ausgänge (25kHz-tauglich)
const int pump1Pin = 9;   // Timer1, OC1A
const int pump2Pin = 10;  // Timer1, OC1B
const int fanPin   = 3;   // Timer2, OC2B

// Schalter-Pins
const int benchmarkSwitchPin = 4; // (3 ist PWM-Ausgang)

 // Konstanten an deinen NTC anpassen:
  const float B = 3950.0;        // Beta-Koeffizient deines NTC
  const float T0 = 298.15;       // 25°C in Kelvin
  const float R0 = 10000.0;      // 10k NTC @ 25°C
  const float R_fixed = 10000.0; // der feste Widerstand in deinem Teiler
  const float Vcc = 5.0;

// PWM-Bereiche (Logik-Ebene 0..255, später auf 0..79 skaliert)
const int PUMP_OFF_PWM        = 0;
const int PUMP_MIN_PWM_NORMAL = 85;   // ~33% (von 255)
const int PUMP_MAX_PWM_NORMAL = 128;  // ~50%
const int PUMP_MAX_PWM_BENCH  = 230;  // ~90%

const int FAN_OFF_PWM         = 0;
const int FAN_MIN_PWM          = 114;   // ~45% von 255 
const int FAN_MAX_PWM         = 255;

// Temperaturschwellen Pumpe1-Kurve
const float T_PUMP_START_NORMAL = 20.0; 
const float T_PUMP_END_NORMAL   = 35.0;

// Temperaturschwellen Lüfter-Kurve
const float T_FAN_START         = 25.0; 
const float T_FAN_END           = 40.0;

// Pumpe2 Hysterese-Grenzen
const float T2_ON  = 20.0; // Schaltet EIN oberhalb 20°C
const float T2_OFF = 15.0; // Schaltet AUS unterhalb 15°C

// Globale Variable: Hysterese-Zustand von Pumpe2
bool pump2IsOn = false;

//Fan-Start
bool fanOn = false;

// Für geglättete Temperaturen
float T1_filtered = 0.0;
float T2_filtered = 0.0;

// Glättungsfaktor für Exponential Moving Average
const float alpha = 0.1; 

// Non-Blocking Zeittakt
unsigned long lastControlTime = 0;
const unsigned long controlInterval = 500; // z.B. alle 500ms regeln

/****************************************************
 * Timer-Init für 25 kHz (unverändert)
 ****************************************************/


void setup() {
  Serial.begin(9600);
  pinMode(pump1Pin, OUTPUT);
  pinMode(pump2Pin, OUTPUT);
  pinMode(fanPin,   OUTPUT);
  pinMode(benchmarkSwitchPin, INPUT_PULLUP);

  // 25 kHz PWM init
  initTimers25kHz();

  // initial auf 0
  setPump1PWM(0);
  setPump2PWM(0);
  setFanPWM(0);

  // Einmal initial messen und filtern
  T1_filtered = readTemperatureSensor(sensor1Pin);
  T2_filtered = readTemperatureSensor(sensor2Pin);
}

void loop() {
  // ** NON-BLOCKING ZEITSTEUERUNG **
  unsigned long currentMillis = millis();
  if ((currentMillis - lastControlTime) >= controlInterval) {
    lastControlTime = currentMillis;


    // --- Benchmarkmodus ---
    bool benchmarkMode = (digitalRead(benchmarkSwitchPin) == LOW);

    // --- Temperaturen lesen + glätten ---
    
    float T1_raw = readNTCTemp_Beta(sensor1Pin);
    float T2_raw = readNTCTemp_Beta(sensor2Pin);
    //float T1_raw = readTemperatureSensor(sensor1Pin);
    //float T2_raw = readTemperatureSensor(sensor2Pin);

    // Exponential Moving Average
    T1_filtered = alpha * T1_raw + (1 - alpha) * T1_filtered;
    T2_filtered = alpha * T2_raw + (1 - alpha) * T2_filtered;

    // Debug
    Serial.print("T1=");
    Serial.print(T1_filtered);
    Serial.print(" | T2=");
    Serial.print(T2_filtered);
    Serial.print(" | Benchmark=");
    Serial.println(benchmarkMode);
    

    // *** PUMPE1: wie bisher ***
    int pump1PWMVal = 0;
    if (benchmarkMode) {
      // 25..60°C -> 33%..90%
      pump1PWMVal = mapTemperatureToPWM(T1_filtered,
                                        T_PUMP_START_NORMAL,
                                        T_PUMP_END_NORMAL,
                                        PUMP_MIN_PWM_NORMAL,
                                        PUMP_MAX_PWM_BENCH);
    } else {
      // 25..60°C -> 33%..50%
      pump1PWMVal = mapTemperatureToPWM(T1_filtered,
                                        T_PUMP_START_NORMAL,
                                        T_PUMP_END_NORMAL,
                                        PUMP_MIN_PWM_NORMAL,
                                        PUMP_MAX_PWM_NORMAL);
    }

    // *** PUMPE2: vereinfachte Hysterese + Kurve ***
    int pump2PWMVal = 0;

    // Hysterese-Ein/Aus
    if (pump2IsOn) {
      // Aus schalten, wenn T2 < 15°C
      if (T2_filtered < T2_OFF) {
        pump2IsOn = false;
      }
    } else {
      // Ein schalten, wenn T2 > 20°C
      if (T2_filtered > T2_ON) {
        pump2IsOn = true;
      }
    }

    if (pump2IsOn) {
      // Pumpenkurve wie Pumpe1, aber nach T2
      if (benchmarkMode) {
        // steilere Kurve: 25..60°C => 33..90%
        pump2PWMVal = mapTemperatureToPWM(T2_filtered,
                                          T_PUMP_START_NORMAL,
                                          T_PUMP_END_NORMAL,
                                          PUMP_MIN_PWM_NORMAL,
                                          PUMP_MAX_PWM_BENCH);
      } else {
        // normale Kurve: 25..60°C => 33..50%
        pump2PWMVal = mapTemperatureToPWM(T2_filtered,
                                          T_PUMP_START_NORMAL,
                                          T_PUMP_END_NORMAL,
                                          PUMP_MIN_PWM_NORMAL,
                                          PUMP_MAX_PWM_NORMAL);
      }
    } else {
      pump2PWMVal = PUMP_OFF_PWM;
    }

    // *** LÜFTER: läuft wie gehabt über T2 ***
    int fanPWMVal = 0;
    if (T2_filtered < T_FAN_START) {
      fanOn = false;
    }
    else if (T2_filtered >= T_FAN_START) {
      fanOn = true;
    }

    if (fanOn) {
      fanPWMVal = mapTemperatureToPWM(T2_filtered,
                                        T_FAN_START, T_FAN_END,
                                        FAN_MIN_PWM, FAN_MAX_PWM);
    }
  else {
    fanPWMVal=FAN_OFF_PWM;
  }
   


    // (In Benchmarkmodus könntest du hier auch eine andere Kennlinie hinterlegen)

    // --- PWM setzen ---
    setPump1PWM(pump1PWMVal);
    setPump2PWM(pump2PWMVal);
    setFanPWM(fanPWMVal);
    Serial.print("fanPWMVal=");
    Serial.print(fanPWMVal);
    Serial.print(" | pump1PWMVal=");
    Serial.print(pump1PWMVal);
    Serial.print(" | pump2PWMVal=");
    Serial.println(pump2PWMVal);
  }

  // Hier kann noch weiteres stattfinden, ohne blockierende Delays
}

void initTimers25kHz() {
  // --- TIMER 1 (Pins 9 und 10) ---
  TCCR1A = 0; 
  TCCR1B = 0;
  // Fast PWM, Modus 14 (WGM13=1, WGM12=1, WGM11=1), ICR1=TOP
  TCCR1A |= (1 << WGM11);
  TCCR1B |= (1 << WGM13) | (1 << WGM12);
  // Non-inverting auf OC1A und OC1B
  TCCR1A |= (1 << COM1A1) | (1 << COM1B1);
  // Prescaler=8
  TCCR1B |= (1 << CS11);
  // TOP=79 -> ~25 kHz
  ICR1 = 79;
  // Duty anfangs 0
  OCR1A = 0;
  OCR1B = 0;

  // --- TIMER 2 (Pin 3) ---
  TCCR2A = 0;
  TCCR2B = 0;
  // Fast PWM mit OCR2A als TOP
  TCCR2A |= (1 << WGM21) | (1 << WGM20); 
  TCCR2B |= (1 << WGM22);
  // Non-inverting OC2B
  TCCR2A |= (1 << COM2B1);
  // Prescaler=8
  TCCR2B |= (1 << CS21);
  // TOP=79
  OCR2A = 79;
  // Duty anfangs 0
  OCR2B = 0;
}

/****************************************************
 * Direktes Setzen der Duty Cycles (0..255 → 0..79)
 ****************************************************/
void setPump1PWM(int pwmValue) {
  if (pwmValue < 0)   pwmValue = 0;
  if (pwmValue > 255) pwmValue = 255;
  int duty = map(pwmValue, 0, 255, 0, 79);
  OCR1A = duty;
}

void setPump2PWM(int pwmValue) {
  if (pwmValue < 0)   pwmValue = 0;
  if (pwmValue > 255) pwmValue = 255;
  int duty = map(pwmValue, 0, 255, 0, 79);
  OCR1B = duty;
}

void setFanPWM(int pwmValue) {
  if (pwmValue < 0)   pwmValue = 0;
  if (pwmValue > 255) pwmValue = 255;
  int duty = map(pwmValue, 0, 255, 0, 79);
  OCR2B = duty;
}

/****************************************************
 * Dummy: analogRead -> Temperatur (0..1023 -> 0..100°C)
 ****************************************************/
float readTemperatureSensor(int pin) {
  int raw = analogRead(pin);
  float temp = (1.0 - raw/1023.0) * 100.0;
  return temp;
}

/****************************************************
 * mapTemperatureToPWM: lineare Kennlinie
 ****************************************************/
int mapTemperatureToPWM(float temperature, float T_low, float T_high,
                        int pwmMin, int pwmMax) {
  if (temperature <= T_low) {
    return pwmMin;
  } else if (temperature >= T_high) {
    return pwmMax;
  } else {
    float ratio = (temperature - T_low) / (T_high - T_low);
    int pwmVal  = pwmMin + (int)(ratio * (pwmMax - pwmMin));
    return pwmVal;
  }
}
float readNTCTemp_Beta(int pin) {
  
  int raw = analogRead(pin);
  // ADC -> Spannung
  float V_pin = (raw / 1023.0) * Vcc;

  // Berechne den NTC-Widerstand
  // R_ntc = R_fixed * (V_pin / (Vcc - V_pin))
  // Vorsicht auf Division durch 0, falls V_pin= Vcc oder 0
  if (V_pin <= 0.0 || V_pin >= (Vcc - 0.0001)) {
    // Unrealistisch: NTC "unendlich" oder "0" Ohm
    // return z.B. -100.0 als Fehlercode
    return -100.0;
  }
  float R_ntc = R_fixed * (V_pin / (Vcc - V_pin));

  // Beta-Formel: T(Kelvin) = 1 / (1/T0 + (1/B)*ln(R_ntc / R0))
  float invT = (1.0 / T0) + (1.0 / B) * log(R_ntc / R0);
  float T_kelvin = 1.0 / invT;
  float T_celsius = T_kelvin - 273.15;
  return T_celsius;
}
