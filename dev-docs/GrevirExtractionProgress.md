# Grevir extraction progress

Latest checkpoint: 23 September 2026. Fifteen local members now exist: Base,
Time, Core, Peripherals, Registers, AVR, Pulse Codec, Pulse IO, Packet, Encoder, Stepper,
Arduino, Arduino AVR, FastLED, and development-only Test Support. AVR compiler,
simavr and Arduino CLI validation run on weftpi (`gianni@10.1.1.236`). Silicon
hardware remains on hold.

## Packet AVR target checkpoint — 23 September 2026

Packet now builds without AVR libstdc++ through Grevir Base's compatibility
headers. On weftpi, the checked-in Packet sketch compiles for both Uno and Nano
at **1,896 program bytes and 85 RAM bytes**. A separate one-slot, two-fragment
firmware loopback passes under simavr in **6,223 simulated cycles**, with one
correct delivery. The installed CMake consumer passes with Base discovered as a
transitive dependency. macOS and Win11 native suites remain at **186/186**.
[Packet AVR evidence](GrevirPacketAvrEvidence.md) records source revisions,
artifacts and limits. Physical hardware remains on hold.

## Win11 native validation and AVR recheck — 23 September 2026

On dabeest, Visual Studio 2022 MSVC builds the workspace's host runtime suite
and all **186** CTest cases pass, including Packet and Pulse IO. The follow-up
MSVC build also passes the public-header/compile-only targets and all configured
positive and expected-rejection probes. Its CMake probe scripts select MSVC
command-line flags and diagnostic wording; the C++23 cases are shared. The
run exposed and led to fixes for a Base type-trait
redeclaration, an MSVC PWM claim-template parse error, and Pulse IO waveform
static-initialization order. A compile-only encoder test was also corrected to
compare its protected enum inside the derived helper. See
[Win11 evidence](GrevirWindowsValidation.md).
After these changes, the weftpi Uno sketch compiles again at 2,624 flash/78 RAM
bytes; simavr again decodes the second loopback frame, now at 30,153 cycles.
The updated [AVR evidence](GrevirPulseIoAvrEvidence.md) records exact provenance.
Physical hardware validation remains on hold.

## Pulse IO AVR target validation — 23 September 2026

On weftpi, the selected ATmega328P Pulse IO Arduino sketch compiles and links
with Debian `avr-g++` 14.2 in C++23 mode without libstdc++. Arduino CLI reports
2,854 flash bytes and 86 RAM bytes. The simavr D5-to-D4 loopback decodes its
second frame as byte `1` after 33 output changes and 30,281 simulated cycles.
The exact source revision, commands, artifact hash and limits are recorded in
[Pulse IO AVR evidence](GrevirPulseIoAvrEvidence.md). Physical timing and
silicon behavior remain unvalidated.

## Pulse IO and native build repair — 23 September 2026

`grevir-pulse-io` now connects the existing pulse encoder/decoder to injected
GPIO pins and a clock through Core modules. The receiver's readiness method
returns the decoder result, correcting the missing return in Ardoinus. Core pin
claims reject a receiver and transmitter assigned the same GPIO. The original
Gammil repeater prototype is archived byte-for-byte. Two mock tests transfer
normal and inverted frames; an installed consumer builds and runs without Catch2.
The three planned Pulse IO mappings are now recorded: two native-checked adapted
files and one retained legacy archive.

The standalone timer-clock compiler probe now reads the AVR compile target's
dependency include directories. Its valid and seven invalid cases pass. The
full all-target native build passes, as do all **186** CTest cases. The ledger has
126 extraction/archive records (119 native-checked, three recorded target
compiles, four archives); original source hashes remain unchanged. Pulse IO
had native evidence only at this earlier checkpoint; the target result is above.

## Recordkeeping reconciliation — 23 September 2026

The ledger now matches the existing Arduino, Arduino AVR and FastLED working-tree
files: 21 previously missing records added, including 15 native-checked header
assignments, three target-compile records and three byte-identical legacy archives.
Eighteen stale destination hashes from the committed AVR compatibility work were
refreshed. All 736 original-source hashes remain unchanged; all 123 recorded
destination hashes match. Of these records, 117 are native-checked, three have
recorded target compilation, and three are archives, not compiled runtime code.
The three newest package implementations were uncommitted at audit and are
included in the subsequent GitHub synchronization checkpoint.

weftpi is reachable at `gianni@10.1.1.236`; the work is in
`/home/gianni/git/grevir-wz`. Read-only comparison found 178 matching files and no
differences within the compared set. Six retained Arduino ELFs and their options
confirm the sizes recorded below; the Timer0 rejection log is present. The compiler
is avr-g++ 14.2 and installed FastLED is 3.7.8. No compiler or simulator run was
repeated. The remote retained host-test log has 170 passes; today's locally rebuilt
runtime suite has 184 passes. See [audit details](GrevirMigrationAudit.md) and
[the source/artifact manifest](GrevirWeftpiEvidence.json).

At this earlier reconciliation checkpoint, the full local all-target build was
not green: the standalone timer-clock probe omitted the Base include directory.
The Pulse IO package was also absent. The newer checkpoint above records both
corrections. Silicon remains on hold.

Schema version 2 adds `target_compile_recorded` and `retained_legacy`, keeping
reported AVR sketch compilation distinct from native checks and archived source.
The audit date is not the target execution date. Unmapped new smoke sketches and
fixtures do not complete unrelated legacy assignments: SerialHello does not port
the legacy timed Serial example, and the two-blinker legacy harness is still pending.
Earlier sections below retain their historical checkpoint scope.

## FastLED extraction — 22 September 2026

`grevir-fastled` wraps `ardOFastLED` as `LedStrip` plus an exclusive
`decltype(FastLED)` claim. Production depends are Base, Core and Library
Manager FastLED; the unused time include is gone. Pins stay injected
(`ExternalPin`). Host tests define `GREVIR_FASTLED_HOST_MOCK`. `get()` now
returns the stored colour.

Host-mock: 4 Catch2 cases. Claim probes accept one strip, and a strip plus a
distinct GPIO; they reject two strips and a reused data pin. Isolated consumer
passes without Catch2.

Arduino CLI on weftpi installs FastLED via Library Manager. **FastLED 3.10.5**
links with stock avr-gcc 7.3 (4482/343 for a one-pixel sketch) but its AVR
clockless `_dc<unsigned>` templates fail on Debian gcc 14. **FastLED 3.7.8**
compiles with Debian `avr-g++` 14.2 and Grevir `-std=c++23`. Command:
`arduino-cli lib install FastLED@3.7.8`.

| Sketch (Uno, gcc 14, C++23, FastLED 3.7.8) | Flash | RAM |
| --- | ---: | ---: |
| StripOn | 3956 | 126 |
| FastLedQuadEncoder | 5360 | 194 |

ParkLightsV2 is copied under `extras/legacy` and is not compiled. Silicon and
WS2812 timing remain unvalidated. Next third-party-free driver remains pulse IO.

## Arduino adapters and Phase 4 CLI — 22 September 2026

`grevir-arduino` and `grevir-arduino-avr` are GWZ members. Shared Arduino
supplies CoreIF (`OutputPin<CoreIF, N>`), Serial, PWM without timer claims, an
optional EEPROM header, and an explicit `GREVIR_ARDUINO_HOST_MOCK`. The AVR
package maps Uno/Nano Arduino 0–19 (pin 13 is PORTB.5), PWM 5/6→Timer0,
9/10→Timer1, 3/11→Timer2, and `ArduinoAvrApplication` claims `HardwareTimer<0>`.

Host-mock: 10 new Catch2 cases (Arduino 6, Arduino AVR 4). Claim probes accept
distinct pins/Serial/PWM-on-Timer1 and reject pin reuse, two `SerialIO<0>`, PWM
on pin 5 with millis, and exclusive Timer0. Isolated installed consumers pass
without Catch2.

Arduino CLI on weftpi: official `arduino-cli` 1.5.2-rc.1, platform
`arduino:avr@1.8.8`. Stock bundled avr-gcc 7.3 rejects `-std=c++23`. Compiles
use Debian `avr-g++` 14.2 via `compiler.path=/usr/bin/` (platforms increment,
not a language downgrade). Two-library discovery: Blink includes
`GrevirArduinoAVR.h` and pulls Grevir Arduino plus Base/Time/Core/Peripherals.
`--library` paths only, no workspace include path.

| Sketch (Uno unless noted) | Flash | RAM |
| --- | ---: | ---: |
| BareMinimum | 442 | 9 |
| Blink | 1146 | 23 |
| Blink (Nano) | 1146 | 23 |
| SerialHello | 1458 | 194 |
| ReservedTimerFail | rejected: `Application has resource conflict.` | |

Arduino.h `min`/`max`/`constrain` macros are undefined in the adapter after
including `Arduino.h`. FastLED extraction is recorded above. Silicon stays on
hold. Next third-party-free driver remains pulse IO.

## AVR Phase 1 on weftpi — 22 September 2026

