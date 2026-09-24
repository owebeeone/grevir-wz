# Grevir ESP32-family Xtensa implementation and review policy

Apply together with [the cross-MCU policy](CrossMcu.md). This supplement covers
the classic ESP32 (LX6), ESP32-S2 (LX7) and ESP32-S3 (LX7) Xtensa targets.
The currently implemented adapter is narrower: classic ESP32 Dev Module using
Arduino-ESP32 3.3.11 and `esp32:esp32:esp32`. S2 and S3 are design targets,
not implemented or validated Grevir backends. See the
[capability study](../GrevirEsp32CapabilityStudy.md) for their distinct
inventories and ownership questions. ESP32-C3 and other RISC-V targets need
their own architecture supplement before implementation.

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

For S2/S3 reviews, use that chip's SoC capability header and a named module/board
profile. Do not reuse classic ESP32 GPIO numbers, LEDC high-speed mode or
channel counts, RMT capacities, MCPWM presence, USB ownership or dual-core
assumptions. The shared API contract remains common while candidate inventories
and runtime drivers are target-specific.

ESP32 code may run on multiple FreeRTOS tasks or callbacks. Review ownership and
callback context explicitly before claiming thread safety, interrupt safety,
or deterministic timing. Do not infer an FPU or its precision from a 32-bit CPU;
assess numeric work against the named target and operation. Neither host tests
nor a target compile/link establishes physical behavior. Silicon validation is
held separately.
