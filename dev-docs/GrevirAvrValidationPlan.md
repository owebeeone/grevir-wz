# Grevir AVR validation plan

Plan date: 22 September 2026. This is the executable plan for AVR compiler,
simulator and later Arduino/silicon checks. It refines, and does not replace,
the test layers in [GrevirBuildAndTestPlan.md](GrevirBuildAndTestPlan.md) and
the consumer/toolchain contract in [GrevirRepositoryPlan.md](GrevirRepositoryPlan.md).
Review uses [CrossMcu.md](review-policies/CrossMcu.md) plus
[Avr.md](review-policies/Avr.md).

**Execution host is weftpi:** `gianni@10.1.1.236`, Linux aarch64 (Raspberry Pi 5
class, Debian trixie). The workspace checkout is `/home/gianni/git/grevir-wz`,
already cloned. Produce compiler, simavr, Arduino CLI and silicon evidence on
this machine. Do not treat macOS host-mock results, Apple Clang IR or Pi native
`g++` as AVR ABI, cycle count or hardware evidence.

Authoring may happen in the Mac workspace; a step is not done until its command
and artifacts exist on weftpi.

## Packet target checkpoint — 23 September 2026

The selected Packet sketch compiles for Uno and Nano with Debian AVR C++23 and
no libstdc++. A one-slot, two-fragment Packet loopback passes in simavr. See
[Packet AVR evidence](GrevirPacketAvrEvidence.md) for sizes, artifact hashes,
commands and limits. Silicon validation remains on hold.

## Pulse IO target checkpoint — 23 September 2026

The selected Pulse IO Uno sketch builds on weftpi with Arduino CLI, Arduino AVR
1.8.8 and Debian `avr-g++` 14.2 in C++23 mode: 2,854 flash bytes and 86 RAM
bytes. simavr 1.6 links simulated D5 output to D4 input and decodes the second
frame as byte `1` after 33 output changes and 30,281 cycles. See
[the Pulse IO AVR evidence](GrevirPulseIoAvrEvidence.md) for the source revision,
artifact hash, commands and limits. This is simulator evidence; physical
loopback and silicon timing remain on hold.

## Phase 1 results — 22 September 2026

Phase 1 is complete on weftpi. Debian `gcc-avr` 14.2.0 / `avr-g++` compiles
`-std=c++23` for `atmega328p`. Measured ABI: `sizeof(int)==2`,
`sizeof(long)==4`, `sizeof(void*)==2`, `sizeof(size_t)==2`, `CHAR_BIT==8`.
avr-libc C and `avr/*.h` headers compile. **No libstdc++** is installed; every
`<c…>` / `<array>` / `<type_traits>` include fails. Language-only C++23
(templates, fold, `if constexpr`) links (150 and 178 text bytes). CMake
configures `avr-probe` with `cmake/toolchains/avr-atmega328p.cmake` in a tree
separate from host-mock.

Default compiler remains Debian 14.2. Stock Arduino AVR 7.x was not selected.
Phase 2 must build Grevir with `HAS_STD_LIB` unset (compat fallbacks) or add a
C++ library; do not set `HAS_STD_LIB=1` on this toolchain.

## Phase 2 results — 22 September 2026

Phase 2 is complete on weftpi. `build/avr-atmega328p` compiles the six public
headers and links `avr-probe/grevir_avr_firmware` with `HAS_STD_LIB` unset.
`avr-size --format=avr --mcu=atmega328p`: 142 program bytes, 0 data bytes.
The image is avr-libc CRT plus `main` at 0x80, which issues `sbi 0x05,7` /
`in r24,0x05` for the Register RMW probe. Core claim case 1 compiles; case 2
fails with `Application has resource conflict.` Map and claim logs stay on
weftpi under that build tree. These numbers are that image only.

## Phase 3 results — 22 September 2026

