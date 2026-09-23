# FastLED API

**Package:** Grevir FastLED. **Header:** `<GrevirFastLED.h>`.
**CMake target:** `grevir::fastled`. **Dependencies:** Base, Core and FastLED.

`ardo_fastled::LedStrip<Count, PinClaim, Chipset, ColorOrder, Brightness>`
owns a strip buffer and the FastLED controller. Use `ardo::ExternalPin<N>`
when FastLED configures the physical pin. One strip module claims the
FastLED singleton; two strip modules in one application cause a Core resource
conflict. `get()` returns the stored colour, and brightness changes mark the
strip for an update.

On the selected Uno AVR toolchain (Debian AVR GCC 14.2), install FastLED
3.7.8. Include `<GrevirArduinoAVR.h>` before `<GrevirFastLED.h>` in an
Arduino sketch. Host tests cover controller setup and buffer behavior;
a selected target sketch compiled, but physical WS2812 timing has not been
validated. The [complete Uno sketch](../examples/fastled-avr.md) is available
under `/docs`. The historical ParkLights application is not a supported example.
