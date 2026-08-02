#include "Timers.h"
#include "Pins.h"

// initTimers25kHz()
//   - Timer1 drives pins 9 and 10.
//   - Timer2 drives pin 3.
//   - TOP = 250, prescaler = 1: this gives about 25 kHz (phase-correct PWM).
void initTimers25kHz() {
  // -- TIMER1 (pin 9 = OC1A, pin 10 = OC1B) --
  TCCR1A = 0;
  TCCR1B = 0;
  TCCR1B |= (1 << WGM13);                     // Phase-correct PWM, TOP = ICR1
  TCCR1A |= (1 << COM1A1) | (1 << COM1B1);    // Non-inverting on OC1A / OC1B
  TCCR1B |= (1 << CS10);                       // Prescaler = 1
  ICR1 = 250;                                  // TOP

  // -- TIMER2 (pin 3 = OC2B) --
  TCCR2A = 0;
  TCCR2B = 0;
  TCCR2A |= (1 << WGM20);                      // Phase-correct PWM, TOP = OCR2A
  TCCR2B |= (1 << WGM22);
  TCCR2A |= (1 << COM2B1);                     // Non-inverting on OC2B
  TCCR2B |= (1 << CS20);                       // Prescaler = 1
  OCR2A = 250;                                 // TOP
}

// setPWM_25kHz(pin, value)
//   value: 0 to 100 (percent).
void setPWM_25kHz(uint8_t pin, int value) {
  if (value < 0)   value = 0;
  if (value > 100) value = 100;
  int duty = map(value, 0, 100, 0, 250);   // 250 = timer TOP, see initTimers25kHz()

  switch (pin) {
    case PIN_PUMP1:
      OCR1A = duty;
      break;
    case PIN_PUMP2:
      OCR1B = duty;
      break;
    case PIN_FAN1:
      OCR2B = duty;
      break;
    default:
      break;
  }
}