Phase 3 is complete on weftpi using **simavr 1.6** via `libsimavr` (`avr-probe/sim/host.cpp`,
native `build/simavr-host`). MCU name is `atmega328p` at 16 MHz. Cycles below
are simavr, not silicon.

- **3.1 smoke:** `grevir_avr_firmware` (142 flash). Bounded 2000 cycles, not
  crashed, `pc=0x008c` (exit spin), `PORTB=0x80`. CLI `simavr -m atmega328p -f 16000000`
  loads the same ELF and runs until `timeout` (exit 124); the CLI has no cycle cap.
- **3.2 OCR double-buffer:** Timer1 Fast PWM, TOP=`ICR1=200`, prescaler `/8`.
  Mid-period write of `OCR1A` 40→120 at `TCNT1=70`; `OCF1A` still clear at BOTTOM.
  CPU `OCR1A` reads 120. Buffer update is at **BOTTOM**, not “PWM works”.
- **3.3 16-bit latch:** Grevir high-then-low write of `ICR1=0xA5C3` reads back.
  Reversed low-then-high also commits `0x1234` in this simavr. Stopped `TCNT1`
  reads as 0 (synthesized). Broken-order disagreement is a silicon item.
- **3.4 interrupt:** `TIMER0_OVF` vector 16. Pending and vector at cycle 440,
  `jmp` to ISR 3 cycles, ISR `0x0090` then RETI back to `main` at cycle 476
  (`isr_to_reti=33`). Datasheet minimum entry is 4 cycles; this simavr run
  showed a 0-cycle flag-to-vector gap. Labeled simavr, not silicon.

See the limitation list in [GrevirAvrValidationPlan.md](GrevirAvrValidationPlan.md).
Phase 4 Arduino CLI is complete on weftpi. Silicon stays on hold.

## Why a separate plan

Host Catch2 fixtures compile Grevir as a native process and inspect a byte-array
register model. That layer is already green (170 cases on weftpi’s host `g++`
14.2). It cannot prove 16-bit `int` promotions, AVR libstdc++/avr-libc headers,
startup/ISR attributes, OCR double-buffering, interrupt entry cost or pin
synchronizers.

Arduino CLI is Phase 4 on weftpi, after `grevir-arduino` and
`grevir-arduino-avr` exist. Real `avr-g++` and simavr remain in front of
Arduino as ABI and cycle evidence.

## Validation layers

| Layer | Proves | Does not prove |
| --- | --- | --- |
| Host mocks (existing) | Portable logic and modeled MMIO on Linux/macOS | AVR ABI, cycles, electrical behavior |
| AVR language/header probe | Compiler version, `-std=`, `sizeof`, headers, a link of a tiny TU | Library-scale composition |
| AVR compile/link of Grevir | Production headers and a no-Arduino ATmega328P image under the real ABI | Timer/interrupt timing |
| simavr on that ELF | Cycle-accurate CPU, timers, PWM, ICU, 16-bit latch order, interrupt dispatch | Analog pin filtering, Timer2 clock-domain sync, crystal start-up |
| Arduino CLI sketchbook | Layout, `depends`, board recipe, CoreIF adapter | Silicon analog/async leftovers |
| Named Uno/Nano | Those leftover analog/async/sleep-stabilisation cases | General substitute for simavr CI |

Never define `__AVR__` on a host compiler to force the firmware path. Use a
separate CMake build directory from `build/host-mock`. Target images are not
host CTest executables.

## Machine survey — 22 September 2026

weftpi already has CMake 3.31.6, Ninja 1.12.1, host GCC 14.2, Git, Python 3.13
and a working Grevir host-mock suite. It did **not** have an AVR toolchain or
simulator installed. Debian trixie/arm64 candidates:

