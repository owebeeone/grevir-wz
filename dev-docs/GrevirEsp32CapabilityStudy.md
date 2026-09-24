# ESP32 capability and ownership study

## Decision and scope

Consider the full ESP32-family Xtensa set—classic ESP32 (LX6), ESP32-S2
(single-core LX7) and ESP32-S3 (dual-core LX7)—when defining the backend
contract. The classic ESP32 Dev Module (`esp32:esp32:esp32`) remains the first
**implementation and Parklights build target**, not a template whose
capabilities are copied to S2/S3. ESP32-C3 is a later RISC-V cross-architecture
check. ESP8266 uses a Tensilica L106 but a different SDK/Arduino core; account
for it as a separate possible backend, not as an ESP32 variant. Arduino-ESP32
3.3.11 is based on ESP-IDF 5.5.5; the linked IDF 5.5 documentation and pinned
SoC headers below define this study's compatibility window. A named board and
module configuration remain necessary before a chip profile is deployable.
[Chip-series comparison](https://docs.espressif.com/projects/esp-idf/en/v5.0/esp32/hw-reference/chip-series-comparison.html),
[Arduino-ESP32 release](https://github.com/espressif/arduino-esp32/releases/tag/3.3.11),
[ESP8266 SDK](https://docs.espressif.com/projects/esp8266-rtos-sdk/en/latest/get-started/)

Keep the common declarative request and deterministic planning contract in
Grevir Core/Peripherals. Put chip capabilities, realizability and application
behind target backends, with module/board reservations in the Arduino binding.
Do **not** derive a universal `TimerN` register API from the AVR model. ESP32
LEDC PWM timers, Timer Group/GPTimer, RMT, and MCPWM have different resource
graphs and ownership rules. The compiler's SoC constants can seed a capability
inventory; they cannot by themselves establish usable board pins, routing,
clock sharing, driver ownership or callback context.

There are two distinct **PWM generator** families to consider: LEDC provides
ordinary duty/frequency PWM, while MCPWM adds timer/operator/comparator/
generator resources for synchronized or complementary outputs, dead time,
fault handling and capture. Timer Group is a third, general-purpose counter
and alarm peripheral; GPTimer is its IDF driver, not an alternate LEDC/MCPWM
PWM block. MCPWM is present on classic ESP32 and S3, but not in the pinned S2
capability header. These families need separate backend models even if common
requests can be realized by more than one family.
[MCPWM](https://docs.espressif.com/projects/esp-idf/en/v5.5/esp32/api-reference/peripherals/mcpwm.html),
[GPTimer](https://docs.espressif.com/projects/esp-idf/en/v5.5/esp32/api-reference/peripherals/gptimer.html)

This is a study and design direction, not evidence of an implemented ESP32
timer backend. [Current PWM integration](GrevirPwmIntegration.md) implements
ATmega328P Timer0/1/2 only. [Parklights build](GrevirEsp32ParklightsBuild.md)
establishes compile/link on the named classic ESP32, not physical behavior.
Physical validation remains on hold.

## Xtensa chip matrix

The counts below come from the `soc_caps.h` files bundled with the selected
Arduino-ESP32 3.3.11 installation on weftpi, cross-checked against the
[ESP-IDF 5.5.5 ESP32](https://github.com/espressif/esp-idf/blob/v5.5.5/components/soc/esp32/include/soc/soc_caps.h),
[S2](https://github.com/espressif/esp-idf/blob/v5.5.5/components/soc/esp32s2/include/soc/soc_caps.h)
and [S3](https://github.com/espressif/esp-idf/blob/v5.5.5/components/soc/esp32s3/include/soc/soc_caps.h)
headers. They are **SoC capacities**, not counts of resources free to Grevir.

| Chip | CPU | LEDC | RMT TX capacity | Distinct constraints |
| --- | --- | --- | --- | --- |
| Classic ESP32 | LX6; two cores in the selected SoC build, with single-core product variants | 8 high-speed + 8 low-speed channels; 4 timers per bank; 20-bit timer width | 8 TX-capable channels | GPIO34–39 input-only; strap pins 0/2/5/12/15; flash/PSRAM and board wiring can reserve more pads. MCPWM present. [GPIO](https://docs.espressif.com/projects/esp-idf/en/v5.5/esp32/api-reference/peripherals/gpio.html), [LEDC](https://docs.espressif.com/projects/esp-idf/en/v5.5/esp32/api-reference/peripherals/ledc.html) |
| ESP32-S2 | One LX7 core | 8 low-speed channels; 4 timers; 14-bit timer width | 4 TX-capable channels | GPIO46 input-only; strap pins 0/45/46; flash/PSRAM typically uses 26–32; USB PHY pins need explicit policy. No MCPWM in the pinned SoC capability header. [GPIO](https://docs.espressif.com/projects/esp-idf/en/v5.5/esp32s2/api-reference/peripherals/gpio.html), [LEDC](https://docs.espressif.com/projects/esp-idf/en/v5.5/esp32s2/api-reference/peripherals/ledc.html) |
| ESP32-S3 | Two LX7 cores | 8 low-speed channels; 4 timers; 14-bit timer width; **one shared clock source across all timers** | 4 TX-capable channels among 8 RMT channels; RMT DMA capability | Strap pins 0/3/45/46; flash/PSRAM uses 26–32 and, with octal memory, 33–37; GPIO19/20 default to USB-JTAG. MCPWM present. [GPIO](https://docs.espressif.com/projects/esp-idf/en/v5.5/esp32s3/api-reference/peripherals/gpio.html), [LEDC](https://docs.espressif.com/projects/esp-idf/en/v5.5/esp32s3/api-reference/peripherals/ledc.html) |

S2 and S3 both have low-speed-only LEDC; S2 still has some timer-specific
clock-source choices, while S3 requires a single LEDC clock source across its
timers. The channel and clock differences invalidate a shared fixed-size
`ESP32 timer` inventory. The RMT capacities and S3 DMA capability also make
FastLED ownership a chip-specific question.
[S2 LEDC](https://docs.espressif.com/projects/esp-idf/en/v5.5/esp32s2/api-reference/peripherals/ledc.html),
[S3 LEDC](https://docs.espressif.com/projects/esp-idf/en/v5.5/esp32s3/api-reference/peripherals/ledc.html),
[S2 RMT](https://docs.espressif.com/projects/esp-idf/en/v5.5/esp32s2/api-reference/peripherals/rmt.html),
[S3 RMT](https://docs.espressif.com/projects/esp-idf/en/v5.5/esp32s3/api-reference/peripherals/rmt.html)

## Capabilities that change the API

| Concern | Across the three Xtensa chips | Design consequence |
| --- | --- | --- |
| GPIO | All have routing through a GPIO matrix, but each has different pads, strap/flash/PSRAM restrictions and USB/JTAG reservations. Module memory and board wiring can remove further pins. | Separate silicon capability, module wiring, board exposure and application reservations. A compilable pin claim does not prove boot or electrical safety. Use a named chip **and** board inventory. |
| LEDC fixed-frequency PWM | Different bank/channel counts, timer widths and clock-sharing rules; frequency/resolution depends on the selected clock. | Model bank/timer/channel/pad and shared clock constraints. Candidate validation must handle group-wide restrictions, not only parent/child overlap. |
| General timing | The GPTimer driver allocates an opaque handle from a timer pool and can report exhaustion on the ESP32-family targets. | Keep general-purpose alarm/counter timing separate from LEDC PWM. The public allocator cannot promise a particular physical timer through this API without further evidence. [GPTimer](https://docs.espressif.com/projects/esp-idf/en/v5.5/esp32/api-reference/peripherals/gptimer.html) |
| Timed symbol output | RMT resources and DMA support differ, and the current IDF driver returns a handle from a pool. | Treat addressable LEDs and other pulse protocols as RMT use, not PWM timer use. Opaque allocation is runtime acquisition with an exhaustion path. [RMT](https://docs.espressif.com/projects/esp-idf/en/v5.5/esp32/api-reference/peripherals/rmt.html) |
| Motor PWM and interrupts | MCPWM is present on classic ESP32 and S3, absent from the pinned S2 SoC capability header. Core count and callback context differ. | Defer MCPWM and interrupt abstraction until a use case justifies them. Do not flatten MCPWM into LEDC or promise ISR safety from a host test. [MCPWM](https://docs.espressif.com/projects/esp-idf/en/v5.5/esp32/api-reference/peripherals/mcpwm.html), [interrupt allocation](https://docs.espressif.com/projects/esp-idf/en/v5.5/esp32/api-reference/system/intr_alloc.html) |

ESP32-C3 still provides a valuable *later* cross-architecture test: it has a
single RISC-V core, six low-speed LEDC channels, and one LEDC clock source
shared by all timers. It is not one of the Xtensa targets requested here.
[C3 GPIO](https://docs.espressif.com/projects/esp-idf/en/v5.5/esp32c3/api-reference/peripherals/gpio.html),
[C3 LEDC](https://docs.espressif.com/projects/esp-idf/en/v5.5/esp32c3/api-reference/peripherals/ledc.html)

ESP8266 is also considered because it is Tensilica-based, but neither
Arduino-ESP32 3.3.11 nor the ESP-IDF 5.5.5 SoC headers describe it. Its
L106/ESP8266 SDK and separate Arduino core require a separate capability and
toolchain feasibility study before promising any Grevir peripheral support.
The common high-level request contract can be reused only where its semantics
are realizable; an ESP32 LEDC candidate inventory cannot be reused there.
[ESP8266 SDK](https://docs.espressif.com/projects/esp8266-rtos-sdk/en/latest/get-started/),
[Arduino-ESP32 supported SoCs](https://docs.espressif.com/projects/arduino-esp32/en/latest/getting_started.html)

The LEDC frequency/resolution combination must be realizable for its selected
clock. High-speed and low-speed timer update semantics differ; only the
classic ESP32 has the high-speed bank in this Xtensa set. Duty endpoint
handling also needs a target-aware rule: IDF warns that setting exactly
`2^resolution` at maximum resolution overflows the duty counter on all three
Xtensa chips. These are backend validation and update semantics, not new
generic meanings for `PwmRequest`.
[ESP32 LEDC](https://docs.espressif.com/projects/esp-idf/en/v5.5/esp32/api-reference/peripherals/ledc.html),
[S2 LEDC](https://docs.espressif.com/projects/esp-idf/en/v5.5/esp32s2/api-reference/peripherals/ledc.html),
[S3 LEDC](https://docs.espressif.com/projects/esp-idf/en/v5.5/esp32s3/api-reference/peripherals/ledc.html)

## The actual Parklights ownership boundary

The present [Parklights application](../parklights/ParkLightsV2/parklights.h)
claims SPI pins 5/18/19/23, TFT reset/DC pins 4/21, FastLED data pin 27,
power pin 32 and network blink pin 2. It also uses the Arduino SPI/display
libraries, FastLED, Wi-Fi, UDP sockets and OTA. GPIO2 and GPIO5 are classic
ESP32 strapping pins; their current use needs board/boot validation, not a
blanket compile-time rejection. The narrow
[Arduino ESP32 adapter](../grevir-arduino-esp32/src/grevir/arduino_esp32/pins.hpp)
checks general pin capability; it does not reserve the SPI host, RMT engine,
interrupts or external library state.

The selected FastLED 3.7.8 source takes its ESP32 RMT path unless
`FASTLED_ESP32_I2S` is set; Parklights' build sets no such option. Under IDF 5,
the FastLED 3.7.8 `idf5_clockless_rmt_esp32.h` includes its older RMT
implementation. Its default `FASTLED_RMT_BUILTIN_DRIVER` is false, and that
implementation configures RMT channels and uses its own interrupt handling.
This is a concrete **integration constraint**, not proof of a present conflict:
Grevir must not allocate or reconfigure RMT independently while FastLED owns it.
Changing FastLED backend/driver choice changes the reservation model.
[FastLED backend selection](https://github.com/FastLED/FastLED/blob/3.7.8/src/platforms/esp/32/fastled_esp32.h),
[IDF 5 shim](https://github.com/FastLED/FastLED/blob/3.7.8/src/platforms/esp/32/idf5_clockless_rmt_esp32.h),
[legacy-driver default](https://github.com/FastLED/FastLED/blob/3.7.8/src/platforms/esp/32/idf4_clockless_rmt_esp32.h),
[RMT controller](https://github.com/FastLED/FastLED/blob/3.7.8/src/platforms/esp/32/idf4_rmt_impl.cpp)

Arduino-ESP32 3.3.11's `ledcAttachChannel(pin, freq, resolution, channel)`
accepts a named channel **but chooses a matching or free timer at runtime**.
`ledcAttach` chooses the channel too. It also interacts with Arduino's
Peripheral Manager for pin ownership. These calls cannot apply a Grevir plan
that names a physical LEDC timer. The chosen Grevir direction is to bypass
both Arduino and IDF **LEDC software drivers**, while using the LEDC hardware
directly. The Arduino calls remain relevant as a source of coexistence risk,
not as an implementation path for Grevir.
[Arduino LEDC implementation](https://github.com/espressif/arduino-esp32/blob/3.3.11/cores/esp32/esp32-hal-ledc.c),
[Arduino Peripheral Manager](https://github.com/espressif/arduino-esp32/blob/3.3.11/cores/esp32/esp32-hal-periman.c)

## Direct LEDC hardware backend

Grevir should translate a deterministic common PWM allocation into explicit
LEDC bank, timer, channel, clock and GPIO-matrix settings through its own
target-specific register driver. Chip-specific SoC register and bitfield
definitions from the pinned core are source facts; IDF's LEDC driver is an
implementation reference, not a function Grevir calls. This does **not** mean
discarding every IDF system service: shared clock-tree, power and GPIO routing
services may still be necessary. Their exact use must be established for each
chip without accidentally acquiring a second LEDC software owner.
[IDF LEDC driver source](https://github.com/espressif/esp-idf/blob/v5.5.5/components/esp_driver_ledc/src/ledc.c)

The backend must own peripheral clock/reset setup, feasible clock/divider and
duty computation, timer/channel configuration, GPIO routing, low-speed update
latching, and any synchronization needed for writes from multiple tasks or
cores. It also needs an explicit startup/shutdown and sleep policy. Start with
fixed-frequency PWM and duty updates; fading, interrupts and sleep retention
are outside that first contract. The existing typed register layer needs an
ESP32 32-bit MMIO access-policy audit before reuse: volatile access alone does
not make read-modify-write atomic or encode write-one-to-clear and update
strobe semantics.
[Register access layer](../grevir-registers/src/grevir/registers/access.hpp),
[IDF LEDC behavior](https://docs.espressif.com/projects/esp-idf/en/v5.5/esp32/api-reference/peripherals/ledc.html)

Grevir must have **exclusive ownership of every LEDC bank, timer and channel
it configures**. An Arduino `ledc*`/`analogWrite` call, IDF LEDC driver call,
or outside library controlling those resources is incompatible unless a
specific coordination adapter is designed and proved. Raw register writes do
not update Arduino's Peripheral Manager. Treat the PWM pad as reserved in the
application inventory and check whether any external library touches LEDC;
physical binding is only guaranteed inside that ownership boundary. Fail on
a detected ownership conflict rather than silently choosing different
hardware; raw register access cannot reliably detect an uncoordinated outside
writer, so application reservations and integration review remain required.

## Deterministic allocation: precise promise

The user requirement is that reordering module declarations must leave
assignments unchanged. Preserve the existing stable request identities and
canonical candidate ordering. For compile-time-owned resources, the named
chip/board inventory and reservations should produce the same plan independent
of declaration order. Distinct request IDs remain mandatory.
[Timer allocation design](GrevirTimerAllocationDesign.md)

There are **three different guarantees**:

1. **Deterministic plan:** Grevir maps the same set of requests to the same
   logical candidates, or rejects it, regardless of declaration order.
2. **Deterministic physical binding:** the backend configures the planned
   physical timer/channel/pad. This is possible when Grevir owns the required
   hardware and shared platform state is coordinated.
3. **Deterministic availability:** no outside Arduino, IDF or library code can
   claim a resource before Grevir. This cannot be inferred from a compile-time
   plan in a mixed-library application.

For LEDC, direct register control can bind named physical timers and channels
once ownership and clock/GPIO setup are settled. GPTimer and current RMT
drivers return pool-allocated handles, so a deterministic logical assignment
does not by itself guarantee a stable physical instance. For Parklights, mark
FastLED's RMT domain and the TFT's SPI/pads as **external owners** until their
actual allocations are brought under a Grevir-controlled adapter. Report
runtime acquisition errors; never silently remap a compile-time assignment.

The first resource graph should distinguish at least physical GPIO pads,
LEDC bank/timer/channel, SPI host, and RMT TX ownership. Edges describe routing
and dependencies; shared settings such as LEDC clock and frequency need explicit
compatibility checks. Board reservations and external owners are input to the
plan. The current PWM allocator already covers stable IDs, candidates and
exclusive parent/child overlap; it does **not** yet cover every ESP32 group
constraint or Arduino runtime owner.

## Recommended bounded implementation sequence

1. Define **three chip profiles** from the pinned SoC capability headers:
   classic ESP32, S2 and S3. Model each one's LEDC bank/timer/channel counts,
   clock-sharing rule, GPIO restrictions, RMT TX capacity and MCPWM presence.
   Keep the current classic ESP32 Dev Module profile as a separate board
   binding; collect exact S2/S3 module and board identities before defining
   their exposure and flash/PSRAM reservations. Do not copy Parklights' pin
   map into those profiles.
2. Add explicit external reservations for Parklights' FastLED RMT use and TFT
   SPI use. First establish which SPI host and FastLED RMT channels the pinned
   libraries really use; the current source proves RMT use but not a complete
   stable runtime channel assignment.
3. Run the **same common PWM requests** through classic ESP32, S2 and S3
   synthetic capability inventories before freezing the Grevir LEDC hardware
   backend contract.
   Include a request that succeeds on classic but fails on S2/S3, and one
   S3 shared-clock conflict; assert declaration-order-independent results.
   These are design/model checks, not S2/S3 support claims.
4. Implement one classic ESP32 **LEDC fixed-frequency PWM** path against the
   existing common request/allocation contract. Keep its setup policy small:
   selected bank, timer, channel, clock, GPIO and duty range. Audit the register
   access policy and prove the direct-driver ownership and shared clock/GPIO
   integration before claiming physical determinism. Add host register-model
   checks and a named target compile/link check; then extend the backend to S2
   and S3 using each one's own board/module profile and target build. Silicon
   behavior remains under the existing validation hold.
5. Use ESP32-C3 as a later RISC-V design check. Treat ESP8266 as a separate
   feasibility decision. Defer GPTimer, RMT abstraction, MCPWM, DMA and generic
   interrupt APIs until their own use cases and ownership contracts are
   specified.

An LEDC proof advances cross-MCU PWM but is **not** required to make current
Parklights compile; that already works. Parklights' nearer runtime risk is the
external SPI/RMT/boot-pin integration, which remains unvalidated on hardware.

## Simulator feasibility and evidence limits

| Route | Useful now | Missing for Parklights |
| --- | --- | --- |
| Native host fixtures | Deterministic allocator, capability validation, adapter failure paths and mock service ownership on macOS/Linux/Windows | No CPU instruction timing, boot straps, signal routing or real peripheral/driver races. |
| Espressif QEMU fork | Classic ESP32 and S3 CPU boot, serial, interrupt matrix and Timer Groups; classic ESP32 LEDC is listed. The matrix does not list S2 as a target. | No Wi-Fi, RMT, general SPI or GPIO matrix/IO MUX; S3 LEDC is not listed. Cannot validate the Parklights LED/TFT/network path. [QEMU guide](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-guides/tools/qemu.html), [feature matrix](https://github.com/espressif/esp-toolchain-docs/blob/main/qemu/README.md) |
| Wokwi | Lists classic ESP32, S2 and S3 boards; documented GPIO/SPI/LEDC/Wi-Fi and transmit-only RMT make a **reduced Parklights experiment** plausible | RMT is partial; MCPWM absent; exact FastLED 3.7.8 behavior, ST7735 model, OTA flow and timing fidelity remain to be proved. Local-network inbound access requires its private gateway; do not use real credentials in a public simulation. [ESP32 simulator](https://docs.wokwi.com/guides/esp32), [Wi-Fi/gateway](https://docs.wokwi.com/guides/esp32-wifi) |

No simulator was run for this study. The table is a documented capability
assessment, not validation results. A Wokwi proof should first boot a small
firmware with one LED strip, one display if supported, and mock/network packets;
only then attempt the full Parklights image. Neither simulator replaces the
deferred physical validation.
