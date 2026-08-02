#pragma once
#include <Arduino.h>

// Set up Timer1 (pins 9, 10) and Timer2 (pin 3) for 25 kHz PWM.
void initTimers25kHz();

// Set the PWM duty cycle on pin 9, 10, or 3.
// value: 0 to 100 (percent). The function maps this to the timer's
// 0-250 duty range.
void setPWM_25kHz(uint8_t pin, int value);
