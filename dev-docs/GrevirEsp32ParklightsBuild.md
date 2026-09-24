# Parklights ESP32 build checkpoint

Parklights is now a separate `parklights` GWZ member. Its port uses the common
Grevir Base, Time, Core, Peripherals, Arduino, Packet and FastLED packages, plus
the new, narrow `grevir-arduino-esp32` package. The source-only legacy copy
remains under `grevir-fastled/extras/legacy/examples/ParkLightsV2` as migration
reference. The new adapter is for the classic ESP32 Dev Module only; it supplies
GPIO capability checks and Serial0 setup/claims. No raw ESP32 timer, interrupt,
DMA or peripheral backend has been implemented.

## Named build

The selected target is `esp32:esp32:esp32` with Arduino-ESP32 3.3.11 on
weftpi (Linux aarch64). The core supplies `esp-x32` 2601. The build used
FastLED 3.7.8, Adafruit ST7735 and ST7789 Library 1.11.0, Adafruit GFX
Library 1.12.6 and their automatically installed dependencies. It compiled
with `compiler.cpp.extra_flags=-std=c++23` from local Grevir source copies,
using `parklights/build-esp32.sh`. No private credentials header was supplied.

The initial full compile and link passed. It reported 1,021,475 bytes of
program storage (77% of the selected 1,310,720-byte partition) and 79,384
bytes of global RAM (24% of 327,680 bytes). Subsequent edits tightened UART0
GPIO claims, braced inherited OTA branches and limited the adapter header to
the selected board profile; the final target compile passed with the same
reported flash and RAM use.
The macOS native workspace build and 186 CTest cases passed; an independently
installed `grevir-arduino-esp32` CMake consumer also compiled and ran after
package sources were removed.

## Published checkout reproduction

On 24 September 2026, a separate `gwz clone` on weftpi materialized all 17
members from GitHub at workspace commit `6e561be` (Parklights `2758fec`,
Arduino ESP32 adapter `c5e65e3`). Running `parklights/build-esp32.sh` with the
installed Arduino CLI against this cold checkout compiled and linked without a
private credentials header. The result matched the initial build: 1,021,475
bytes of flash (77%) and 79,384 bytes of global RAM (24%). This verifies the
published workspace and package dependencies for the named target; it does not
exercise the device.

This is compile/link evidence only. Wi-Fi, UDP, FastLED output, display,
OTA, task interactions, boot behavior and physical GPIO wiring have not been
exercised on silicon. The empty credentials fallback exists so a clean
source checkout can compile without a private file; it is not a usable network
configuration. Physical validation remains on hold.
