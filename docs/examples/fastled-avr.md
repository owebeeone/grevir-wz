# FastLED strip on Uno

Requires Arduino AVR, Grevir FastLED, their Grevir dependencies and FastLED 3.7.8 with the selected Debian AVR GCC 14.2 toolchain. This exact sketch compiled for Uno. It fills eight LEDs blue during setup and shows the buffer from the module loop. Physical LED timing and color output have not been validated.

```cpp
#include <GrevirArduinoAVR.h>
#include <GrevirFastLED.h>

using Strip = ardo_fastled::LedStrip<8, ardo::ExternalPin<6>, WS2812B, GRB, 16>;

class FillBlue : public ardo::ModuleBase<ardo::Parameters<>> {
public:
  static void runSetup() {
    for (unsigned i = 0; i < Strip::COUNT; ++i) {
      Strip::set(i, CRGB::Blue);
    }
  }
};

using App = ardo::ArduinoAvrApplication<Strip, FillBlue>;

void setup() {
  App::runSetup();
}

void loop() {
  App::runLoop();
}
```

The packaged source is `grevir-fastled/examples/StripOn/StripOn.ino`. See [support](../supported.md) for evidence levels.
