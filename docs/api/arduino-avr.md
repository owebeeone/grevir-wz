# Arduino AVR API

**Package:** Grevir Arduino AVR. **Header:** `<GrevirArduinoAVR.h>`.
**CMake target:** `grevir::arduino_avr`. **Dependencies:** Arduino, AVR, Core,
Peripherals and their transitive Grevir dependencies.

`ardo::ArduinoAvrApplication<Modules...>` composes modules with the Arduino
millisecond timer reservation. On Uno/Nano ATmega328P, that reservation claims
Timer0. Arduino PWM pins 5 and 6 use Timer0 and therefore conflict with this
application type; pins 9/10 route to Timer1 and 3/11 to Timer2. The adapter
also supplies board pin aliases and ATmega328P PWM pin-to-timer bindings.

```cpp
#include <GrevirArduinoAVR.h>
using Led = ardo::arduino::OutputPin<LED_BUILTIN>;
using App = ardo::ArduinoAvrApplication<ardo::ArduinoParamModule<Led>>;
```

Call `App::runSetup()` from Arduino `setup()` and `App::runLoop()` from
`loop()`. See the [complete Blink sketch](../examples/blink.md).
Uno, Nano and old-bootloader Nano declarations are present; the old-bootloader
variant affects upload configuration, not timer or GPIO silicon behavior.
Selected Uno and Nano sketches compile, but no physical board has been run.