| Package | Candidate | Role |
| --- | --- | --- |
| `gcc-avr` | 1:14.2.0-2 | `avr-gcc` / `avr-g++` |
| `binutils-avr` | 2.43.50.20250108-1 | `avr-ld`, `avr-objdump`, `avr-size` |
| `avr-libc` | 1:2.2.1-1 | Headers and C runtime |
| `gdb-avr` | 15.1-1+b3 | Optional ELF debugging under simavr |
| `simavr` | 1.6+dfsg-3+b3 | CLI simulator |
| `libsimavr-dev` | (same series) | In-process simavr for scripted tests |
| `arduino-core-avr` | 1.8.7+dfsg-1~deb13u1 | Stock Arduino AVR core; **not** the first compiler |

There is no `arduino-cli` package. Do not use the Debian `arduino` IDE as the
sketch runner. When Phase 4 starts, install the official `arduino-cli` binary.

GCC 14.2 on AVR is the **first probe compiler**. It matches weftpi’s host GCC
major and is far newer than stock Arduino AVR 7.x. The community AVR-GCC 16.1.0
build remains a comparison option if a Linux aarch64 artifact exists; it is not
the default. Do not quietly drop to C++14 to fit stock Arduino `platform.txt`.
C++23 stays a candidate until the probe records what this `avr-g++` and
`avr-libc` actually compile and link.

Disk: about 29 GB free. Sufficient for toolchains and `build/avr-atmega328p`.

## Simulator choice

Use **simavr 1.6** on weftpi for automated ELF regression. It is cycle-accurate
enough for timers, PWM, ICU, interrupt entry and 16-bit access order.

| Tool | Use on weftpi | Do not use as Grevir CI oracle |
| --- | --- | --- |
| simavr | Primary: run linked ATmega328P ELF, traces, GDB | Analog filter limits, Timer2 domain-sync edge cases |
| Microchip Studio | Not available (Windows GUI) | — |
| Wokwi, SimulIDE | Interactive demos only | Bit-banging, pulse-codec edges, mid-period prescaler changes, stepper phase timing |

Trusted under simavr when the test names the device and clock: CTC, Fast PWM,
phase-correct PWM, OCR double-buffer at TOP/BOTTOM, synchronous prescalers,
ICP1 capture into ICR1, 4-cycle vector plus stack push plus `RETI`, and the
`TCNT1H`/`TCNT1L` temporary latch (the same broken access order fails in simavr
as on silicon).

Treat as **idealized or untrusted** until a silicon check exists: Timer2
asynchronous TOSC1/32.768 kHz domain synchronizer delays, external T0/T1
synchronizer vs analog pin filtering, and oscillator stabilization after
Power-save wake-up.

Host fixtures remain. simavr does not replace them; it runs the **AVR image**
those fixtures never produce.

## Out of scope

ESP32, FastLED, Arduino examples that need unextracted adapters, uploading from
the Mac, and claiming hardware validation from simavr.

## Phased plan

Each step is one goal. LOC budgets are aspirational for authored Grevir/CMake/
probe sources, not Debian packages or generated maps.

### Phase 1 — AVR toolchain on weftpi

Milestone: weftpi can compile, link and dump a tiny ATmega328P C++ program with
recorded ABI and header results.

| Step | Goal | Budget | Evidence |
| --- | --- | ---: | --- |
| 1.1 | Install `gcc-avr`, `binutils-avr`, `avr-libc`, `simavr`, `libsimavr-dev`, and optionally `gdb-avr` from Debian | ops | `avr-g++ -v`, `avr-size --version`, `simavr -h` recorded in the progress log |
| 1.2 | ABI probe: `sizeof(int/long/void*/size_t)`, signedness, `CHAR_BIT`, presence of libstdc++/avr-libc headers | <150 | Values match the AVR policy assumption (16-bit `int`, 32-bit `long`) or the mismatch is filed before any Grevir AVR build |
| 1.3 | Language and standard-header probe at the candidate `-std=` (C++23 first, then the highest mode that links) | <250 | Pass/fail table for features and headers Grevir already uses on host |
| 1.4 | Write the weftpi CMake toolchain file for `atmega328p` (clock 16 MHz Uno-class) | <200 | Separate `build/avr-atmega328p` configures; host-mock tree unchanged |
| 1.5 | Keep a comparison note on stock Arduino AVR 7.x vs Debian 14.2; do not switch the default | <100 | Written decision: Debian 14.2 remains the compiler until a later platforms package |