Debian packages installed: `gcc-avr` 14.2.0, `binutils-avr` 2.43.90, `avr-libc`
2.2.1, `simavr` 1.6, `libsimavr-dev`, `gdb-avr` 15.1. `avr-g++` is
`/usr/bin/avr-g++`. Commands and ELFs are under `avr-probe/` with toolchain
`cmake/toolchains/avr-atmega328p.cmake`. Host-mock builds were not reused.

ABI (`-mmcu=atmega328p -std=c++23`), matching [Avr.md](review-policies/Avr.md):
`CHAR_BIT` 8, `sizeof(int)` 2, `sizeof(long)` 4, `sizeof(void*)` 2,
`sizeof(size_t)` 2. Linked `abi.elf` is 150 text bytes; `features.elf` (fold
expressions, `if constexpr`) is 178. The CMake Ninja tree `build` equivalent is
`avr-probe/out/cmake` and produces the same sizes.

avr-libc headers pass: `limits.h`, `stdint.h`, `stddef.h`, `assert.h`, `stdlib.h`,
`string.h`, `avr/io.h`, `avr/interrupt.h`. Every probed C++ standard header
fails (`climits`, `cstdint`, `array`, `type_traits`, `utility`, `limits`,
`tuple`, …). Debian gcc-avr looks for
`/usr/lib/avr/include/c++/14.2.0` and that tree is absent. C++23 as a *language*
works; libstdc++ is not installed. Host CMake still sets `HAS_STD_LIB=1`; the
AVR tree leaves it unset and uses Base compat fallbacks. Debian 14.2 stays the
weftpi compiler; stock Arduino AVR 7.x was not used.

See [GrevirAvrValidationPlan.md](GrevirAvrValidationPlan.md).

## AVR Phase 2 on weftpi — 22 September 2026

The separate CMake tree `build/avr-atmega328p` uses
`cmake/toolchains/avr-atmega328p.cmake` with `GREVIR_HAS_STD_LIB` OFF. Ninja
does not pass `HAS_STD_LIB`. Host-mock is unchanged.

Public header TUs compiled with `avr-g++ -mmcu=atmega328p -std=c++23`:
`GrevirBase.h`, `GrevirTime.h`, `GrevirCore.h`, `GrevirPeripherals.h`,
`GrevirRegisters.h`, `GrevirAVR.h`. Compat fallbacks added or extended for
missing libstdc++ pieces, including `compare` / `string_view` used by PWM
requirements.

`grevir_avr_firmware` links as ELF 32-bit LSB Atmel AVR, statically linked
against avr-libc `crtatmega328p`. `avr-size --format=avr --mcu=atmega328p`:
**142 program bytes, 0 data bytes** (0.4% of 32 KiB flash). Map:
`build/avr-atmega328p/avr.map`. `main` is at 0x80 (`.text.startup.main`, 10
bytes). Disassembly shows avr-libc vectors/startup calling `main`, then
`sbi 0x05,7` and `in r24,0x05` for the Register RMW/Read of I/O 0x05 (data
address 0x25). These sizes are this image only; no cycle claims.

Core claim probes on the same `avr-g++` (no `HAS_STD_LIB`): case 1 compiles;
case 2 is rejected with `static assertion failed: Application has resource
conflict.` Logs:
`build/avr-atmega328p/avr-probe/claim-results/`.

## AVR Phase 3 on weftpi — 22 September 2026

simavr 1.6 / libsimavr on weftpi, MCU `atmega328p` 16 MHz. Native host
`build/simavr-host/grevir_simavr_host`. Cycles are simavr, not silicon.

Smoke of `grevir_avr_firmware`: 2000 cycles, `PORTB=0x80`, `pc=0x008c`, not
crashed. CLI `simavr -m atmega328p -f 16000000` loads the ELF (timeout exit 124).

Timer1 Fast PWM (`ICR1` TOP=200, `/8`): OCR1A write 40→120 at `TCNT1=70` does not
set `OCF1A` before BOTTOM; CPU OCR reads 120. Double-buffer at BOTTOM.

Grevir `ICR1=0xA5C3` high-then-low reads back. simavr 1.6 also commits reversed
`ICR1` `0x1234`. Stopped `TCNT1` reads 0. TEMP disagreement is silicon.

`TIMER0_OVF` vector 16: pending/vector cycle 440, 3-cycle `jmp` to ISR `0x0090`,
RETI to `main` at 476 (33-cycle ISR). Datasheet 4-cycle entry was not a gap here.

Probe sizes: ocr 526/14, latch 442/14, irq 356/14 flash/RAM. Limitation list:
[GrevirAvrValidationPlan.md](GrevirAvrValidationPlan.md). Phase 4 Arduino CLI
is recorded above. Silicon stays on hold.

## Stepper extraction — 22 September 2026


The eleventh member, `grevir-stepper`, extracts `ardOStepper.h` into
`src/grevir/stepper/stepper.hpp` with aggregate `GrevirStepper.h`, CMake export
`grevir::stepper`, Arduino metadata and license. Production dependencies are Base,
Time, Core and Peripherals. Pins and clocks are injected; Encoder is not a
production dependency. Phase tables, bit order, wrap, coil-off and float time
scale are retained.

`ardo::CoreIF::MillisTime` / `now()` are replaced by `Clock`. Output pins replace
implicit Arduino pins. The unused cyclic-int include is dropped. The custom-sequence
constructor takes typed periods because `Period` construction is explicit. The first
forward step still applies sequence row 1. Coil hold 0 leaves coils on; an unsigned
maximum never expires. The Arduino `StepperEncoder` example remains planned.

All 170 workspace host cases pass, including 11 stepper cases and one optional
encoder-follower case linked only when Encoder is present. The same 12 pass
ASan/UBSan. Two public headers compile independently. Two valid and two rejected
pin-claim probes pass. Raw-token brace checks pass on the seven new C++ files.
Isolated production/install/consumer checks pass with Catch2, Test Support and
Encoder discovery disabled. The ledger now has 102 verified assignments; the
stepper header and test rows are extracted, the Arduino example stays planned.
All 736 original source hashes remain unchanged. See the
[package README](../grevir-stepper/README.md). Next third-party-free driver:
Pulse IO. FastLED remains held.

AVR compiler and hardware validation remain on hold.

## Quadrature encoder extraction — 22 September 2026

The tenth member, `grevir-encoder`, extracts `ArdoQuadEncoder.h` into
`src/grevir/encoder/encoder.hpp` with aggregate `GrevirEncoder.h`, CMake export
`grevir::encoder`, Arduino metadata and license. Production dependencies are Base,
Time, Core and Peripherals. Pins and clocks are injected; there is no Arduino,
FastLED or MCU-backend dependency. Decoder polarity, `NUllScaler`/`NonlinerarScaler`
spellings, virtual `getInput`/`scaleValue` and the module wrapper are retained.

`InputPin<N>` and `ardo::CoreIF::now()` are replaced by pin types with static
`get()` and `InteractiveScaler<Clock>`. Time's interactive scaler still ignores the
100/500 constructor periods and uses its 4/100 bounds; unit steps can truncate to
zero until the float remainder accumulates. The old 16-line Arduino harness is
adapted into ten host cases.

All 158 workspace host cases pass, including the ten encoder cases. The same ten
pass ASan/UBSan. Two public headers compile independently. Two valid and two
rejected pin-claim probes pass. Raw-token brace checks pass on the six new C++
files. Isolated production/install/consumer checks pass with Catch2 and Test
Support discovery disabled; the consumer include path is the copied install tree.
The ledger now has 100 verified assignments; both encoder rows are extracted. All
736 original source hashes remain unchanged. See the
[package README](../grevir-encoder/README.md). Next bounded driver without a
third-party library: stepper. FastLED remains held for the Arduino/FastLED
dependency.

AVR compiler and hardware validation remain on hold.

## Packet extraction — 21 September 2026

The ninth member, `grevir-packet`, supplies the four mapped header/reassembler/
manager/sender headers, aggregate `GrevirPacket.h`, exported `grevir::packet`
target, Arduino metadata, license and standalone consumer. It has no Grevir or
Arduino production dependency. Peer addresses are a caller-selected type (default
opaque uint32 identity). Standard-library requirements remain explicit; callback
templates avoid mandatory `std::function` conversion while preserving optional
aliases. Fixed payload arrays and a bounded reassembler pool retain deterministic
codec storage.

The extraction repairs inherited bitmap-shift and stream-ID narrowing defects,
fragment-size/count validation, uint16 length truncation, duplicate overwrites,
raw-object overlays, reserved-marker collisions and recency comparison across
counter wrap. Wire marker, header layout, CRC-32C/sequence identity and unmarked
passthrough remain. CRC checking is not added: the original receiver used CRC only
as part of an ID, despite its introductory corruption-detection claim.

The broken historical harness is adapted into 12 cases, retaining the original
payload and shuffled-stream scenarios with deterministic seeds. All 148 workspace
host cases pass; the 12 packet cases also pass ASan/UBSan. Five independent public
headers, one valid/six rejected compiler probes and raw-token brace checks on eight
new C++ files pass. A copied production package installs and its consumer runs with
no other Grevir dependency and Catch2/Test Support discovery disabled.

Five new extraction records bring the ledger to 98 verified assignments; all 736
original source hashes remain unchanged. The [package README](../grevir-packet/README.md)
records capacity, callback lifetime, replay/eviction and integrity limits. MCU
standard-library availability, AVR compiler validation and hardware behavior remain
unvalidated under the existing hold. This checkpoint includes both Pulse Codec and Packet extractions.

