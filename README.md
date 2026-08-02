# Arduino Pump and Fan Controller for a 24 V PSU

This project controls two pumps and one fan with an Arduino Nano.
The controller reads two NTC temperature sensors and sets the PWM
output for each pump and the fan. The PWM signal runs at 25 kHz, so
the pumps and the fan do not make an audible whine. The code uses
two sets of settings: normal mode and benchmark mode. A button on
pin 4 switches between the two modes.

Try the [Wokwi simulation](https://wokwi.com/projects/427233338419995649).
Note: the PWM output does not work correctly in the simulator. The
code works correctly on real Arduino Nano hardware.

---

## Features

1. **Two operating modes**
   - Normal mode: user-defined on/off temperatures and PWM curves.
   - Benchmark mode: steeper curves, different on/off temperatures,
     and a higher maximum PWM value.
   - A button toggles the mode. The LED on pin 13 shows the current
     mode: on for benchmark mode, off for normal mode.

2. **Object-oriented design**
   - The `Actuator` base class manages two hysteresis sets (on/off
     temperature) and two temperature-to-PWM curves.
   - `Pump` extends `Actuator` and adds a kickstart period, to start
     the pump motor reliably at a low PWM value.
   - `Fan` extends `Actuator` with no kickstart.

3. **NTC temperature sensors and fail-safe behavior**
   - Each `Sensor` object converts a raw ADC reading to a
     temperature with the Beta formula.
   - An exponential moving average (EMA) smooths the reading.
   - If a sensor reports an invalid raw value (an open or a shorted
     circuit), the sensor keeps its last valid temperature and sets
     a fault flag.
   - The matching pump or fan then runs at 100% PWM until the sensor
     reports a valid reading again. This fail-safe assumes the worst
     case (overheating) when the controller cannot trust a sensor.

4. **Non-blocking main loop**
   - The loop uses `millis()` to time each task. The code does not
     call `delay()`, so all tasks stay responsive.

5. **25 kHz PWM on pins 9, 10, and 3**
   - `Timers.cpp` sets Timer1 and Timer2 to 25 kHz with direct
     register access.
   - `setPWM_25kHz()` sets the duty cycle for a given pin.

6. **Central pin configuration**
   - `Pins.h` lists every pin the sketch uses. Change your wiring in
     this one file.

7. **Modular files**
   - `Pins.h`: pin assignment.
   - `Timers.*`: 25 kHz PWM setup.
   - `Sensor.*`: NTC reading, EMA smoothing, and fault detection.
   - `Actuator.*`: base class for hysteresis, curves, and fail-safe
     behavior.
   - `Pump.*` and `Fan.*`: pump kickstart and fan-specific behavior.
   - `DisplayManager.*`: OLED output.

8. **128x64 SSD1306 OLED output**
   - The display shows the temperature (or `ERR` on a sensor fault)
     for sensor 1 and sensor 2, the PWM value for pump 1, pump 2,
     and the fan, and the current mode (`BM: ON` or `BM: OFF`).

---

## Hardware Setup

1. Arduino Nano (5 V, ATmega328P).
2. 24 V power supply. Note: we tested 12 V first and found it too
   weak for our pumps, so we moved to 24 V.
3. Two 10 kΩ NTC sensors (Beta ≈ 3950) in a voltage divider circuit,
   on analog pins `A0` and `A1`.
4. PWM outputs at 25 kHz:
   - Pump 1 → pin 9 (OC1A)
   - Pump 2 → pin 10 (OC1B)
   - Fan → pin 3 (OC2B)
5. Benchmark button on pin 4 (`INPUT_PULLUP`). The button toggles
   the hysteresis and curve set.
6. Onboard LED (pin 13) as the benchmark mode indicator.
7. N-channel MOSFET drivers for the pumps and the fan.
8. 128x64 SSD1306 OLED display (SCL → A5, SDA → A4).

![Schematic](https://github.com/user-attachments/assets/a0198f37-dbb2-4c98-975d-91f54a14b9a9)
[![Build photo](https://i.postimg.cc/4xChdyvJ/IMG-4341.jpg)](https://postimg.cc/JGqhY1dg)

The MOSFETs set the power limit for each channel. We used IRLZ44N
MOSFETs, which give more headroom than these pumps need.

---

## Workflow and Logic

1. **Timer configuration**: `initTimers25kHz()` sets Timer1 (pins 9,
   10) and Timer2 (pin 3) to 25 kHz.
2. **Sensor reading**: `Sensor::update()` reads the ADC, converts
   the value to Celsius with the Beta formula, and smooths the
   result with an EMA filter. If the raw ADC value is 0 or 1023,
   the sensor flags a fault instead of computing a new temperature.
3. **`Actuator` base class**: `update(bool benchmarkMode)` first
   checks the sensor fault flag. On a fault, the actuator runs at
   100% PWM. Otherwise, it checks the hysteresis, maps the
   temperature to a PWM value with the active curve, and calls
   `postProcessPWM()`.
4. **`Pump`**: `postProcessPWM()` forces a high PWM value for the
   kickstart duration, so the motor starts reliably.
5. **`Fan`**: uses the base class behavior with no kickstart.
6. **Benchmark mode**: the button toggles `benchmarkMode`. This
   switches the active hysteresis and curve set and sets the LED on
   pin 13.
7. **Non-blocking loop**: `loop()` checks `millis()` against fixed
   intervals to run each task. The code does not call `delay()`.

---

## Quick Example (Main Sketch Snippet)

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

1. Clone or download this repository with all `.h` and `.cpp` files.
2. Open `arducool.ino` in the Arduino IDE, or use PlatformIO.
3. Select "Arduino Nano" and the correct COM port.
4. Compile and upload the sketch.
5. Wire the hardware as described above (sensors, MOSFET drivers,
   and so on).
6. Power up the board. The system now controls the pumps and the
   fan from the sensor temperatures.

---

## Memory Notes

The Beta formula uses `log()` from `<math.h>`, which uses flash and
RAM. If you need more free RAM, replace the Beta formula with a
lookup table. Debug output already uses the `F()` macro, to store
strings in flash instead of RAM.

---

## License

GNU GPLv3

---

## Contact and Contributions

Open an issue or a pull request for questions or improvements.

Match the Beta constant to your NTC datasheet for accurate readings.
