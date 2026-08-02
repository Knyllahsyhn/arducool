#pragma once
#include <Arduino.h>

// Hardware pin map. Change your wiring in this file only.

constexpr uint8_t PIN_PUMP1 = 9;              // OC1A output, 25 kHz PWM
constexpr uint8_t PIN_PUMP2 = 10;             // OC1B output, 25 kHz PWM
constexpr uint8_t PIN_FAN1 = 3;               // OC2B output, 25 kHz PWM
constexpr uint8_t PIN_BENCHMARK_BUTTON = 4;   // INPUT_PULLUP
constexpr uint8_t PIN_LED = 13;               // Benchmark mode indicator
constexpr uint8_t PIN_SENSOR1 = A0;
constexpr uint8_t PIN_SENSOR2 = A1;