## Pulse Codec extraction — 21 September 2026

`grevir-pulse-codec` is the eighth GWZ member. The four planned public headers
extract bit storage, waveform parameters, encoder and decoder from `pwe_serial.h`.
The aggregate header, CMake export (`grevir::pulse_codec`), Arduino metadata and
license follow the existing packages. Only Base and Time are production dependencies;
GPIO scheduling, packet framing and hardware integrations remain separate.

The legacy array read failed compilation. Collector fixes then exposed three
failing cases for alternate polarity and stale bits after reset/error. Corrections
cover array sizing/indexing/reset, false-bit replacement, complementary pulse halves,
partial/unread frame recovery, genuine idle detection, malformed final pulses,
modular deadlines and saturating wait estimates. Integer waveform ratios now use
bounded quotient/remainder arithmetic, retaining floor rounding without floating
point or unnecessary wider intermediates. Existing default wire polarity is retained;
the inherited polarity flag's actual behavior is documented explicitly.

All 136 workspace host cases pass, including 11 Pulse Codec cases and the original
32,768-value sweep with deterministic jitter. The same 11 cases pass address/undefined
sanitizers. Five independent headers, representative template instantiations and an
isolated production/install/consumer build pass. One valid/seven rejected compiler
probes and raw-token brace checks on eight new C++ files pass. The installed consumer runs with
Catch2 and Test Support package discovery disabled. Source-level review includes
16-bit integer promotions, unsigned shifts, fixed storage and clock bounds; AVR
compiler/hardware validation remain on hold.

The ledger records all five planned Pulse Codec assignments, bringing actual
extractions to 93. Original Ardoinus source hashes remain unchanged. See the
[package contract and limits](../grevir-pulse-codec/README.md). Next recommendation:
extract Packet Codec as the next bounded migration increment.

## Installed PWM integration and full-width Registers fix — 21 September 2026

The full-width uint32 mask identity failure was reproduced before the fix.
`ApplyMaskShift` now has one single-operation implementation and a multi-operation
specialization requiring at least two operations. The redundant identity
specialization is removed. Static checks cover 8/16/32/64-bit identities; a native
32-bit register check preserves the high bit and performs no read before a full
write. The installed Registers consumer exercises the same boundary.

The existing PWM MVP now resides in installed Core, Peripherals and AVR packages,
with experiment forwarding headers retaining the independent checks. Core supplies
a policy-free bounded search plus RequestedModule/ExistingModule/AllocatedApplication
assembly; Peripherals owns portable PWM requirements and validation; AVR owns the
ATmega328P backend and typed drivers. Descriptor dependencies and existing module
closure claims participate in assembly. Timer/range/shared-use and physical GPIO
claims become reservations. One owner claims each selected timer and its outputs;
Core validates the complete bound application. Setup runs once per owner before
parameter/module callbacks. Whole-resource ownership now also conflicts with shared
users, closing an inherited Core claim gap exposed by integration.

All 125 production cases and the four sanitizer-backed allocation checks pass.
The new application compiler control and six expected rejections pass. Isolated
production/install/consumer checks pass for Core, Registers, Peripherals and AVR,
including a real portable module declaration in the AVR installed consumer.
Search budgets use uint32_t to preserve 100,000 on a 16-bit size_t ABI; frequency
specializations match their uint32_t declaration types. No timer features were
added. See [the integration guide](GrevirPwmIntegration.md) for the contract and
remaining limits. AVR compiler/hardware validation remain on hold.

## ATmega328P portable timer MVP — 21 September 2026

The experimental portable path now generates synchronous fast-PWM candidates from
all three existing ATmega328P timer declarations and emits typed setup/duty bindings.
Built-in TOP, OCRA TOP with B output, Timer1 ICR TOP, independent shared channels,
reservations and declaration-order independence are covered. No new device tables
were copied. `pwm_clock.hpp` is newly authored waveform conversion, not another
legacy extraction; the two existing configuration extraction hashes are refreshed.
Fast PWM programs count−1 and reports clock/(divider*(TOP+1)); dual-slope remains
clock/(2*divider*TOP). The low-level raw-OCR duty/rescaling convention remains;
the experimental endpoint handles fast-PWM high ticks explicitly.

All 123 production host cases and the isolated AVR production/host/install/consumer
checks pass. The four standalone checks pass with address/undefined sanitizers,
including all six PWM routes, full 16-bit period, endpoint/one-tick writes, invalid
writes without IO, independent grouped outputs and 168 exhaustive period-oracle
comparisons. Existing 6,144 allocator and 84,672 frequency comparisons still pass.
Native optimized setup/duty and configuration probes show no floating or 64-bit
arithmetic; metadata generation is constant evaluated. This does not establish AVR
code size, cycle costs or electrical behavior. See the experiment README for the
supported scope, startup preconditions and duty rounding contract.

The experimental MVP is complete. Installed portable API/Core integration, a named
ESP32 backend, board policies and additional timer features remain separate work.
Other AVR variants, asynchronous operation, capture/interrupts and portable runtime
frequency control stay TBD. AVR compiler and hardware validation remain on hold.

## Portable timer design prototype — 21 September 2026

The independently reviewed design now has a standalone experiment under
[`experiments/timer-allocation`](../experiments/timer-allocation/README.md).
Common/resident-target configuration, lazy inactive sections, named request
identities, canonical backtracking, structured failures, shared timer ownership
and shared-setting compatibility run against synthetic inventories. No production
header, member repository or source-extraction ledger entry changed.

AppleClang 21 compiles the static checks. All three standalone CTest checks pass with
address/undefined sanitizers: 6,144 comparisons against an independent exhaustive
oracle across all 512 three-request/three-timer graphs, plus a positive compiler
control and seven expected failures. Frequency-window membership additionally
agrees with 84,672 independently evaluated pairs of relative-error constraints.
The eight-request constexpr example establishes
one immediate-solution case, not a worst-case compilation guarantee. Raw-token
brace checking passes all eleven prototype C++ files; seven headers and the README
example compile independently.

The experiment is deliberately narrower than the design: explicit synthetic
candidates/pins/group IDs, two-component names, a simple timer/channel tree and one
shared domain per candidate. Common and active-target frequency clauses now
intersect as closed exact rational intervals. Touching endpoints remain valid;
empty intersections report a request conflict, independent of declaration order.
There is no interval rounding or new firmware arithmetic. No real driver binding,
owner setup, Core range-claim
integration or register IO is implemented. The existing 123 production host cases
are unchanged and were not rerun for this isolated experiment. AVR TOP conversion,
target compiler validation and hardware validation are not completed by it.

## ATmega328P timer/GPIO bindings — 21 September 2026

Concrete clock/mode tables, COM encodings, timer fields/registers, Timer0/1/2
compositions and the 23 real GPIO identities now use the extracted reusable layers.
The clock/mode/COM/field groups (80/166/40/328 legacy lines) match between both
source headers. Device headers separate metadata, fields, access, GPIO and timer
composition; each stays below 500 lines. Raw facts occupy the bounded subset
`generated/atmega328p/timer_gpio.hpp`; the full generated-device assignment remains
planned. Consumers explicitly include the device and supply byte-access/barrier
policies. No global target switch, clock or Arduino reservation is assumed.

All 276 selected raw facts match upstream avr-libc. Microchip's ATmega328P
7810D datasheet was consulted for pin routes, word access and flag clearing;
reference links are in the AVR README. The timer adapter now separates synthetic
TCCR byte pairs from native Timer1 words, reads words low/high and writes high/low,
and brackets word operations/masked timer updates with the caller's RAII barrier.
W1C flag updates issue only the requested clear bits. The old Timer0/2 inventories
omitted native TCCRnB despite exposing force-compare fields; these are now selectable.
Missing capture IRQ/status fields are included. No fictitious PC7 or board-specific
frequency/output presets are carried into production bindings.

All 123 host cases pass (Base 5, Time 4, Core 10, Peripherals 29, Registers 23,
Test Support 3, AVR 49). Seven device cases and nineteen static assertions cover
address/route data, both timer widths, shared-latch sequencing, flag isolation,
force-compare access, capture controls and status fields. One valid/three rejected
native probes pass. Seventeen AVR public headers compile independently. Isolated
production/host/install/consumer checks pass with Catch2/Test Support disabled for
the consumer, including concrete Timer1 setup and frequency/duty updates. The brace
checker passes 153 C++ files including disabled branches. Evidence is under ignored
`build/atmega328p-*` and `build/atmega328p-review/`.

The ledger now has 88 actual extraction assignments plus one superseded. Mapping
revision 4 records the supporting mode/register/raw-fact/access headers separately;
the original CSV import is unchanged. All 736 original source hashes remain intact.
Timer configuration/output work is included in the same checkpoint as these bindings.

This is native register-effect validation: PWM waveform timing, interrupt execution,
Timer2 asynchronous operation and electrical behavior remain unmodeled. A target
barrier implementation, waveform-specific TOP conversion, portable timer/GPIO
integration, resource inventory/reservations and board mappings remain. The separate
full-width Registers conversion issue recorded below is still open. AVR compiler
and hardware validation remain on hold.

