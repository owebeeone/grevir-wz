# Grevir classic ESP32 implementation and review policy

Apply together with [the cross-MCU policy](CrossMcu.md). This supplement covers
the initial classic ESP32 Xtensa target using the Arduino-ESP32 3.3.11 core and
`esp32:esp32:esp32` board profile. It does not describe other ESP32 variants.

The first supported path is intentionally narrow: Arduino GPIO, millisecond
clock, and serial adapters needed by Parklights. Do not infer timer, interrupt,
DMA, or hardware peripheral support from the `esp32` Arduino architecture label.
Keep the application and its Wi-Fi, display, and OTA dependencies separate from
the reusable Grevir packages.

GPIO capability checks must distinguish exposed pins, output-capable pins,
flash-connected pins, board wiring, and boot strapping. Passing a compile-time
pin check does not prove an attached board's electrical safety. Arduino's runtime
Peripheral Manager is not a substitute for Grevir's compile-time resource
claims.

ESP32 code may run on multiple FreeRTOS tasks or callbacks. Review ownership and
callback context explicitly before claiming thread safety, interrupt safety,
or deterministic timing. Do not infer an FPU or its precision from a 32-bit CPU;
assess numeric work against the named target and operation. Neither host tests
nor a target compile/link establishes physical behavior. Silicon validation is
held separately.