1.2 and 1.3 can proceed in parallel after 1.1. 1.4 can be drafted against 1.2
sizes. Creating `grevir-platforms` is not required to finish Phase 1; start the
toolchain file under `cmake/toolchains/` in this workspace or `grevir-avr`, and
move it when that member exists.

### Phase 2 — Cross-compile Grevir without Arduino

Milestone: `grevir-base`, `grevir-time`, `grevir-registers`, `grevir-core`,
`grevir-peripherals` and `grevir-avr` compile for ATmega328P; a minimal
no-Arduino image links; map and size are retained on weftpi.

| Step | Goal | Budget | Evidence |
| --- | --- | ---: | --- |
| 2.1 | AVR compile of public headers already covered by native compile checks | <200 | Each header is a target TU with `avr-g++`; failures listed with the missing facility |
| 2.2 | Link a minimal firmware that instantiates AVR GPIO or timer setup against real `avr-libc` startup | <350 | `.elf` plus `.map` and `avr-size` output stored under weftpi `build/avr-atmega328p/` |
| 2.3 | Repeat one expected-failure claim/collision probe with `avr-g++` | <200 | Failure is a Grevir diagnostic, not a missing include or a failed compiler launch |
| 2.4 | Record flash/RAM of the minimal image; no cycle claims | <100 | Numbers labeled as that image only |

Portable packages have no Arduino dependency. Do not add `grevir-arduino` here.
If C++23 fails in 1.3, fix the dialect in the toolchain file before expanding 2.1.

### Phase 3 — simavr regressions

Milestone: the Phase 2 ELF (and small dedicated probes) run under simavr 1.6 on
weftpi with deterministic traces.

| Step | Goal | Budget | Evidence |
| --- | --- | ---: | --- |
| 3.1 | Smoke: load the minimal ELF in simavr, run a bounded cycle count, exit cleanly | <150 | Command line, MCU name `atmega328p`, stdout/log on weftpi |
| 3.2 | Timer CTC or Fast PWM plus a synchronous prescaler: OCR double-buffer at the documented boundary | <400 | Trace or GDB observation; names TOP/BOTTOM, not “PWM works” |
| 3.3 | 16-bit register latch: correct `TCNT1` access vs the known broken order | <250 | Broken order disagrees with the correct order in simavr |
| 3.4 | One interrupt path: enable, pending, vector, `RETI`, return to the interrupted context | <400 | Cycle budget compared to the ATmega328P manual, labeled simavr not silicon |
| 3.5 | Write the simavr limitation list used by later silicon tests | <150 | Timer2 async, T0/T1 analog filter, sleep oscillator delay explicitly deferred |

3.2–3.4 can be separate ELFs if the minimal image is too large to reason about.
Prefer `libsimavr` plus a small host driver on weftpi over browser tools. VCD
dumps are acceptable artifacts.

Do not add pulse-codec bit-banging or stepper phase-timing simavr cases until
the corresponding firmware image exists and Phase 3.1 is green.

### Phase 4 results — 22 September 2026

Phase 4 is complete on weftpi. `arduino-cli` 1.5.2-rc.1, `arduino:avr@1.8.8`.
Members `grevir-arduino` and `grevir-arduino-avr` exist. Stock avr-gcc 7.3 does
not accept `-std=c++23`. Sketches compile with Debian 14.2
(`compiler.path=/usr/bin/`, `compiler.cpp.extra_flags=-std=c++23`). Blink,
BareMinimum and SerialHello link for Uno; Blink also for Nano. Exclusive Timer0
is rejected with `Application has resource conflict.` Sizes:
[GrevirExtractionProgress.md](GrevirExtractionProgress.md).