## Reusable timer output application — 21 September 2026

Two matching output groups (97 and 178 lines) from both legacy timer headers now
share `timer/output.hpp`. The cohesive output layer supplies `TimerOutputPin`,
output settings/selection, `TimerPwmPinConfiguration` and the `Timer` facade.
COM encodings come from the output field type; GPIO, registers, mode metadata and
clock traits remain explicit. No concrete device inventory or Arduino binding is
introduced. The header remains below 500 lines.

Native compilation first reproduced the fractional writer's illegal narrowing
initializer and the wrapper's call to a nonexistent configuration `pwmWrite`.
Conversion now follows finite/range checks, and the wrapper routes to the chosen
output using live TOP. Integer duty rescaling replaces the inherited unconditional
float ratio with a floor-divided 32-bit product. Both factors are bounded by 65535,
with widening before multiplication; this output backend supports up to 16 logical
count bits. Generic cross-MCU numeric defaults are unchanged. Legacy endpoint
polarity is retained and documented rather than silently inverted.

Output composition rejects repeated channels/GPIO types, OCRA used for both TOP
and duty, insufficient count capacity, and writes through unconfigured settings.
Invalid frequency changes skip duty adjustment and perform no writes (the wrapper
may read previous TOP). Disconnected or unrelated COM modes are preserved. Compare
and endpoint-latch writes precede connecting/disconnecting timer control; setup
prepares output settings before DDR. Checked facade TOP reads preserve unavailable
results as zero. These are host-observed effects, not atomic/glitch-free guarantees.

All 116 host cases pass (Base 5, Time 4, Core 10, Peripherals 29, Registers 23,
Test Support 3, AVR 42). Eight output cases and ten static assertions cover polarity,
register order/widths, isolated channels, fractional bounds, exact integer rescaling,
valid/rejected frequency updates, facade reads and OCRA-TOP/OCRB-output composition.
One valid/seven rejected standalone probes pass. Ten AVR headers compile alone.
Isolated production/host/install/consumer checks pass; the installed consumer uses
custom COM encodings and explicit traits, with Catch2/Test Support discovery disabled.
Native UBSan/float-cast-overflow checks pass, and optimized dynamic integer duty and
adjustment IR contains no floating or 64-bit arithmetic. The brace checker passes
143 C++ files including disabled branches. Evidence is under ignored
`build/timer-output-*` and `build/timer-output-review/`.

Assignments 391 and 776 share the canonical header: 78 actual extraction assignments
plus one superseded. Configuration hashes were refreshed for built-in TOP metadata;
all 736 original source hashes remain unchanged. Next is concrete ATmega328P timer
field/inventory binding and its host composition, followed by portable pin/backend
integration. The separate full-width Registers conversion issue recorded below
remains open. AVR compiler and hardware validation remain on hold.

## Reusable timer configuration — 21 September 2026

The matching 363-line configuration groups in both legacy timer headers now share
`timer/configuration.hpp`. PWM settings, programmable/built-in TOP calculations,
setup and dynamic frequency updates use caller-provided definitions and optional
explicit clock traits. Legacy API spellings are retained. Waveform traits come
from the timer definition. Output-pin application remains forward-declared.

Native probes first reproduced invalid-frequency register writes and an incorrect
`actual_divider` (16,000,000 instead of 64). Invalid updates now return zero before
any IO, and divider reporting uses the selected mapping. Programmable TOP selection
uses the smaller counter/TOP field capacity and rejects counts below two before
narrowing. Built-in configurations diagnose invalid frequencies/resolutions and
missing modes. Frequency reads check optional metadata/TOP presence, retaining the
legacy -1 sentinel for unavailable results and zero for stopped/unmapped clocks.

All 108 host cases pass (Base 5, Time 4, Core 10, Peripherals 29, Registers 23,
Test Support 3, AVR 34). Six new cases cover setup/update register effects, preserved
fields, native widths, narrow TOP selection, rejected updates without IO, and live
frequency reads. Twelve static assertions plus one valid/seven expected-rejection
probes pass. Nine public AVR headers compile independently. Isolated production,
host, install and installed-consumer checks pass with Catch2/Test Support disabled
for the consumer. Optimized native IR for dynamic integer configuration and
frequency reads has no floating or 64-bit arithmetic. The brace check passes for
139 files, including disabled branches. Evidence is under ignored
`build/timer-configuration-*` and `build/timer-configuration-review/`.

Assignments 388 and 773 share the canonical header: 76 actual extraction
assignments plus one superseded. All 736 original source hashes remain unchanged.
The inherited count model and clock/mode-then-TOP application order are retained;
no claim of waveform-accurate timing, glitch-free live updates or atomic register
access is made. Next is runtime output application, then concrete device bindings.
AVR compiler and hardware validation remain on hold.

Separate inherited Registers follow-up: native instantiation of
`ApplyMaskShift<MaskShift<uint32_t, UINT32_MAX, 0>>` is ambiguous between its
identity/single/variadic specializations. This surfaced while deriving capacity;
configuration derives the mask directly from metadata and does not instantiate a
converter for that query. The 32-bit capacity assertion is metadata coverage;
runtime configuration checks use 8-/16-bit fields, not full-width 32-bit register IO.
The generic Registers conversion issue remains to be corrected separately.

## Reusable timer-definition extraction — 21 September 2026

The matching 213-line groups in the two legacy timer headers are consolidated
into `timer/definition.hpp`. It provides capture/noise/edge setting appliers,
output-compare metadata, TOP getters/register selection and `TimerDefinition`.
The header remains below 500 lines. Caller-provided register policies and an
optional explicit mode-traits argument remove any need for concrete device
globals; omitted traits retain legacy enum specialization.

Host probes reproduced three inherited failures: built-in TOP lookup reported
present data for unknown or register-based modes; OCRA-only sources could not
convert their inferred narrow optional to the public 32-bit optional; empty
source lists had no dispatch implementation. Built-in getters now preserve
absence and check the mode's TOP source. Timer-local typed dispatch fixes the
result representation and empty-list behavior without changing Base algorithms.
Low-level built-in getters now return an optional; OCRA/ICR getters still return
native-width integers. Public `get_timer_top` retains its optional 32-bit result.

All 102 host cases pass (Base 5, Time 4, Core 10, Peripherals 29, Registers 23,
Test Support 3, AVR 28). Five new runtime cases cover capture control preservation,
absent-feature no-ops, exact register-read traces/widths, optional validity, and
restricted/empty sources. Thirteen static assertions plus compile-only MMIO users
cover traits, field types and capability composition. Eight public AVR headers
compile independently. Isolated production/host/install/consumer checks pass;
the consumer exercises explicit traits and TOP reads without Catch2/Test Support.
The brace checker covers 135 files including disabled branches. Evidence lives
under ignored `build/timer-definition-*` and `build/timer-definition-review/`.

Assignments 389 and 774 now share the canonical definition header: 74 actual
extraction assignments plus one superseded. All 736 original source hashes remain
unchanged. This is reusable definition logic, not concrete Timer0/1/2 inventories,
interrupt wiring, register atomicity or validated hardware capture/noise behavior.
No floating-point or 64-bit arithmetic was introduced. Next is timer configuration,
then runtime output application and concrete device bindings; AVR compiler and
hardware validation remain on hold.

## Timer waveform-mode extraction — 21 September 2026

`timer/mode.hpp` contains one canonical waveform metadata/lookup group from
`ardo_avr_base_timer.h` and `ardo_supplemental_atmega328p_dev.h`. The 263-line
legacy groups match after whitespace normalization. A bounded move keeps the
complete declarations and namespace together; the new header is 284 lines.
Its direct Base dependency is explicit in CMake exports and Arduino metadata.

Mode filtering returns a tuple in declaration order; built-in TOP lookup returns
an exact match or `void`. Runtime enum lookup returns optional metadata, with
first-match behavior retained. Unknown/reserved encodings produce an empty
optional, and out-parameter misses preserve their caller's previous value.
Native probes reproduced two inherited interface gaps: `found` existed only on
empty tables and `built_in_type` only on nonempty tables. Both now work in either
case. An explicit 32-bit `UnspecifiedTimerTop` marker replaces the unrelated
native-width `NA` dependency. No floating or 64-bit arithmetic is introduced.

ATmega328P enum encodings and tables are fixture data only, not a production
device inventory. Unsupported queries return an empty tuple/`void`; mandatory
request diagnostics belong in the later configuration layer. Caller-supplied
tables are not newly validated as hardware facts in this increment.

Evidence: all 97 host cases pass (Base 5, Time 4, Core 10, Peripherals 29,
Registers 23, Test Support 3, AVR 23). Four new runtime cases cover 21 fixture
entries, reserved/unknown encodings, output preservation and split mock fields.
Six relocated legacy assertions plus 22 additional assertions cover compile-time
selection and missing requests. Seven public AVR headers compile independently.
Isolated AVR production/host builds, installation and the installed consumer pass;
the consumer uses its own enum/table and disables Catch2/Test Support discovery.
The brace check covers 131 C++ files, including disabled branches. Logs are under
ignored `build/timer-mode-*` and `build/timer-mode-review/`.

