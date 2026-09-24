# Support and validation

Grevir's current target path is AVR-first. The portable APIs accept injected
backends. That design does not by itself establish a working backend for every
MCU. Physical hardware behavior has not yet been validated.

| Environment | Current evidence | Scope |
| --- | --- | --- |
| macOS, Apple Clang 21, C++23 | Full native build and 186 CTest cases pass; generated mock interrupt dispatch passes | Host behavior, mock hardware and compiler contracts |
| Raspberry Pi, native GCC, C++23 | Full native build and 186 CTest cases pass; generated mock interrupt dispatch passes | Host behavior and Linux object transport |
| Windows 11, MSVC 19.44, C++23 mode | Full native build, compiler probes and 186 CTest cases pass; generated mock interrupt dispatch passes | Host behavior, native COFF transport and public-header portability |
| ATmega328P, Arduino Uno/Nano, Debian AVR GCC 14.2 | Selected Uno sketches and selected Nano sketches compile with Arduino AVR 1.8.8 and `-std=c++23` | Arduino, PWM/pin, Pulse IO and Packet compositions; individual package coverage varies |
| ATmega328P, simavr 1.6 | Selected timer, GPIO/Pulse IO and Packet probes pass; generated Timer1 overflow handler executes | Simulated behavior only |
| Physical Uno/Nano | Not run | Electrical behavior, real timing and silicon-specific effects remain unvalidated |
| Classic ESP32 Dev Module, Arduino-ESP32 3.3.11 | Parklights compiles and links with selected Grevir GPIO, clock and serial adapters; generated TG0/T0 interrupt example compiles and links | Interrupt routing, physical-board behavior and other ESP32 timer/peripheral backends are not validated |

"Native tested" means production code ran with host mocks or an installed
consumer. "AVR compiled" means a named target program compiled and linked with
the AVR toolchain; it does not cover every template instantiation. "Simulated"
means a named firmware path ran in simavr. "Hardware validated" would require
a named physical board; no feature has that evidence yet.

The selected ATmega328P toolchain has no AVR libstdc++. Grevir Base provides the
compatibility subset used by the selected target programs. Standard-library
availability for a new instantiation must still be checked. Arduino's stock
AVR GCC 7.3 does not accept this project's C++23 flag; the recorded builds use
Debian AVR GCC 14.2. FastLED target examples use FastLED 3.7.8 with that
compiler. Package `architectures=*` metadata describes packaging, not a tested
board list.

The current timer MVP is fixed-frequency synchronous fast PWM on ATmega328P
Timer0/1/2. Arduino's `millis()` use reserves Timer0 in
`ArduinoAvrApplication`, so its PWM pins 5 and 6 are unavailable there. Other
waveforms, device families and broad peripheral allocation are outside this
scope. Packet's selected target proof uses one receiver slot and two fragments;
Pulse IO's selected proof uses an 8-bit collector. See each [package
contract](api/index.md) for additional bounds.