### Phase 4 — Arduino CLI on weftpi

Milestone: a clean sketchbook on weftpi builds an Uno sketch using only cloned
`libraries/` directories. Done; extraction was repository plan 4.5–4.7.

| Step | Goal | Budget | Evidence |
| --- | --- | ---: | --- |
| 4.1 | Install `arduino-cli` on weftpi; do not use Debian `arduino` as the runner | ops | `arduino-cli version` |
| 4.2 | Two-library discovery sketch (base + one dependent) in a throwaway sketchbook | <200 | Build uses no `grevir-wz` include path and no symlinks |
| 4.3 | Uno sketch using `grevir-arduino-avr` board map and reservations | <300 | Reserved core timer cannot be allocated; clone layout only |
| 4.4 | Compile the first extracted example that has an `.ino` (not FastLED) | <200 | `arduino-cli compile -b arduino:avr:uno` on weftpi |

The Arduino AVR core may still be 1.8.x with older `-std=`. If it cannot compile
Grevir, the platforms recipe (repository plan 1.5) must supply Debian 14.2 or
another maintained compiler. That is a platforms increment, not a silent
language downgrade in the libraries.

### Phase 5 — Focused silicon

Milestone: a named Uno or Nano attached to weftpi (or its USB) covers only what
simavr is documented not to trust.

| Step | Goal | Budget | Evidence |
| --- | --- | ---: | --- |
| 5.1 | `avrdude` plus one blink or GPIO loopback on a named board | ops | Board identity, fuses/clock, pass/fail |
| 5.2 | One Timer2 async or T0/T1 external-clock case if a Grevir API needs it | <400 | Observation vs the simavr limitation note |
| 5.3 | Optional Power-save + Timer2 wake-up if that API is in scope | <400 | Stabilization delay measured, not assumed from simavr |

No silicon step is implied by a green simavr suite. Hardware remains separately
scheduled even after Phases 1–3.

## simavr 1.6 limitation list (Phase 3.5)

Recorded on weftpi, 22 September 2026. Use this list when scheduling Phase 5.
Do not treat a green simavr run as covering these.

| Topic | simavr 1.6 on weftpi | Silicon (Phase 5) |
| --- | --- | --- |
| Timer2 asynchronous TOSC1 / 32.768 kHz domain | Idealized / untrusted | Required if a Grevir API uses AS2 |
| External T0/T1 clock vs analog pin filtering | Digital path only | Required if that clock source is used |
| Oscillator stabilization after Power-save | Not modeled | Required if sleep+Timer2 wake is in scope |
| Timer1 16-bit TEMP latch (TCNT1/OCR1A/OCR1B/ICR1) | Reversed byte order still commits; stopped TCNT1 reads as 0 | Broken `TCNT1L` then `TCNT1H` must disagree with Grevir high-then-low |
| Interrupt entry | Flag and vector can share a cycle (measured 0-cycle gap vs datasheet 4) | 4-cycle hardware response plus vector `jmp` |
| Analog pin filtering | Not modeled | Electrical |

Trusted in this simavr for the probes above: Timer1 Fast PWM OCR double-buffer at **BOTTOM** with a synchronous `/8` prescaler; `jmp` to ISR (3 cycles); RETI back to the interrupted `main` context.

## Immediate next step

Phase 4 Arduino CLI is done on weftpi. FastLED 3.7.8 compiles with Debian 14.2
and C++23. Silicon Phase 5 stays on hold and must cover the limitation table,
especially TEMP latch and the 4-cycle interrupt entry gap. The next
third-party-free driver extraction is pulse IO.

## Evidence location

Record compiler versions, probe tables, map/size and simavr logs in
[GrevirExtractionProgress.md](GrevirExtractionProgress.md) at each completed
phase. Keep binaries out of git; keep the commands and numeric summaries.