Ledger assignments 390 and 775 now share the canonical header: 72 actual
extraction assignments plus one superseded. All 736 original source hashes remain
unchanged. Timer definitions are next, followed by configuration/output application
and concrete device bindings; AVR compiler and hardware validation remain on hold.

## Arithmetic corrections — 21 September 2026

- AVR clock helpers now branch by the caller's numeric types. Integer paths use
  32-bit masks and successive divisions, preserving ceiling selection, odd clocks,
  invalid sentinels, the unit-divider fallback and narrow-result range checks.
  Explicit floating calls retain their chosen/common floating precision; bounds
  reject a rounded power-of-two limit before conversion to an integer.
- Base's compatibility random generator now has an unsigned 32-bit recurrence
  and masks before conversion to `int`. Its disabled branch is exercised by a
  standalone host executable; this does not validate every compatibility fallback.
- The generic integer scaler widens its input before shifting. The correction
  follows C++ promotion rules for AVR's 16-bit `int`, preserving the generic API.
  Shared duration literals and fractional interactive scaling are unchanged.

Evidence: 93/93 host cases pass (Base 5, Time 4, Core 10, Peripherals 29,
Registers 23, Test Support 3, AVR 19), alongside existing compile probes and six
new timer constant-expression assertions. Optimized native IR contains no
floating-point or 64-bit arithmetic in the inspected dynamic 32-bit timer calls;
the explicit `float` result path uses `float`. Native UBSan checks pass for the
fallback random sequence and all 256 byte-to-32-bit scaler inputs. The brace check
covers 127 C++ files, including disabled branches. Logs/probes are under ignored
`build/arithmetic-fixes-*` and `build/embedded-cost-review/`. These are host/source
checks, not AVR code generation, size, timing or hardware validation.

Extraction hashes for Base scaler/random and both canonical clock assignments
are updated. Original Ardoinus sources remain unchanged. The earlier extraction
records below describe their original checkpoints; their uncommitted status was
superseded by `203a8fc`.

## Initial timer clock extraction — 21 September 2026

The user authorized the next timer recommendation while AVR compiler and hardware
validation remain on hold. Concrete GPIO device bindings/integration are still
pending; this clock group has no dependency on them. No commit was made.

The divider mappings and five clock helpers were compared between
`ardo_avr_base_timer.h` and `ardo_supplemental_atmega328p_dev.h`: their source groups
match after whitespace normalization. One canonical `timer/clock.hpp` now provides
lookup, divider choice, count and frequency calculations with explicit clock
frequency and optional explicit traits. Concrete selector enums remain fixture
inputs; no device timer inventory or waveform configuration is claimed.

Native checks reproduced two inherited defects: lookup used <= instead of equality,
so stopped clock mapped to divide-by-one, and divider calculation truncated before
selection, choosing insufficient prescalers at boundaries. Lookup now returns a
32-bit invalid sentinel for unmapped selectors; required ratios round upward.
Table validation rejects invalid dividers, order, repeated selectors and mixed
enums, including faults deeper in the table. Range/finite checks and widened
arithmetic avoid invalid divisions, shifts and result conversions. The documented
unit-divider fallback remains explicit in `getClockTimerTop`; frequency queries
for unmapped selectors return zero.

The count convention is intentionally preserved. Waveform-dependent TOP encoding,
clock-source hardware behavior, complete configuration, timer inventory and output
application remain later groups. Target numeric behavior and cost are unvalidated.

Evidence on Apple Clang 21 / arm64 macOS / C++23:

- Full suite: 89/89 pass, with 16 AVR cases and 136 AVR assertions in seeded order.
  Seven new cases cover exact lookup, rounding boundaries, legacy examples,
  an independent integer capacity model, invalid arithmetic, sparse selector
  encodings with explicit traits, and computed settings applied to mock registers.
- Eleven legacy compile-time assertions pass. One valid and seven invalid mapping
  probes pass. All six AVR public headers compile independently.
- Isolated AVR production/host builds and installed consumer pass offline with
  Catch2/Test Support disabled for the production consumer. It instantiates explicit
  clock traits and checks divider/count/frequency values. Logs are under ignored
  `build/timer-clock-*` and `build/timer-clock-packages/`.
- Scope checks cover 126 C++ files including disabled branches. All 736 original
  source hashes are unchanged; all 70 actual extraction hashes match the ledger.

Clock mappings 387 and 772 are now extracted into the same canonical header.
The other timer groups remain planned. This increment and the preceding shared
fixture/GPIO increment remain uncommitted; the last commit is `2797924`.

## Shared register fixture and first AVR register/GPIO extraction — 21 September 2026

The register checkpoint was clean before this increment. `gwz repo create
grevir-avr` registered the seventh member. The requested three steps are now
implemented with the following boundaries.

1. Test Support installs `grevir/test/register_memory.hpp` and exports
   `grevir::test_support`. The former Registers-local memory implementation is now
   a reusable capacity/identity-bound store with traces, reset and checked access.
   The legacy `DebugMcuRegister` binding is adapted to `memcpy`, checked regions and
   masked input, replacing its packed-object pointer. Its caller-owned storage
   contract remains; raw `ptr()`/`Wrapper` internals do not. Fixture-only package
   discovery does not load Catch2. Registers and AVR both use the shared fixture.
2. `bit_fields_test.cpp` migrates the four remaining portable legacy exercises:
   `testRegSelector`, `testRegSelector2`, `getTypeWGM1`, `rwTypeWGM1`. Their printed
   values are now assertions. Local enum/field/address fixture definitions remove
   the device-header dependency. Three traits assertions and an exhaustive
   sixteen-value split-enum case supplement the migrated behavior. Original
   timer calculations, concrete device ports and timer inspection remain pending;
   therefore the whole historical test assignment (786) is not marked complete.
3. AVR now provides explicit register bindings, memory/I/O-offset definitions,
   `GpioPortDefinition`/`GpioPort` and input/output/bidirectional wrappers. The
   duplicate GPIO bodies are consolidated; the generic resource graph already
   lives in Core. `legacy_register_definitions.hpp` forwards to the richer canonical
   definitions instead of duplicating names. No generated device inventory or
   Arduino dependency is added.

`RegisterSelector<Policy>` replaces the numeric debug-mode switch. Register aliases
require an access binding, and GPIO requires a RAII barrier type. `VolatileAccess`
is an explicit raw-MMIO policy adapted from legacy `McuRegister`; it is compiled
but never executed by host tests. It supplies neither interrupt exclusion nor
special peripheral/multi-byte behavior. A real AVR barrier and target compiler
validation remain pending. Mock byte storage does not simulate electrical pads,
clear-on-write flags, interrupts or target synchronization.

The GPIO trace checks reproduced a legacy mismatch: dynamic output configuration
wrote DDR before PORT, while typed output configuration wrote PORT before DDR.
The dynamic path now follows the typed order. Input configuration still disables
output before updating the latch. Both dynamic output levels failed before this
fix; all four dynamic/typed combinations now agree.

Evidence on Apple Clang 21 / arm64 macOS / C++23:

- Workspace: 82/82 cases pass (Base 4, Time 4, Core 10, Peripherals 29, Registers 23,
  Test Support 3, AVR 9). Seeded runs pass 692 register assertions and 45 AVR
  assertions. New shared-fixture cases cover storage isolation/reset, invalid
  access before mutation, masked updates and unaligned compatibility access.
- Shared compatibility bounds accept one valid region and reject three invalid
  ones. Existing register/Core/Peripheral compiler probes remain passing. Five
  AVR public headers compile independently with explicit offset/type assertions
  and a compile-only raw-MMIO user.
- Isolated Test Support, Registers and AVR production/host builds pass offline.
  Installed Registers/AVR consumers build and execute with Catch2 and Test Support
  discovery disabled; a fixture-only consumer passes with Catch2 disabled.
  Logs: ignored `build/avr-packages/`, `build/avr-*`, `build/shared-registers-*`.
- Clang raw-token scope checking passes 121 C++ files, including disabled branches.
  All 736 original source hashes are unchanged; 68 actual extraction assignment
  hashes match, plus the separately superseded sequencer assignment.

New extracted mappings: 71 (shared debug register access), 32/72 (register
address definitions), 767 (AVR access binding), and 386/766 (consolidated GPIO).
The complete legacy test mapping, old global AVR mock adapters, concrete device
inventory, portable pin-backend binding and timer extraction remain later work.
All current changes remain uncommitted; hardware validation remains on hold.

## Register selection and multi-register operations — 21 September 2026

The next two planned register groups are now extracted: `selection.hpp` (282 lines)
and `apply.hpp` (348 lines). They retain the original `setl` APIs and Base-only
production dependency. `ApplierRunner::applySync<Operations, MemoryBarrier>()`
requires an explicit RAII barrier type, replacing the implicit `System` policy.
The mock checks scope entry/exit; no real synchronization guarantee is claimed.

Native checks reproduced and fixed two defects: `FindRegisterForField` returned a
helper type when the match followed the first entry (and for a missing field in a
nonempty list); grouped constant writes performed an unnecessary full-register
read because of integer promotion. The lookup now returns the recursive result's
actual type, and the full-mask comparison uses the register's width. Empty
selectors now expose their register tuple and support empty no-op reads/writes;
nonempty requests against them receive a missing-field diagnostic.

