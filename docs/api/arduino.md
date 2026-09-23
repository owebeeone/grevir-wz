# Arduino API

**Package:** Grevir Arduino. **Header:** `<GrevirArduino.h>`.
**CMake target:** `grevir::arduino`. **Dependencies:** Base, Time, Core and
Peripherals.

This adapter supplies Arduino `CoreIF`, pin aliases, Serial and PWM bridges.
`ardo::arduino::InputPin<N>` and `OutputPin<N>` are convenient board-facing pin
types for an Arduino sketch. `ardo::ArduinoMillisClock` binds the typed clock
used by the [Blink example](../examples/blink.md). EEPROM support is in a
separate header and is not pulled in by `GrevirArduino.h`.

An Arduino build includes the platform's `Arduino.h`. Host mock builds select
`GREVIR_ARDUINO_HOST_MOCK` explicitly; absence of Arduino macros does not
silently select a mock in production. Arduino's own timer and pin reservations
depend on the chosen board adapter. For Uno/Nano use [Arduino AVR](arduino-avr.md).
This common package alone does not choose an MCU backend.
