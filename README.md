# Arduino-Based Pump and Fan Controller (OOP, 25 kHz PWM, Separate Hysteresis & Curves for Normal/Benchmark Mode)

This project uses an Arduino Nano to control two pumps and one fan based on temperature measurements. It features separate **hysteresis** thresholds, **temperature-to-PWM curves** (for normal and benchmark modes), and a **kickstart** mechanism for pumps. The PWM outputs run at **25 kHz** to avoid audible noise, and the code is organized in an **object-oriented** manner for easy extensibility.
Feel free to test it out in the [Wowki](https://wokwi.com/projects/427233338419995649) simulator. 

---

## Features

1. **Two Operating Modes**  
   - **Normal mode**: user-defined on/off thresholds and PWM curves  
   - **Benchmark mode**: steeper curves, different on/off thresholds, higher maximum PWM  
   - Toggled via a push button (with an LED indicator on Pin 13)

2. **Object-Oriented Design**  
   - **Base class** `Actuator` handles:  
     - Two **hysteresis** sets (on/off temperature)  
     - Two **curves** (normal/benchmark)  
     - General on/off switching and linear mapping from temperature to PWM  
   - **Pump** inherits from `Actuator` and adds a **kickstart** period (to ensure reliable spin-up at low PWM values)  
   - **Fan** inherits from `Actuator` (with no kickstart, but with separate hysteresis and curves if desired)

3. **NTC Temperature Sensors**  
   - Beta-coefficient calculation for precise °C reading  
   - **EMA smoothing** to reduce noise

4. **Non-blocking Loop**  
   - Uses `millis()`-based timing instead of `delay()`, allowing other tasks to run concurrently

5. **25 kHz PWM** on Pins 9, 10, and 3  
   - Eliminates annoying audible frequencies  
   - Implemented via timer register manipulation and a helper function `setPWM_25kHz()`

6. **Modular Files**  
   - `Timers.*`: Timer setup for 25 kHz PWM  
   - `Sensor.*`: Beta-NTC reading & exponential smoothing  
   - `Actuator.*`: Base class with hysteresis, separate normal/benchmark curves  
   - `Pump.*` & `Fan.*`: Specializations for pumps (kickstart) and fans
7. **Output on 128x64 SSD1306 OLED Display**
   - Displays temperatures of `sensor1` and `sensor2`, PWM Values of `pump1`, `pump2` and `fan1`, `Benchmark Mode (BM)`

---


## Hardware Setup

1. **Arduino Nano** (5V, ATmega328p).  
2. **10 kΩ NTC sensors** (Beta ~3950) in voltage divider circuits, analog inputs (`A0`, `A1`).  
3. **PWM outputs** (25 kHz):  
   - **Pump1** → Pin 9 (OC1A)  
   - **Pump2** → Pin 10 (OC1B)  
   - **Fan** → Pin 3 (OC2B)   
4. **Benchmark button** on Pin 4 (`INPUT_PULLUP`); toggles the second hysteresis and curve set.  
5. **Onboard LED** (Pin 13) as an indicator for benchmark mode.  
6. **MOSFET drivers** for pumps and fan.
7. **SSD1306** 128x64 OLED Display (SCL - A5, SDA - A4)

![Untitled Sketch_Schaltplan](https://github.com/user-attachments/assets/3d93944c-8488-4729-bf87-16d31e0ad469)
[![IMG-4341.jpg](https://i.postimg.cc/4xChdyvJ/IMG-4341.jpg)](https://postimg.cc/JGqhY1dg)
---

## Workflow / Logic

1. **Timer Configuration**  
   - `initTimers25kHz()` sets Timer1 (Pins 9,10) and Timer2 (Pin 3) to ~25 kHz instead of default Arduino PWM frequencies.

2. **Sensor**  
   - `Sensor` class reads ADC, converts to Celsius via the Beta equation and smooths the result with exponential moving average.

3. **`Actuator` Base Class**  
   - Manages two sets of **hysteresis** (normal/benchmark) and two temperature->PWM curves.  
   - `update(bool benchmarkMode)` checks if the actuator should switch on or off, maps temperature linearly to PWM, then calls `postProcessPWM()`.

4. **`Pump`** (Inherits from `Actuator`)  
   - Implements a **kickstart** in `postProcessPWM()`, forcing a high PWM for a set duration to ensure the pump starts spinning reliably.

5. **`Fan`** (Inherits from `Actuator`)  
   - Shares the same base hysteresis/curve mechanism but typically has no kickstart.

6. **Benchmark Mode**  
   - Toggled by a button.  
   - Uses different on/off thresholds and steeper PWM curves (e.g., higher maximum PWM).  
   - LED on Pin 13 indicates whether benchmark mode is active.

7. **Non-blocking Loop**  
   - The main `loop()` uses `millis()` checks to call `update()` for sensors and actuators at fixed intervals (e.g., every 500 ms).  
   - No calls to `delay()` are used so the system remains responsive.

8. **Memory Optimization**  
   - Large `float` usage from Beta-equation calls can be replaced with simpler approximations if desired.  
   - Debug strings can be stored in Flash using the `F()` macro to save RAM.

---

## Quick Example (Main Sketch Snippet)

```cpp
void loop() {
  
  unsigned long now = millis();
  if ((now - lastControlTime) >= controlInterval) {
    lastControlTime = now;
    handleBenchmarkButton();
    if(now - lastDisplayTime >= displayInterval) {
    lastDisplayTime = now;
    displayMgr.update(); // Schreibt Sensor/Pump/Fan-Werte aufs OLED
  }


    sensor1.update();
    sensor2.update();

    pump1.update(benchmarkMode);
    pump2.update(benchmarkMode);
    fan1.update(benchmarkMode);
}
```
---
## Installation
1. Clone or download this repo with all .h/.cpp files.

2. Open PumpController.ino in Arduino IDE or use PlatformIO.

3. Select "Arduino Nano" and the proper COM port in the IDE.

4. Compile and upload.

5. Wire your hardware as specified (sensors, MOSFET drivers, etc.).

6. Power up and watch the system control the pumps and fan according to the temperatures.

---
## License

GNU GPLv3
---
## Contact / Contributions
Feel free to open an Issue or Pull Request if you have questions or improvements.

For precise temperature readings, ensure your Beta constant matches your NTC specs.

If you need more available memory, see the notes on disabling log() or simplifying the code.