Observable legacy ordering is preserved. Selectors operate in forward tuple order,
broadcast writes to every matching entry, and leave the last matching read value.
Tuple-bound individual appliers use the first match. Grouped `ApplierValues` run
in reverse register order, while explicit `Appliers` retain their operation order.
No register-alias deduplication or multi-register atomicity is introduced.

Evidence on Apple Clang 21 / arm64 macOS / C++23:

- Full workspace suite: 65/65 cases pass; Registers now has 18 cases and 611
  assertions in seeded random order. New cases cover mixed register widths,
  skipped entries, one snapshot per participating entry, broadcast/read precedence,
  operation order, empty operations, typed readers and explicit barrier scope.
- Six valid compiler operations and fifteen expected failures pass, covering
  direct, selected and grouped writes; missing/duplicate/overlapping fields; empty
  selections; bounds; and tuple-bound applier lookup. Seven public headers compile
  independently. Before the fixes, lookup compilation failed and one runtime case
  failed on the extra full-width read.
- Isolated production/host builds pass. The installed consumer exercises selection,
  grouped writes, later-entry lookup and explicit barriers with Catch2/Test Support
  disabled. Logs: `build/register-packages/` and `build/register-selection-*`.
- Source scope checking covers 106 C++ files, including disabled branches. All
  736 original hashes are unchanged, and all 62 actual extraction hashes match.

Ledger mappings 66 and 70 are now extracted. The complete legacy test mapping
(786), shared register fixture, MCU adapters and hardware validation remain
pending. Both register increments are included in this checkpoint, following
the storage/timer checkpoint `bfaf111`.

## Portable register fields and explicit access — 21 September 2026

The storage/timer checkpoint was committed through GWZ at workspace `bfaf111`,
with a clean workspace before this increment. `gwz repo create grevir-registers`
registered the sixth member. Its four production groups extract bit mappings,
value types, field formats/evaluation and register access from `setl_bit_fields.h`.
Each header remains below 500 lines. CMake and Arduino-layout metadata declare
Base as the only production dependency; existing `setl` names remain.

`McuRegister<T, Address, Access>` now requires an explicit policy supplying typed
read/write/modify operations. `IoRegister` likewise requires its access binding;
there is no implicit raw-pointer MMIO backend. The policy owns address meaning,
barriers, volatile I/O and atomicity. A new local byte-array fixture uses `memcpy`
for typed accesses, recording their addresses, widths, values and order.

The extraction exposed and corrected these inherited problems:

- A dependent type lacked `typename`; `Evaluate` narrowed a promoted expression
  during aggregate initialization. Both prevented native template compilation.
- An 8-bit full-field update read first because integer promotion made `~mask`
  nonzero. The branch now checks the complement at the register's width.
- Masked access accepted value bits outside the mask and could overwrite unrelated
  bits. The access binding now masks the value before calling the policy.
- Shift storage used the highest bit index as a bit count. It now uses index + 1;
  indices 8 and 16 select 16- and 32-bit storage respectively.

Field formats also now reject positions outside the register width. Overlapping
field views may coexist in a format; supplying overlapping or duplicate fields in
one write is rejected by the existing operation checks. Legacy RO/WO tags are not
complete hardware permission enforcement.

Evidence on Apple Clang 21 / arm64 macOS / C++23:

- Full suite: 56/56 cases pass (Base 4, Time 4, Core 10, Peripherals 29, Registers 9).
  The register cases pass 594 assertions together in seeded random order. Two
  initially failed: unnecessary full-width read and value bits outside a mask.
- All five register public headers compile independently. Eight embedded header
  assertions and eighteen pure mapping assertions from the legacy test compile,
  along with two new bit-width assertions. A separate failing width probe preceded
  the shift-storage correction.
- Two compiler probes pass and five expected failures diagnose overlapping writes,
  duplicate fields, foreign fields, field bounds and zero masks. Existing Core and
  Peripheral compiler probes remain passing.
- Isolated production and host builds pass with installed dependencies. An installed
  consumer executes partial/full writes and unaligned typed reads with Catch2/Test
  Support discovery disabled. Logs are under ignored `build/register-packages/`.
- Clang raw-token brace checking covers 101 C++ files, including disabled branches.
  All 736 original source hashes remain unchanged; all 60 actual extraction hashes
  match the ledger, with one separately superseded sequencer assignment.

Only register assignments 65, 67, 68 and 69 are marked extracted. Selection (70)
and multi-register application (66) remain planned. The complete legacy test
assignment (786) remains planned because only its pure mapping subset was moved
to `mapping_static_tests.cpp`. Shared `DebugMcuRegister` extraction remains planned;
the current local fixture is new. Device-specific clear-on-write semantics,
interrupts, MCU adapters and hardware validation remain outside this increment.

## Storage and portable timer requirements — 21 September 2026

The PWM/sequencer checkpoint is committed through GWZ at workspace `dedfa39`;
the working tree was clean before this increment. No new member is needed.

`EepromReaderWriter<T, Address, Backend>` now uses an explicit byte store with
`Resource`, `capacity`, `read` and `update`. It preserves native object bytes and
claims the region on the backend's physical resource identity. Its inherited
pointer-valued write was first reproduced as a compiler failure against a strict
byte backend, then fixed to pass each byte. Negative/out-of-capacity/unrepresentable
regions and nontrivially-copyable types are diagnosed at compile time. Adjacent
regions and distinct stores remain legal; aliases of the same store must share the
resource type and therefore conflict on overlap. The Arduino adapter remains planned.

The portable portion of `ardo_timers.h` is extracted to `timer/requirements.hpp`:
common parameter categories, frequency/variable-frequency/resolution requests,
configuration tuples and explicit filtering. AVR enums/options and implicit system
inventory are absent. The variable-frequency flag now correctly reports true.
Zero values/dividers and repeated/conflicting frequency or resolution requests are
rejected. New `CheckedTimerConfig<Backend, Config>` verifies both allowed categories
and the backend's complete-configuration acceptance predicate. Unlike explicit
filtering, checked mandatory requests cannot silently disappear.

Only the portable requirements/filter and embedded assertion mappings are marked
extracted. The planned `timer/selection.hpp` mapping (BaseTimerSelector/TimerSelector
and inventory binding) remains unimplemented; no replacement allocator or timer
hardware behavior is claimed. AVR options stay in their original source until their
backend extraction. The legacy filter assertion is adapted with a synthetic option.

Evidence on Apple Clang 21 / arm64 macOS / C++23:

- Full host suite: 47/47 cases pass (Base 4, Time 4, Core 10, Peripherals 29).
  Five new storage cases cover round trips, exact byte/address traces, preexisting
  data, backend update semantics, adjacent/end-of-store regions and no-I/O lifecycle.
- All 29 peripheral cases pass together in seeded random order: 133 assertions.
- Storage compiler probes: 3 accepted configurations and 8 expected rejections for
  type, bounds, overlap and whole-resource conflicts. Timer probes: 2 accepted
  backend configurations and 12 expected rejections for malformed, unsupported,
  contradictory or unsatisfied requests. Twelve timer static assertions pass.
- All 11 peripheral public headers compile independently. Existing pin/PWM/Core
  probes remain passing. A Clang raw-token brace check covers 90 C++ files,
  including disabled branches.
- An isolated Peripherals build and installed consumer pass offline. The consumer
  exercises storage at the backend's upper bound and instantiates checked timer
  requirements with Catch2/Test Support discovery disabled. Logs are under
  ignored `build/storage-timer-packages/`.
- All 736 original source hashes remain unchanged. All 56 actual extraction hashes
  match the ledger; the old sequencer remains separately superseded without a copy.

Hardware validation stays on hold.
Timer inventory selection/allocation, MCU/Arduino adapters and backend-specific
register/capability behavior remain later work.

## Sequencer resolution and portable PWM — 21 September 2026

The preceding period-division/debounce/button checkpoint is committed through GWZ
at workspace `e5bd777`. Its working tree was clean before this increment.

The legacy `ardo_sequencer.h` assignment is now **superseded** by the existing
`time_poller.hpp`. The old `ardox` poller and sequence types duplicate that path.
A whole-source search found no C/C++ consumers/includes; old Visual Studio project
and filter files only list the header. The original remains untouched, and no new
`sequencer.hpp` is created. The SQLite assignment records the replacement and
reason. `migration_status` exposes `superseded` with no destination hash/date;
this is a mapping resolution, not a claimed extraction. Mapping revision is now 3;
the imported CSV remains the historical snapshot.

`HardwarePwm` is extracted to `pwm_output.hpp` with a concrete `Backend` template
argument replacing implicit `ardo_system::HardwarePwmResources` selection. A backend
supplies resolution (`timer_bits`), additional `Claims` and a static write operation.
The wrapper combines pin/backend claims, applies the selected scaler, retains
optional virtual-interface support and forwards setup/loop to the pin. Backend
register configuration and timer selection remain the caller's responsibility.

