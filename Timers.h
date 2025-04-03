#pragma once
#include <Arduino.h>

/**
 * Initialisiert Timer1 (Pins 9,10) und Timer2 (Pin 3) auf ~25 kHz.
 */
void initTimers25kHz();

/**
 * Setzt an Pin 9, 10 oder 3 einen PWM-Wert (0..255).
 * Intern wird auf 0..79 gemappt und ins passende OCR-Register geschrieben.
 */
void setPWM_25kHz(uint8_t pin, int value);
