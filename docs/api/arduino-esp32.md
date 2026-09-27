# Arduino ESP32 API

**Package:** Grevir Arduino ESP32. **Header:** `<GrevirArduinoESP32.h>`.
**CMake target:** `grevir::arduino_esp32`. **Dependencies:** Arduino, Base,
Core, Peripherals and their transitive Grevir dependencies.

This is a narrow Arduino adapter for the classic ESP32 Dev Module. It adds
`ardo::arduino_esp32::ExternalPin<N>` and `OutputPin<N>` with compile-time
checks for exposed and output-capable GPIO numbers, plus
`ardo::arduino_esp32::SerialPort0<Baud>`, which claims UART0 and the selected
variant's default TX/RX GPIO 1/3. The common Arduino package supplies
`ardo::ArduinoMillisClock`; Core supplies `ardo::Application<Modules...>`.

```cpp
#include <GrevirArduinoESP32.h>

using Led = ardo::arduino_esp32::OutputPin<2>;
using Console = ardo::arduino_esp32::SerialPort0<115200>;
using App = ardo::Application<ardo::ArduinoParamModule<Led, Console>>;
```

The GPIO checks exclude flash pins 6–11 and reject output use of input-only
pins 34–39. They do not prove that a chosen pin is safe for attached hardware,
boot strapping or a particular Dev Module clone. The
[classic ESP32 TG0/T0 direct-interrupt example](../examples/interrupt-esp32/interrupt-esp32.ino)
compiles and links through the staged interrupt generator. The adapter does not
provide a general ESP32 timer or peripheral allocator, deferred interrupt
dispatch, or DMA support; the direct example has not run on physical hardware.
The selected
`parklights` application repository compiles for `esp32:esp32:esp32`; no
physical board has been run.