Two new runtime cases exposed an inherited narrowing bug: the wrapper's public
input type used the scaler's output width, so 16-bit values were truncated before
8-bit scaling and 65535 produced zero. `value_type` now uses the selector's
`in_value_type`; 65535 correctly produces 255. The default selector still accepts
8-bit inputs. No clamping or automatic allocation is introduced.

Evidence on Apple Clang 21 / arm64 macOS / C++23:

- Full suite: 42/42 cases pass (Base 4, Time 4, Core 10, Peripherals 24).
  Five PWM cases cover expansion, equal resolution, reduction, virtual calls and
  actual application pin setup/loop order. Before the type fix, 2 of 5 failed.
- All 24 peripheral cases pass together in seeded random order: 112 assertions.
- Nine peripheral public headers compile independently. PWM claim probes pass two
  valid cases (separate timers; compatible shared timer with adjacent channels)
  and reject six conflicts: reused pin, exclusive timer, duplicate pin within one
  combined claim, incompatible timer settings, overlapping channels and an
  external timer owner. Existing GPIO and Core probes continue to pass.
- An isolated Peripherals build passes its host suite against installed dependencies.
  The installed consumer also executes PWM setup, full-scale reduction and virtual
  output with Catch2/Test Support discovery disabled. Logs: `build/pwm-packages/`.
- The Clang raw-token brace check passes 83 C++ files, including disabled branches.
  Original inventory hashes and all actual extraction hashes are verified. The
  ledger now has 53 extracted assignments and one separately superseded assignment.

Hardware validation is on hold;
portable timer-selection contracts, storage regions and MCU bindings remain later.

## Period division, debounce and button events — 21 September 2026

Gianni authorized fixing period division and extracting debounce/button handling.
`Period::operator/` now divides rather than multiplies, retaining its storage type,
units and ordinary numeric truncation. The regression first reproduced `12 / 3`
returning 36; signed/unsigned, fractional and truncation checks now pass.

Two further source mappings are extracted: `DebounceInput` from `ardOinus.h` and
`ButtonEventModule` from `ardo_button_events.h`. Their new signatures are
`DebounceInput<InputPin, Clock, debounceTime = 300>` (interval in clock ticks) and
`ButtonEventModule<InputPin, Clock>`. Pins retain their inherited claims/interfaces.
Button thresholds retain their physical 300 ms / 400 ms durations across clock
units. The active-low classifier and its decision order are preserved, including
second-press priority over a simultaneously observed timeout. Event retrieval still
consumes one pending slot rather than a queue.

Mock sequences exposed inherited debounce errors: returning to the accepted level
did not cancel a pending transition, initial high input was reported low, and setup
retained previous state. Debounce now cancels the pending transition, samples the
initial level during setup and resets its state. Strict `>` expiry remains.
Button setup also clears classification state and pending events. Its previously
undefined static instance now has a template definition in the public header.

Validation on Apple Clang 21 / arm64 macOS / C++23:

- The ten new focused runtime cases initially had eight failures; all now pass.
  The full suite passes 37/37: Base 4, Time 4, Core 10, Peripherals 19.
- Button/debounce cases exercise startup, both bouncing edges, strict debounce
  expiry, repeat setup, delayed single click, double click, long press, event
  consumption, clock wraparound and native microsecond clock thresholds.
- All 19 peripheral cases also pass together in seeded random order: 106 assertions.
- Eight peripheral public headers compile independently. Raw/debounced pin probes
  pass two valid cases and reject two conflicts with the intended diagnostic.
  Existing Core application/index probes remain passing.
- Isolated Time and Peripherals builds pass their host suites offline against
  installed dependencies. The installed consumer executes output timing, a
  debounced long click and period division with Catch2/Test Support discovery
  disabled. Logs are under ignored `build/button-packages/`.
- The raw-token scope check passes across 80 extracted C++ files, including disabled
  branches. All 736 original source hashes remain unchanged, and all 52 extracted
  assignment hashes match the current files in the ledger.

Hardware validation stays on hold. The separate
sequencer, PWM, timer selection, storage regions and MCU bindings remain later work.

## Portable GPIO/timing and foundation tests — 21 September 2026

The first `grevir-peripherals` increment extracts pin interfaces, input pins,
output/open-drain/external pins, and `TimePoller` with cyclic/finite time sequences.
Pins now take an explicit GPIO backend and logical mode enums. Pollers take an
explicit clock exposing `TimeType` and `now()`; `Sequence` takes an explicit tick
type. Class names/namespaces and original polling behavior remain: strict `>`
expiry, one-step catch-up, `init()` preserving state and `reset()` clearing it.
Callers must supply the new bindings; no MCU/Arduino adapter has been introduced.
Pin resource identity remains the logical pin number, independent of backend type.

All seven retained Base/Time test files now build under Catch2. The buffer harness
now honors requested operation counts and actually selects both buffer variants;
integer-width checks explicitly instantiate the original boundaries. Five Base
files contribute four runtime cases and static type-algorithm assertions. Two Time
files contribute three runtime cases, including unsigned wraparound. The earlier
PICOS/NANOS enum correction is included in this build; no dedicated regression was
added for it. The inherited `Period::operator/` multiplication defect was observed
while reading the time implementation and recorded in Time's README; it remains
outside this extraction and outside the retained test coverage.

`grevir-test-support` now owns shared Catch2 setup, supporting installed 3.8.1,
local source and explicit SHA-256-pinned fetching. Test fixtures stay with their
owning libraries; the legacy test-support source mappings remain planned. Normal
production builds/exports do not depend on Catch2 or Test Support. Both new members
were registered through GWZ, without manually editing workspace configuration.

Evidence on Apple Clang 21 / arm64 macOS / C++23:

- 27 CTest cases pass: Base 4, Time 3, Core 10 and Peripherals 10. The new peripheral
  cases execute actual wrappers and pollers with recorded GPIO operations and a
  controlled clock, including both open-drain variants, wraparound, cyclic/finite
  sequences, reset/catch-up and a composed blinking application.
- Six peripheral public headers compile independently. Production input/output
  wrappers accept separate pins and reject the same pin with Core's expected
  resource-conflict diagnostic. Core's 6 valid / 19 invalid application probes and
  2 valid / 3 invalid parameter-index probes still pass.
- Separate source copies of all four production libraries build and install using
  only installed dependencies, before provision of test dependencies. An installed
  peripheral consumer links and runs without Catch2.
- All four standalone host suites pass offline with installed Test Support/Catch2.
  The workspace also passes with installed Catch2, in addition to local-source
  setup. Package results/logs are under ignored `build/portable-packages/`.
- A Clang raw-token scope check inspects the extracted C++ files, including disabled
  branches. Original source inventory hashes remain unchanged; extraction hashes
  and per-file validation notes are updated in the SQLite ledger.

This validates software GPIO calls and timer logic, not electrical behavior,
register side effects, interrupts or target toolchains. Debounce/button logic,
the separate sequencer, PWM, timer selection and MCU bindings remain later work.

## Internal resource-claim validation — 21 September 2026

Gianni selected this policy: repeated exclusive resources and conflicting ranges
are errors, including within one module's own claim list. Compatible explicit
shared-use claims remain allowed. `SelfModuleParamsConflictTest<Param>` now runs
`SelfParamsConflictTest` on the parameter's resources instead of returning false
unconditionally. This closes the lone-parameter gap without changing conflict
predicates or dependency-module deduplication.

The new duplicate-resource regression compiled before the fix and now fails with
the intended diagnostic. Six positive application probes pass and nineteen
negative probes fail as intended. New cases cover lone-parameter duplicate pins,
overlapping/contained/identical ranges, whole-resource versus range in both orders,
incompatible shared settings, nonadjacent duplicate entries and a conflict in a
dependency module. Positive controls retain compatible sharing, adjacent ranges,
different resource types/IDs and empty claim lists. The prior unrelated-parameter
case still fails with the same internal-conflict diagnostic.

The workspace build, parameter indexing probes and all ten Core mock runtime tests
pass on Apple Clang 21. Original Ardoinus sources remain unchanged. Next: adapt the
retained Base/Time tests, then extend portable peripheral mock coverage. Hardware
validation remains on hold.

## Parameter indexing fix — 21 September 2026

`Parameters<...>::Param<N>` now returns the Nth type for positive indices as well
as zero. `ParamByIndex` inherits the next lookup's result and specializes index
zero to return the selected type. The public alias is unchanged. Empty and
out-of-range lookups produce `GREVIR_CORE_PARAMETER_INDEX_OUT_OF_RANGE`.

Ten new static assertions reproduced the old bug and now pass, covering first,
middle and last indices, repeated types, a singleton list, exact type preservation
and the helper itself. Two valid compiler probes pass; three invalid probes
(one past the end, a larger index, and an empty list) fail with the intended
diagnostic. The workspace build, existing application probes and all ten Core
runtime mock tests still pass on Apple Clang 21. Original Ardoinus sources remain
unchanged; the Core module destination hash is updated in the ledger.

At this checkpoint the lone-parameter duplicate-claim gap was next; the subsequent
policy decision and fix are recorded above. Hardware validation stays on hold.

## Core mock validation — 20 September 2026

