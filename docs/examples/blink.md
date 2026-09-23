# Blink on Uno/Nano

Requires Grevir Base, Time, Core, Peripherals, Registers, Arduino, Arduino AVR and AVR. This exact sketch compiled for the selected ATmega328P Arduino toolchain. `ArduinoAvrApplication` reserves Timer0 for Arduino timekeeping. `poll()` uses the Peripherals strict-expiry rule, so a transition occurs after the interval is exceeded.

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

The packaged sketch is `grevir-arduino/examples/Blink/Blink.ino`. See [target support](../supported.md) for the distinction between compilation, simulation and physical validation.
