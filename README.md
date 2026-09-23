# Grevir

Grevir is a set of C++23 embedded libraries for declaring an application's
modules, dependencies, resource claims and hardware requirements as types.
Application assembly checks conflicts at compile time. Portable modules receive
pins, clocks and device bindings from the selected backend rather than importing
one MCU's registers.

The currently documented platform paths are:

| Platform | Current state |
| --- | --- |
| Arduino Uno/Nano, ATmega328P | Selected sketches compile with Debian AVR GCC 14.2; selected firmware passes in simavr. Physical-board behavior is unvalidated. |
| macOS and Windows 11 | Full native test suites pass with Apple Clang and MSVC. |
| ESP32 | Portable declarations can contain ESP32-specific options; a Grevir ESP32 backend is not implemented. |

See the exact [support and evidence levels](docs/supported.md) before choosing a
target.

## Declarative composition

This complete Uno/Nano sketch declares an LED pin, a two-period poller and an
application. Grevir initializes the module through `App::runSetup()` and runs it
through `App::runLoop()`:

```cpp
#include <GrevirArduinoAVR.h>

using Led = ardo::arduino::OutputPin<LED_BUILTIN>;
using Seq = ardo::Sequence<unsigned long, 500UL, 500UL>;
using Blinker = ardo::CyclicTimeSequencePoller<Seq, ardo::ArduinoMillisClock>;

class BlinkModule : public ardo::ModuleBase<ardo::Parameters<Led>> {
public:
  static void runSetup() {
    Led::runSetup();
    poller.init();
  }

  static void runLoop() {
    if (poller.poll()) {
      on = !on;
      Led::set(on);
    }
  }

  inline static Blinker poller{};
  inline static bool on = false;
};

using App = ardo::ArduinoAvrApplication<BlinkModule>;

void setup() {
  App::runSetup();
}

void loop() {
  App::runLoop();
}
```

The [annotated example](docs/examples/blink.md) explains the timing and Arduino
Timer0 reservation. A [portable PWM declaration](docs/guides/pwm.md) shows how
common requirements and target-specific sections coexist; only the resident
target's section is applied.

## Libraries

The packages cover foundations and typed time; module composition and resource
claims; GPIO, registers and ATmega328P timers; pulse encoding and GPIO I/O;
packet fragmentation; quadrature decoding and stepper control; plus Arduino AVR
and FastLED adapters. The [package index](docs/api/index.md) gives the exact
header, dependency and target scope of each library.

Start at the [public documentation index](docs/index.md) for installation,
concepts, guides, examples and API contracts.