Gianni authorized host-side behavioral validation and explicitly put hardware
validation on hold. Core now has ten passing Catch2/CTest cases exercising its
production `Application`, `ModuleBase`, `ModuleInstanceBase` and `Singleton`
code through deterministic mock callbacks and state. This is Core coverage, not
completed GPIO, virtual-clock, timer/register, interrupt or AVR/ESP32 mock coverage.

The first run passed seven of nine cases and failed two: a shared dependency could
run after a consumer, including when introduced through a parameter and explicit
root. The legacy closure traversal was not a valid dependency-first callback
order. The new `lifecycle_order.hpp` computes that order for `ModuleRunner`, with
no change to `AllModules` membership or the parameter-before-module phase split.
Independent applications keep their existing reverse module order. Cyclic module
dependencies now fail with `GREVIR_CORE_DEPENDENCY_CYCLE`; two compile-fail cases
cover mutual and self-cycles. This is a focused correctness change after the
initial behavior-preserving extraction.

Evidence, Apple Clang 21 / arm64 macOS / C++23:

- All ten CTest cases pass: empty application, callback phases, dependency chains,
  shared and uneven graphs, parameter dependencies/duplicate roots, persistent
  state, independent identities, cross-translation-unit identity and fixture reset.
- A single-process run in seeded random order passes 35 assertions, checking that
  test isolation does not rely on CTest giving each case a separate process.
- Workspace compilation still passes: eleven Core headers (38 across all three
  libraries), existing compile cases, two valid applications and ten expected
  failures. Two of those failures are the new cycle checks.
- Standalone Core tests pass against installed Base, with a local Catch2 source
  and fetching disabled. Catch2 3.8.1's default download is SHA-256 pinned and
  requires an explicit setup option. Running tests never downloads dependencies.
- Installation contains no Catch2 or test headers. An installed consumer builds
  and executes with the source copies hidden. Ordinary library-only configuration
  works without finding/fetching Catch2.
- Clang raw-token brace checks pass for all 61 extracted/new C++ files, including
  disabled branches. All 736 original source hashes remain unchanged; extraction
  destination hashes and the SQLite integrity checks pass.

Use the [workspace commands](../README.md#host-mock-validation) or
[standalone test instructions](../grevir-core/tests/README.md). Shared presets,
CI/other host OSes and a permanent source-scope lint remain outstanding.

Next mock work: resolve the already documented parameter-index/claim-check gaps
with regressions, adapt the retained Base/Time tests, and add deterministic clock
and GPIO fixtures as portable peripheral code is extracted. Hardware checks stay
on hold. The allocator remains a placeholder.

## Core extraction — 20 September 2026

This section records the earlier compilation-only checkpoint; the behavioral
validation and subsequent callback-order fix are recorded above.

`grevir-core` was created and registered using `gwz repo create grevir-core`.
It supplies `GrevirCore.h`, Arduino library metadata, an independent CMake target
`grevir::core`, install/export rules and the original MIT notice. Its only
production dependency is Base.

Nine owned headers separate module parameters/dependencies, singleton storage,
resource claims, conflict checks, application composition, board inventory traits,
device mappings, resource topology and the legacy allocation placeholder. GPIO,
Arduino services, target selection and register access remain outside Core.
The splits preserve complete declarations and namespaces; the resource graph
retains the historical `ardo::sys::avr::base` spelling without any MCU dependency.
Each new C++ file is below 500 lines.

Necessary boundary repairs were limited to direct includes and replacing the
graph's `setl::has_type_v` register-header dependency with Base's equivalent
`setl::tuple_contained_in<...>::value`. Existing algorithms and diagnostic strings
were retained. `SelectionResolver` remains a pass-through stub, not an allocator.

Validation with Apple Clang 21.0.0, arm64 macOS, C++23 and `HAS_STD_LIB=1`:

- Workspace build passes, including all ten Core public headers independently
  (37 across Base, Time and Core), concrete template users, historical dependency
  assertions and shared/range claim assertions.
- Both historical singleton module styles compile against test-only pin/poller
  bindings. This adapts the useful declarations, not the empty old runtime harness.
- Two valid applications compile. Eight invalid compositions fail with their
  expected static assertions: exclusive resources, same-module parameters,
  overlapping ranges, whole-resource versus range, incompatible shared settings,
  transitive dependency claims, internal claim duplicates and an empty range.
- Separate copies of Base and Core build/install outside the workspace. Core
  resolves only installed Base. A two-translation-unit installed consumer links
  with both copied source trees hidden; its compile commands contain no workspace
  or Ardoinus include paths. The executable was not run.
- A one-off Clang raw-token check passes for all 56 extracted/new C++ files,
  including disabled branches. A disabled-branch fixture verifies detection of
  an unbraced `if`; braced `do`/`while` and `else if` forms are accepted. Permanent
  lint integration and broader legacy-source migration remain deferred.
- All 736 original source hashes still match the inventory. Twelve Core ledger
  assignments now record destination hashes and compile-check evidence.

The inspection also confirmed inherited issues: nonzero `Parameters::Param<N>`
returns an index wrapper, and duplicate resources in a lone parameter of a lone
module escape checking. `RootDependencies` also retains its different treatment
of an empty dependency list at the top level versus nested traversal. These are
documented in [Core's README](../grevir-core/README.md), not silently fixed during
relocation. The negative probes do not establish complete claim-check coverage.
Runtime lifecycle/order, storage identity, fallback compatibility, Arduino and
AVR/ESP32 validation remain outstanding.

Next: make focused Core correctness changes with regression checks for those
confirmed gaps, then continue the planned portable register/peripheral extraction.
Catch2/CTest runtime fixtures and target validation remain separate increments.

## Foundation extraction — 18 September 2026

| Repository | Contents | Validation |
| --- | --- | --- |
| `grevir-base` | Portable types, compatibility wrappers, type/tuple algorithms, buffers, memory-policy contract and diagnostic hook | Native source/header compilation, concrete template instantiations, extracted tuple static assertions, standalone install/export |
| `grevir-time` | Typed time, periods, units and interactive scaling | Native source/header compilation, concrete template instantiations, standalone build against installed Base, install/export |

Both are registered GWZ member repositories with Arduino-style root layouts,
`library.properties`, public entry headers, standalone CMake targets and the
original MIT notice. They are local, uncommitted repositories without published
remotes. The original checkout remains intact; all 736 inventoried source hashes
still match. Source history import and final removal from Ardoinus remain undecided.

Existing namespaces and API spellings are retained. New entry headers are
`GrevirBase.h` and `GrevirTime.h`; owned headers use the planned `grevir/...` paths.
This increment does not modernize template algorithms or rename their symbols.

## Boundary repairs

- Optional and integer helpers now include their direct prerequisites.
- Tuple type operations, tuple iteration/search and embedded test assertions were
  separated using Clang declaration ranges. Production assertions within templates
  remain with their declarations; namespace-scope examples moved into the compile
  test. Every resulting source file is below 500 lines.
- Standard-library configuration no longer implies mock AVR registers. Native
  CMake sets `HAS_STD_LIB=1`; target capability selection remains pending.
- Portable diagnostics are separated from the memory policy. `System` is an
  explicitly sequential-use, no-op default. The AVR `_NOP()` binding is not part
  of this portable extraction; interrupt/platform policies need the later backend.
- The compact buffer specialization now uses its `S::MemoryBarrier` parameter,
  matching the full-capacity variant. Compile assertions check both policy types.

The original compatibility fallback bodies are retained, including disabled
branches; they have not passed a no-standard-library build or capability audit.
Legacy literal-operator warnings remain. The duplicate PICOS/NANOS enum values
are recorded in Grevir Time's README for a separate correctness change.

## Evidence and scope

Apple Clang 21.0.0 on arm64 macOS, C++23, using the installed CMake/Make executor:

- Workspace build: 27 public headers compiled independently, two template-usage
  translation units, the extracted tuple assertions, and the diagnostic source.
- Separate copies of Base and Time build outside the workspace. Time resolves
  only the installed Base package. An installed-package consumer compiles and
  links successfully through `grevir::time`, including the diagnostic symbol.
- No original Ardoinus include path, Arduino installation, target identity macro,
  downloaded test dependency or MCU compiler is used by these builds.
- A Clang raw-token scope check covered all 38 relocated/new C++ files, including
  disabled branches, with no unbraced control bodies found. This is a source check,
  not evidence that disabled branches compile. A permanent lint rule is deferred.

Seven historical test files have been retained with relocated production includes,
but are not compiled or run: their harness dependencies await adaptation. No runtime
test framework or target/Arduino validation was added. Build commands are in the
workspace and library READMEs; the earlier legacy compile baseline remains separate.

## Queryable status

`GrevirFileMap.sqlite` now has an `extractions` table and `migration_status` view.
The existing ownership mapping is unchanged. Statuses distinguish files checked
by native compilation, retained tests awaiting their harness, and planned work.
Destination hashes capture the extracted copies at this checkpoint; the original
source hashes remain untouched. New packaging/build files are not legacy mappings.

```sql
SELECT repository, status, count(*) AS assignments
FROM migration_status
GROUP BY repository, status
ORDER BY repository, status;
```

At the foundation checkpoint the next extraction was `grevir-core`; its first
increment is recorded above. MCU backends, Arduino adapters, runtime fixtures and
the other package extractions remain later increments.
