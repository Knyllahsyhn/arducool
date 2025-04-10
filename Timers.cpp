#include "Timers.h"

/*************************************************
 * initTimers25kHz()
 *   - Timer1 -> Pins 9,10
 *   - Timer2 -> Pin 3
 *   - TOP=79, Prescaler=8 => ~25kHz
 *************************************************/
void initTimers25kHz() {
  // -- TIMER1 (Pins 9=OC1A, 10=OC1B) --
  TCCR1A = 0;
  TCCR1B = 0;
  // Fast PWM, Mode 14: ICR1=TOP
  TCCR1A |= (1 << WGM11);
  TCCR1B |= (1 << WGM13) | (1 << WGM12);
  // Non-inverting auf OC1A / OC1B
  TCCR1A |= (1 << COM1A1) | (1 << COM1B1);
  // Prescaler=8
  TCCR1B |= (1 << CS11);
  // TOP=79
  ICR1 = 79;

  // -- TIMER2 (Pin 3=OC2B) --
  TCCR2A = 0;
  TCCR2B = 0;
  // Fast PWM, TOP=OCR2A
  TCCR2A |= (1 << WGM21) | (1 << WGM20);
  TCCR2B |= (1 << WGM22);
  // Non-inverting auf OC2B
  TCCR2A |= (1 << COM2B1);
  // Prescaler=8
  TCCR2B |= (1 << CS21);
  // TOP=79
  OCR2A = 79;
}

/*************************************************
 * setPWM_25kHz(pin, value 0..255)
 *   - mapped auf 0..79
 *   - schreibt OCR1A, OCR1B oder OCR2B
 *************************************************/
void setPWM_25kHz(uint8_t pin, int value) {
  // Begrenzen
  if (value < 0)   value = 0;
  if (value > 100) value = 100;
  int duty = map(value, 0, 100, 0, 79);

  switch(pin) {
    case 9:
      OCR1A = duty; 
      break;
    case 10:
      OCR1B = duty;
      break;
    case 3:
      OCR2B = duty;
      break;
      default:
      break;
  }
}
