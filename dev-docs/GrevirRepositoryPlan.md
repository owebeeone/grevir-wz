# Grevir repository and C++ API extraction plan

Plan date: 18 September 2026. Status updated 20 September: `grevir-base`,
`grevir-time` and the first `grevir-core` extraction are in place with native
compile checks. Core also has ten passing host behavioral mock tests and a fix
for dependency callback ordering. Core's allocator remains a legacy placeholder;
broader runtime/target milestones and remaining packages are still planned.
Hardware validation is on hold at Gianni's request. See
[extraction progress](GrevirExtractionProgress.md) for actual results and limits.

## Recommendation

Use **`grevir-<component>`** consistently for repositories and checkout directories:
`grevir-base`, `grevir-core`, `grevir-avr`, and so on. Keep `grevir-wz` as the
GWZ coordination workspace. The full prefix makes independently encountered
packages recognizable; `gr-` saves little and loses that identity.

Make each runtime repository **one Arduino library at its root**. A checkout goes
directly into the configured sketchbook's `libraries/` directory. Keep host tools,
toolchain/board distributions and test infrastructure in separate repositories.
Consumers should not need GWZ, Python, a repository reshuffle or generated headers
from a developer's machine to compile a normal installed library.

This document refines the boundaries in [ArdoPackagingPlan.md](ArdoPackagingPlan.md).
The package names below are this plan's recommendation, not a claim that the user
has approved every split. The chosen project name, Grevir, is already settled.

## Inventory and how to read the mapping

The source snapshot is `/Users/owebeeone/limbo/ardoinus`, surveyed on the date
above. [GrevirFileMap.sqlite](GrevirFileMap.sqlite) is the queryable extraction ledger:
**736 unique C/C++ headers, translation units and Arduino sketches**, represented
by **796 destination rows** because mixed files have multiple destinations.
Each row records the original relative path, line count, SHA-256, repository,
exact destination path, API unit, action and qualification.

The [first-30-files view](GrevirFirst30Files.md) lists up to 30 distinct planned
destination files per repository, alphabetically, with their original sources.
The [saved SQL](GrevirFirst30Files.sql) reproduces that query. The database separates
source files, repository identities and assignments; its `file_map` view exposes
the complete mapping. [GrevirFileMap.csv](GrevirFileMap.csv) is retained as the
original import snapshot; subsequent ownership corrections live in SQLite.
Tooling/platform repos with no existing C++ mappings are
listed explicitly in the database; their non-C++/new work remains in this plan.

| Source group | Files | Treatment |
| --- | ---: | --- |
| Raw `ardo_supplemental_<mcu>.h` | 302 | Generated facts owned by `grevir-avr` |
| Manual `ardo_supplemental_<mcu>_dev.h` | 302 | Split the substantive ATmega328P implementation; retain the other 301 development/stub headers as historical material |
| `ardo_board_*.h` | 38 | Three initial Uno/Nano board entries; retain the other 35 outside the supported API |
| All other headers, sources and sketches | 94 | Explicit per-file/per-API ownership in the ledger and appendix |

An entry under `src/` is a **planned implementation location**, not a statement
that its current contents compile or have been validated. `extract` retains a
cohesive unit; `split` separates responsibilities; `adapt` identifies necessary
boundary/harness changes; `reconcile` identifies overlapping implementations;
`generated` retains generated facts; `preserve` places historical material outside
production compilation. No input file is assigned to an unspecified future repo.

Multiple sources targeting the same file mean **reconciliation into one canonical
implementation**, never concatenation or two conflicting definitions. This applies
particularly to the two timer implementations, GPIO definitions, board maps and
board selection. Source hashes identify the surveyed versions before extraction;
regenerate the inventory if those sources change.

The map covers existing files. New APIs and new aggregate headers are listed
separately below so proposed features are not confused with working legacy code.

## Repositories and allowed dependencies

There are **16 proposed runtime libraries** and **three host/infrastructure repos**.
Create each when its first useful increment is ready; this is not a request to
publish 19 empty repositories at once. Each library can be released independently
when its own dependency closure and examples work.

In this table, dependency names omit the common `grevir-` prefix. These are the
**intended direct production dependencies after extraction**, not a claim about the
current monolithic include graph. List a direct dependency whenever its public
headers/types are directly used; do not rely on an accidental transitive include.

| Repository | Public entry header / Arduino library name | Owns | Direct Grevir dependencies | Outside dependency |
| --- | --- | --- | --- | --- |
| `grevir-base` | `GrevirBase.h` / Grevir Base | Integer/container/type utilities, color conversion, standard-header policy, portable diagnostic and memory-policy contracts | None | Selected compiler's required language/library facilities |
| `grevir-time` | `GrevirTime.h` / Grevir Time | Time units, periods, clock-value arithmetic, interactive scaling | base | Clock supplied by caller when needed |
| `grevir-registers` | `GrevirRegisters.h` / Grevir Registers | Typed fields, bit mappings, register operations and access-policy contracts | base | Access/barrier policy supplied by target or test |
| `grevir-core` | `GrevirCore.h` / Grevir Core | Resource claims/graph, module parameters/dependencies, lifecycle, application composition, allocation engine | base | None |
| `grevir-peripherals` | `GrevirPeripherals.h` / Grevir Peripherals | Portable GPIO, debounce/buttons, polling/sequencing, PWM/storage wrappers and peripheral request contracts | base, time, core | Backend/clock supplied by application |
| `grevir-avr` | `GrevirAVR.h` / Grevir AVR | AVR facts, MMIO policies, device inventory, GPIO/timer implementations and later AVR peripheral/IRQ backends | base, time, registers, core, peripherals | AVR compiler/runtime; no Arduino core dependency |
| `grevir-esp32` | `GrevirESP32.h` / Grevir ESP32 | ESP32 capabilities, SDK-backed GPIO/timers, concurrency/IRQ policy and resource inventory | base, time, core, peripherals | Selected Espressif SDK/toolchain; registers only if an actual implementation requires it |
| `grevir-arduino` | `GrevirArduino.h` / Grevir Arduino | Shared Arduino CoreIF, Serial, PWM and optional EEPROM adapters | base, time, core, peripherals | Selected Arduino core |
| `grevir-arduino-avr` | `GrevirArduinoAVR.h` / Grevir Arduino AVR | Uno/Nano board mapping, Arduino reservations, AVR integration and application entry glue | arduino, avr, core, peripherals | Arduino AVR core plus validated compiler package |
| `grevir-arduino-esp32` | `GrevirArduinoESP32.h` / Grevir Arduino ESP32 | Named ESP32 board binding, Arduino reservations and ESP32 integration | arduino, esp32, core, peripherals | Selected Arduino-ESP32 core |
| `grevir-encoder` | `GrevirEncoder.h` / Grevir Encoder | Quadrature decoder/scaling/module wrapper | base, time, core, peripherals | Caller supplies pins/clock |
| `grevir-stepper` | `GrevirStepper.h` / Grevir Stepper | Stepper phases, movement state and module wrapper | base, time, core, peripherals | Caller supplies pins/clock |
| `grevir-fastled` | `GrevirFastLED.h` / Grevir FastLED | FastLED strip integration and resource claims | base, core | FastLED; remove the currently unused time include |
| `grevir-pulse-codec` | `GrevirPulseCodec.h` / Grevir Pulse Codec | PWE bit collection, waveform parameters, encoder and decoder | base, time | None; no Arduino or GPIO dependency |
| `grevir-pulse-io` | `GrevirPulseIO.h` / Grevir Pulse IO | PWE pin/timing transmitter and receiver modules | base, time, core, peripherals, pulse-codec | Caller supplies pins/clock |
| `grevir-packet` | `GrevirPacket.h` / Grevir Packet | Packet fragments, reassembly, manager and sender | None initially | `<array>`, `<functional>`, `<cstring>`, `<cstdint>`; actual AVR availability must be proved |
| `grevir-test-support` | Host include tree; no `library.properties` | Native Linux/macOS/Windows fixture, controlled clock/events, mock I/O, register storage and assertions | Development dependencies only: base, time, registers, core | Host C++ runtime/test runner; target models remain with their backends |
| `grevir-tools` | Python package/CLI; no `library.properties` | Hardware-data extraction, generation and eventual IRQ binding generation | No target runtime dependency | Versioned generator inputs/tool dependencies |
| `grevir-platforms` | Board/toolchain distribution; no `library.properties` | Arduino platform recipes, compiler versions, board settings, indexes and integration hooks | Build/distribution dependencies, not C++ library dependencies | Maintained upstream/vendor compilers and cores |

The package dependency graph must remain acyclic. In particular:

- `base`, `time`, `registers`, `core` and `peripherals` never include an MCU backend
  or Arduino adapter. Their behavior is expressed through injected policies/types.
- Backends implement portable contracts. They do not import an application or
  determine which user's modules exist.
- `grevir-arduino` does not auto-include both target adapters. A composition root
  selects `GrevirArduinoAVR.h` or `GrevirArduinoESP32.h`; reusable modules stay the
  same. A platform-generated selection header can be added later if useful.
- FastLED, encoder and stepper are independent siblings. Their combination examples
  may require each other; that does not make them unconditional runtime dependencies.
- `grevir-packet` is not a Wi-Fi/UDP abstraction. The current file implements
  fragmentation/reassembly; network transport belongs to a future integration.
- Host tests may depend on test support. No installed target library depends on it.

Do not create a new `grevir-all` dependency umbrella in the initial extraction.
The workspace coordinates tested combinations; users install only the closure
they need. Keep all AVR fact headers in one backend repo initially, rather than
maintaining hundreds of near-empty device repositories.

## Arduino-compatible checkout contract

Arduino permits hyphens in library directory names. Library metadata belongs at
the root; modern-format sources live under `src/`, whose compilable source tree is
recursive. `depends` identifies libraries by their metadata names and supports
Library Manager/CLI installation; cloning a repo does not invoke that installation.
`architectures` participates in selection/warnings and is not a source-exclusion
mechanism. [Arduino library specification](https://docs.arduino.cc/arduino-cli/library-specification/)

Use this layout for every runtime repo, with no enclosing `arduino/` folder:

```text
grevir-encoder/
  library.properties
  README.md
  LICENSE...                         # audited notices for this component
  CMakeLists.txt                     # same sources; optional Arduino consumers
  src/
    GrevirEncoder.h                 # unique Arduino discovery/entry header
    grevir/encoder/encoder.hpp
  examples/
    BasicEncoder/BasicEncoder.ino
  tests/                             # never nested below src/
  extras/                            # historical/reference material
  dev-docs/
```

Grevir's conventions:

1. Use `grevir-xxx` for repository/folder identity, `Grevir Xxx` for Arduino metadata,
   `GrevirXxx.h` for the unique root header, and `grevir/<component>/...hpp` for
   implementation/public detail paths. Hyphenated component path segments become
   underscores, e.g. `grevir/pulse_codec/decoder.hpp`. These are separate naming
   surfaces with an explicit one-to-one mapping.
2. Namespace the new API under `grevir`, with component namespaces where useful.
   Track old symbol names in the ledger. Make namespace/API renaming a separate
   reviewed step from behavior-preserving relocation. Legacy compatibility aliases
   are optional migration work, not a required dependency of every package.
3. Give each package a small entry header. Optional facilities, such as EEPROM,
   stay in explicitly included feature headers so the entry header does not require
   every optional provider. Include another package's unique root
   header at the package boundary before including its nested headers, so Arduino
   can discover its include root. Internal includes remain within that repo.
   Check this in a clean sketchbook rather than relying on GWZ include paths.
   Arduino recursively resolves included library dependencies during its build.
   [Arduino sketch build process](https://docs.arduino.cc/arduino-cli/sketch-build-process/)
4. Metadata lists actual direct dependencies using names from the table. Record
   separately any extra dependencies needed by a particular example. The `includes`
   field selects the IDE's inserted entry header; it is not a substitute for actual
   C++ includes or a dependency resolver.
5. Use `architectures=*` for genuinely architecture-neutral packages, `avr` for
   the AVR packages and `esp32` for ESP32 packages, matched to the selected platform
   identifiers. State tested devices and compiler/library requirements separately.
   Do not claim every MCU is supported because a raw constants header exists.
6. Use explicit target checks around target-specific sources. Keep host mocks,
   test runners, experiments and module interface experiments outside production
   `src/`. In particular, do not reproduce `ardOStepper/src/devel/`.
7. Commit generated hardware facts. Ordinary builds must not regenerate them.
   Keep templates and inline definitions visible to consumers; avoid prebuilt-only
   implementations that obstruct specialization and whole-application optimization.
8. Every sketch directory and primary `.ino` stem agree, including the legacy
   `FixedFreqencyCounter` typo correction. Provide one small runnable example per
   published package, even when larger combination examples remain historical.

The resulting consumer installation is:

```text
<configured sketchbook>/libraries/
  grevir-base/
  grevir-time/
  grevir-registers/
  grevir-core/
  grevir-peripherals/
  grevir-avr/
  grevir-arduino/
  grevir-arduino-avr/
  grevir-encoder/                     # if the sketch uses it
```

This is an illustrative Uno/Nano + encoder closure. The corresponding ESP32 closure
substitutes `grevir-esp32` and `grevir-arduino-esp32`, and needs no AVR package or
`grevir-registers` unless a selected implementation actually uses it. A pure pulse
codec installation needs only base, time and pulse-codec. Packet reassembly can
stand alone where the required standard headers exist.

Use the **configured sketchbook**, not the Arduino application installation or a
hardcoded Documents path. Development checkouts can live under `grevir-wz`; consumer
checkouts can instead be cloned directly under `libraries`. Prove the latter
without symlinks, absolute include paths, a workspace checkout or a special script.

### Compiler integration is a separate installation

A library layout cannot change the board platform's C++ compile recipe. The chosen
compiler and `-std=` mode belong to the platform/toolchain installation maintained
in `grevir-platforms`; Arduino documents those recipes and tool properties in its
[platform specification](https://docs.arduino.cc/arduino-cli/platform-specification/).
Cloning libraries is sufficient **after** that supported environment is installed.

Retain Gianni's direction: use the most advanced common C++ capability that can be
supported on AVR and ESP32 with maintained compilers, without carrying GCC patches.
C++23 is a candidate to evaluate, not a measured baseline in this document. Probe
language features and standard headers independently. Do not quietly reinstate
C++14 to fit stock Arduino packages. Board integration/toolchain packaging work is
acceptable; a private compiler fork is outside the chosen direction.

Keep normal headers as the initial delivery surface. C++ modules and later
reflection experiments can live in opt-in build experiments until both target
compilers and Arduino integration support them. They must not make a clone require
an undocumented preprocessing/build step.

## Required splits before the dependencies can be true

### 1. Separate composition from pins and Arduino services

`ardOinus.h` currently combines GPIO interfaces/wrappers with conflict checking
and `Application`. Put GPIO, open-drain variants and debounce in
`grevir-peripherals`; put claim validation, closure and application lifecycle in
`grevir-core`. A module's resource bindings remain template/type inputs, including
an explicit identity when two instances would otherwise have the same specialization.

`ardo_sys_defs.h` is a second boundary, not a portable core header. Its board/inventory
primary traits belong to core; its calls to Arduino pin/time/serial/PWM functions
belong to the common Arduino adapter; target selection belongs to the selected
Arduino target adapter. Remove automatic mock activation based on `!INPUT_PULLUP`
from production paths and make it an explicit host-test choice.

`ardo_eeprom.h` has two owners: the storage resource/claim contract belongs to
peripherals, while its current `EEPROM.read/update` implementation belongs to the
Arduino adapter. Keep EEPROM availability explicit; do not unconditionally include
an EEPROM provider in all Arduino builds. On 21 September the portable typed
wrapper was extracted with explicit byte-storage binding and capacity/resource
checks. Its `EEPROM.update(..., &addr)` bug was reproduced against a strict byte
backend and corrected to send the byte value. The Arduino adapter remains planned.

### 2. Extract actual portable register machinery

Split `setl_bit_fields.h` into mapping, value types, fields, register access,
selection and application operations. The ledger names the defining API groups.
`DebugMcuRegister` moves to test support. Architecture-specific access/barrier
behavior must be supplied explicitly, rather than pulled into portable machinery
through `setl_system.h`.

On 21 September mapping, typed values, fields, access, selection and multi-register
application were extracted into `grevir-registers` with explicit policies and
preserved operation ordering. Shared memory and the adapted `DebugMcuRegister`
now live in Test Support. The remaining portable legacy exercises now assert their
results; the full test mapping stays pending for timer/device-specific exercises.
The first AVR register/GPIO layer passes native mocked access checks. Target
compilation, actual barriers and device side effects have not been validated.

`setl_system.h` currently defines `System` only for a limited macro set, and its
Xtensa barriers are empty. Separate portable diagnostic/policy contracts from AVR
and ESP32 behavior. Preserve observable access semantics while extracting; separately
specify and test compiler ordering, volatile MMIO, interrupt atomicity and ESP32
concurrency where needed. An AVR `_NOP()` and a no-op Xtensa function do not establish
a general synchronization guarantee.

`setl_bit_io_defs.h` is AVR-specific despite its generic filename. Keep it with
`ardo_avr_bit_io_defs.h` in `grevir-avr`, compare their address-offset conventions,
and consolidate only after equivalence or intentional differences are established.

### 3. Separate resource topology from AVR access

`ardo_avr_base_register.h` contains both low-level register access and a broadly
useful resource graph. Move `ResourceType`, `Dependency`, `RootDependencies`,
`ResourceFinder` and their helpers to `grevir-core/resource_graph.hpp`; keep actual
AVR access in `grevir-avr/register.hpp`. Device pin/mux/peripheral relationships
instantiate that generic graph in the device backend. Mock register storage now lives
in Test Support, selected by an injected access policy. The first `grevir-avr`
increment extracts this register binding, explicit memory/I/O offsets and the
consolidated GPIO configuration/wrapper code. Core already owns the generic graph.
Dynamic output configuration now follows the typed PORT-before-DDR ordering;
host traces reproduce the old mismatch and verify the correction. Concrete device
inventories and target/Arduino bindings remain planned.

`ardo_timers.h` also needs a real split. Portable frequency/resolution requests and
timer-selection interfaces belong to peripherals; generic allocation belongs to
core; AVR waveform/top-count options belong to the AVR backend. The present
`SelectionResolver` is a **placeholder**, not an implemented global allocator.
Replace the shared enum's fixed AVR-specific entries with an extension mechanism
without making portable code import AVR. Preserve the current filter behavior in
extraction tests, then make unsupported mandatory requirements a diagnosed error
in the separately implemented allocator. The 21 September requirements increment
now preserves the explicit filter and adds `CheckedTimerConfig<Backend, Config>`
for mandatory request validation against a supplied backend. This does not implement
`TimerSelector`, board inventory binding or global allocation; those remain planned.

### 4. Split the manual ATmega328P implementation by ownership

The handwritten [`ardo_supplemental_atmega328p_dev.h`](../../ardoinus/ardOinus/src/sys/mcu/avr/ardo_supplemental_atmega328p_dev.h)
is 2,843 lines. It is not just generated data. Its template computations and
semantic bit-field wrappers are central to the project.

| API group in that file | Canonical owner and destination, relative to `src/grevir/` |
| --- | --- |
| Divider mappings and clock/prescaler computations | `grevir-avr`: `avr/timer/clock.hpp` |
| Timer/PWM/top mode selection, waveform algorithms | `grevir-avr`: `avr/timer/mode.hpp` |
| Capture/noise/output-compare abstractions, top register selection, timer definition | `grevir-avr`: `avr/timer/definition.hpp` |
| Compile-time timer configuration computations and settings | `grevir-avr`: `avr/timer/configuration.hpp` |
| Timer output pins, PWM pin settings and runtime application of computed settings | `grevir-avr`: `avr/timer/output.hpp` |
| Reusable AVR port access/direction operations | `grevir-avr`: `avr/gpio.hpp` |
| CS/COM/WGM encodings, their traits and timer register/field aliases | `grevir-avr`: `avr/devices/atmega328p/timer_fields.hpp` |
| Device GPIO register aliases and port bindings | `grevir-avr`: `avr/devices/atmega328p/gpio_fields.hpp` |
| Remaining device-specific register/bit-group aliases | `grevir-avr`: `avr/devices/atmega328p/peripheral_fields.hpp` |
| Concrete `Timer0`, `Timer1`, `Timer2` and full MCU timer inventory | `grevir-avr`: `avr/devices/atmega328p/timers.hpp` |
| Concrete pin/peripheral dependencies and `McuResources` | `grevir-avr`: `avr/devices/atmega328p/resources.hpp` |
| `ArduinoNanoPinmap`, `ArduinoUnoPinmap` and their mappings | `grevir-arduino-avr`: `arduino_avr/boards/nano.hpp`, `arduino_avr/boards/uno.hpp` |
| Arduino `safe_to_use` inventory/reservations | `grevir-arduino-avr`: `arduino_avr/timer_policy.hpp` |

The 2,006-line `ardo_avr_base_timer.h` hardcodes ATmega328P data and overlaps this
manual header. It is not automatically a clean generic backend. Compare the two
implementations per API group and establish one canonical implementation with
explicit MCU traits. Keep historical copies outside `src/` while reconciling.

The first clock group is now consolidated in `avr/timer/clock.hpp`: both legacy
copies matched after whitespace normalization. Native checks exposed and corrected
non-exact selector lookup and truncated divider requirements; explicit traits,
checked arithmetic and compile-time mapping diagnostics are included. Eleven
legacy assertions and computed mock-register writes pass. The matching waveform
group is also consolidated in `avr/timer/mode.hpp`, with caller-provided tables,
compile-time filtering/exact TOP selection and runtime optional metadata lookup.
ATmega328P tables remain host fixtures. The matching reusable definition group is
now in `avr/timer/definition.hpp`: capture/control metadata, TOP source access and
timer composition with optional explicit mode traits. Configuration, concrete device
encodings/timer inventory and output application remain planned.
AVR compiler and hardware validation are on hold.

The GPIO base header has a similar overlap. Do not discard either implementation
merely because names look duplicated.

Keep raw register addresses and `constexpr` constants in
`grevir-avr/src/grevir/avr/generated/<mcu>.hpp`; keep manually authored semantics in
`devices/<mcu>/`. This preserves the original principle: typed symbols usable as
template arguments, semantic field access and compile-time shift/mask calculation.
Preprocessor use remains limited to unavoidable include/platform boundaries.

Initial validated board candidates are Uno ATmega328P, Nano ATmega328P and its
old-bootloader variant. The latter changes upload configuration, not silicon
semantics. Reconcile board-header facts with the manual pin maps, including Nano's
analog-only pins, before claiming a complete mapping. The other 35 board headers
and 301 device development layers remain inventoried historical inputs; their
presence is not a support promise.

### 5. Keep codecs, drivers, tests and applications distinct

`pwe_serial.h` becomes a hardware-independent pulse codec. `pwm_serial_comms.h`
becomes the separate pin/timing module integration. `ardo_packet_reassembler.h`
is similarly transport-independent and splits by header format, reassembler,
manager and sender; retain and measure its real standard-library requirements.

Encoder and stepper each keep their decoder/state machine and module wrapper in
one useful component repository. Their current headers are sufficiently focused
for initial extraction. FastLED gets its own integration repository; its existing
resource claim around the FastLED singleton must not disappear during packaging.

Example ownership follows the API demonstrated, not the first board used to test
it. `Arrays`, `BareMinimum`, `BetterBlink`, `Blink`, `DependentModules`,
`DigitalInputSerial` and `Serial` belong in `grevir-arduino/examples/`. They use
common composition/GPIO/time/serial APIs. Keep target-adapter selection and valid
board pin/serial bindings in their example configuration; test the shared logic
on AVR and ESP32. These example-only bindings add no target-backend dependency to
the common library. `FixedFrequencyCounter` remains in `grevir-arduino-avr` because
it explicitly uses AVR timer types, waveform modes and register fields.

Tests follow their implementation owner. Shared mocks/assertions live in the
host-only test-support repo. The old combined test runner references a missing
`tests/modules_test.h`, so retain it as evidence and create independent runners;
do not report its mere relocation as passing tests.

Keep `ParkLightsV2` under FastLED's `extras/legacy/examples/` initially: it is a
substantial ESP32 application with network/display dependencies and a missing
local credentials header. Its three C++ files and image generator are accounted
for, but it is not the first minimal public example. Treat `gammil_repeater.cxx`
similarly under pulse I/O. Their eventual ports are separate application work.

### 6. Run AVR and ESP32 tests in ordinary host processes

The [build and test framework plan](GrevirBuildAndTestPlan.md) records the selected
CMake, Ninja, CTest and host-only Catch2 stack, accepted by Gianni on 18 September
2026. His subsequent clarification narrows the immediate step to compiling with
Xcode Clang and minimal mock declarations; neither MCU compilers nor a behavioral
fixture is required now. The broader plan defines independent repo builds,
workspace presets, compiler-failure
checks and the real-target/Arduino validation layers supporting this contract.

Clarified by Gianni on 18 September 2026: the mock fixture must let us build and
run **AVR and ESP32 tests as native Linux, macOS and Windows processes**, without
an attached MCU. This includes backend configuration logic and composed application
behavior, as well as the portable utilities. Both simulated targets must be
selectable on every supported host OS.

Compile the production C++ logic with the host compiler, supplying mock hardware
access and SDK/service boundaries. Keep host OS selection separate from the MCU
being modeled: an ATmega328P test running on macOS is still a macOS executable.
Do not define `__AVR__` or `__XTENSA__` to impersonate a target compiler. Isolate
target instructions, ISR attributes and native SDK entry points in small binding
layers, so the same configuration, allocation and module logic can use a host
fixture. Host tests must exercise that logic rather than a second implementation
of the algorithm under test.

The existing `mock_arduino.h`, `sys/ardo_sys_mock.h`, `DebugMcuRegister` and mock
register storage are starting material. The Arduino mocks contain IDE-oriented
stubs, including zero-valued time/input functions; those alone do not meet this
runtime fixture requirement. Their existing destinations in the file map remain
valid, with the following new facilities built around them:

| Facility | Owner and proposed file | Required behavior |
| --- | --- | --- |
| Fixture lifecycle | `grevir-test-support/include/grevir/test/fixture.hpp` | Explicit setup/reset/teardown; reproducible initial state and no state leakage between tests |
| Controlled clock | `grevir-test-support/include/grevir/test/clock.hpp` | Advance simulated time explicitly; exercise wraparound and deadlines without wall-clock sleeps |
| Event/interrupt control | `grevir-test-support/include/grevir/test/events.hpp` | Schedule and deliver modeled events deterministically, including interrupt enable/mask/pending behavior where used |
| GPIO fixture | `grevir-test-support/include/grevir/test/gpio.hpp` | Inject input levels/edges; inspect output transitions and pin modes |
| Serial and storage fixtures | `grevir-test-support/include/grevir/test/serial.hpp`, `storage.hpp` | Inject/capture bytes and inspect/reset EEPROM-like state; support relevant failure cases |
| Register access fixture | Existing planned `grevir-test-support/include/grevir/test/register_memory.hpp` | Inspect reads/writes and inject peripheral events through a modeled access policy |
| Initial AVR device model | `grevir-avr/tests/support/atmega328p_fixture.hpp` | Compose shared fixture primitives with ATmega328P register/peripheral semantics and actual backend logic |
| Initial ESP32 device model | `grevir-esp32/tests/support/esp32_fixture.hpp` | Compose shared primitives with the named chip's capabilities and fake SDK/service boundary; expand per supported peripheral |
| Arduino integration fixtures | `grevir-arduino-avr/tests/support/fixture.hpp`, `grevir-arduino-esp32/tests/support/fixture.hpp` | Exercise board mappings, reservations and common Arduino adapters against the selected device model |

Shared fixture primitives stay independent of concrete MCU backends. Device models
live with the corresponding backend and consume test support as a development
dependency; integration tests may consume those models through test-only build
targets. MCU-specific register side effects, such as clear-on-write flags, belong
to the device model, not the generic memory array. Add models as the corresponding
peripheral becomes supported, with observable effects and limitations documented.

Provide ordinary host build/test targets and run the same applicable suites in a
Linux/macOS/Windows CI matrix. No Arduino IDE, board upload or target compiler is
needed for the native fixture tests. Keep target integer widths, register layouts
and arithmetic assumptions explicit so the host ABI does not silently substitute
for MCU behavior. Preserve compile-fail resource/allocation tests alongside runtime
tests. A shared behavior test should be reusable with AVR and ESP32 fixtures;
register-specific assertions remain target-specific.

This is a model of the hardware/service boundary, with the C++ rebuilt for the host.
Instruction execution, electrical behavior, cycle accuracy and real ESP32 concurrent
scheduling remain outside what these fixtures establish. Target compile/link checks
and focused hardware tests still validate those aspects. Interrupt fixture tests
exercise handler logic and modeled delivery; target builds validate the actual ISR
ABI and generated vector bindings.

## Ownership for new work

These paths are proposed **new** files, not existing implementations hidden in the
legacy tree. Add them only as their corresponding feature is implemented.

| New API/work | Owner and proposed location | Boundary |
| --- | --- | --- |
| Generic compile-time allocator and diagnostics | `grevir-core/src/grevir/core/allocation.hpp`, `allocation_diagnostics.hpp` | Solve requests against an injected capability graph; no timer-register encodings here |
| Repeated module identity/binding | `grevir-core/src/grevir/core/module_instance.hpp` | Separate logical instance identity from a shared module implementation |
| Boot selection between application roots | `grevir-core/src/grevir/core/application_set.hpp` | Proposed feature, pending semantics; validate each root and shared boot services, not a false simultaneous conflict across mutually exclusive roots |
| Generic timer/GPIO/UART/I2C/SPI/ADC contracts | `grevir-peripherals/src/grevir/peripherals/{timer/requirements,gpio/requirements,uart,i2c,spi,adc}.hpp` | State required capabilities and observable behavior; reject unsatisfied mandatory requirements |
| Backend capability inventories/drivers | `grevir-avr/src/grevir/avr/devices/atmega328p/` and `grevir-esp32/src/grevir/esp32/devices/<selected-chip>/` | Physical instances, pin routing, sharing constraints, reserved resources and realizable configurations |
| Interrupt request and handler binding contract | `grevir-peripherals/src/grevir/peripherals/interrupt.hpp` | Typed generic requests; resource ownership remains in core |
| MCU interrupt ABI/vector integration | `grevir-avr/src/grevir/avr/interrupts.hpp`, `grevir-esp32/src/grevir/esp32/interrupts.hpp` | Vector names, attributes, dispatch and critical-section rules stay target-specific |
| Generated application interrupt bindings | `grevir-tools/src/grevir_tools/interrupts.py`; output in the consuming application's generated build area | Program-specific C++ must not be written into a shared installed library |
| Build integration for generated bindings | `grevir-platforms` recipe/hooks | Ordinary examples require no manual generation; Arduino integration must own any required step |
| Initial ESP32 GPIO/timer backend | `grevir-esp32/src/grevir/esp32/{gpio,timer,capabilities}.hpp` | New substantive implementation; the legacy 24-line Arduino header is not this backend |
| Named ESP32 Arduino board policy | `grevir-arduino-esp32/src/grevir/arduino_esp32/boards/<selected-board>.hpp` | Keep board pin defaults and Arduino/SDK reservations distinct from MCU facts |

The initial ESP32 proof should use a named Xtensa chip/board and exact SDK/core.
RISC-V can become another backend within the ESP32 repo once the first cross-MCU
contract is proved. Neither Arduino's `esp32` architecture label nor the family
name establishes instruction-set or peripheral equivalence.

A library needing a timer declares its requirements and receives an allocated
binding. The global application assembly sees dependent modules, reservations and
all resource claims. MCU backends supply candidates. This supports whole-application
optimization without forcing a single monolithic source repository.

## Generators and supporting files

| Existing input | Proposed owner/destination | Treatment |
| --- | --- | --- |
| Workspace `dev-docs/avr_api_gen.py` | `grevir-tools/extras/legacy/avr_api_gen.py` initially | Preserve the original; compare provenance, inputs and output format before selecting canonical generator code |
| `ardoinus/ardoExtract/extract.py` | `grevir-tools/extras/legacy/extract.py` initially | Preserve independently; do not assume equivalence with the other generator |
| Canonical generator after comparison | `grevir-tools/src/grevir_tools/avr.py` | Record input versions, generation options and deterministic output checks |
| `ardOinus/examples/ParkLightsV2/pklt.py` | `grevir-fastled/extras/legacy/examples/ParkLightsV2/pklt.py` | Example image tooling, not a hardware-constants generator |
| Five old `library.properties` files | Respective owner's `extras/legacy/`; `ardOinus` manifest in core | Replace with metadata for each new library; do not preserve incorrect includes/dependency omissions as the new contract |
| `ardOinus/devel/make/Makefile.base` and old Visual Studio projects | `grevir-test-support/extras/legacy/ardOinus/` for shared infrastructure; encoder/stepper owner for their projects | Historical build references, outside installed source compilation |
| `ardoExtract/ardOinus.code-workspace` | `grevir-tools/extras/legacy/ardOinus.code-workspace` | Historical generator workspace reference |
| Original root README | `grevir-wz/dev-docs/legacy/ArdoinusREADME.md` if copied during extraction | Historical context; each new library gets a focused README |
| Original `LICENSE` and `LICENSE.txt` | Component-specific notices after provenance review | Root contains both LGPL and MIT texts; establish which applies to each moved source before publication |
| Development notes currently in this workspace | `grevir-wz/dev-docs/` | Shared principles, plans, compatibility matrix and cross-repo decisions |

Do not relocate the preserved generator merely as a side effect of writing this
plan. During actual extraction, preserve its hash and history/provenance. For new
GWZ members use `gwz repo ...`; never hand-edit `gwz.conf`. Commit/push/tag operations
are separate user instructions, not implicit consequences of planning or extraction.

## Phased implementation plan

Each numbered step is one goal with an **aspirational budget below 500 new or
substantively changed lines**. Mechanical moves, generated data and retained legacy
snapshots are counted separately. A repeated step means one independent change per
named component or API group, not one enormous batch hidden behind a small budget.
If a group needs more logic, divide it at a tested contract before proceeding.

### Phase 1 — A reproducible consumer baseline

Milestone: the directory/metadata/toolchain contract is demonstrated with a minimal
library and a documented compiler capability matrix.

| Step | One goal | Approximate authored LOC | Completion evidence |
| --- | --- | ---: | --- |
| 1.1 | Preserve this inventory and establish per-component provenance/notices | <150 | Hashes and all ownership decisions accounted for; license questions resolved before affected publication |
| 1.2 | Probe required C++ features and standard headers on selected AVR compiler | <250 | Compiler version, target, flags, headers and compile/link results recorded |
| 1.3 | Run the same capability probe on selected ESP32 Xtensa compiler | <250 | Comparable results; no assumption that language support implies library support |
| 1.4 | Demonstrate root-layout Arduino dependency discovery with a minimal two-library consumer | <200 | Clean sketchbook build from direct clones; no workspace include paths |
| 1.5 | Define the reproducible compiler/platform recipe for each target, one target per step | <400 each | Selected standard mode used for sketch and library translation units; working compile/link/upload configuration |

Steps 1.2–1.4 can proceed independently. The common baseline decision follows the
measured target results. Packaging a new board toolchain can require more than one
increment; split acquisition, recipe and distribution work rather than hiding it
inside a compiler-flag change.

### Phase 2 — Independently usable portable libraries

Milestone: base, time, registers and pure codecs build outside Ardoinus and outside
the GWZ workspace, with their existing relevant tests migrated.

| Step | One goal | Approximate authored LOC | Completion evidence |
| --- | --- | ---: | --- |
| 2.1 | Establish the common `grevir-base` compatibility/memory/diagnostic contracts | <400 | No target backend or Arduino includes; one coherent standard-header configuration |
| 2.2 | Extract one cohesive base utility group per change, following the ledger | <250 each plus moves | Existing tests retained; standalone consumer includes work |
| 2.3 | Extract `grevir-time` | <250 plus moves | Unit conversion/wraparound tests and isolated build |
| 2.4 | Extract one register-engine group per change, with access policy explicit | <350 each plus moves | Existing mapping/field tests and modeled register writes agree |
| 2.5 | Extract host test support and each required runner independently | <300 each plus moves | Native Linux/macOS/Windows runners; target artifacts do not include host streams/globals/tests |
| 2.6 | Extract pulse codec, then packet codec as separate changes | <300 each plus moves | Independent codec tests; standard-library requirements measured on target |
| 2.7 | Add each library's public header, metadata and minimal example | <200 each | Clean Arduino and non-Arduino consumer build using only declared dependencies |
| 2.8 | Implement each deterministic fixture primitive separately: lifecycle, clock, events, GPIO, serial, storage and register observation | <350 each | Repeatable injected inputs/observed effects and reset between tests; no wall-clock sleeps |
| 2.9 | Establish the Linux/macOS/Windows native test matrix | <250 | Common host suites compile and run on all three operating systems |

Time and codec work can progress once the small base contracts they use are stable;
register extraction does not need the application framework. Do not rewrite all
metaprogramming as part of the moves. Modernization is a later, measured change.

### Phase 3 — Portable composition and peripheral contracts

Milestone: a host-simulated application composes multiple module instances, detects
resource conflicts and exercises GPIO/time wrappers without Arduino or an MCU repo.

| Step | One goal | Approximate authored LOC | Completion evidence |
| --- | --- | ---: | --- |
| 3.1 | Extract claims and resource topology from their current owners | <350 plus moves | Positive and expected-failure compile cases preserve conflict rules |
| 3.2 | Extract module dependencies, lifecycle and application composition | <350 plus moves | Dependency initialization/loop behavior and storage identity tests |
| 3.3 | Establish the injected GPIO/clock contract used by portable wrappers | <400 | Synthetic backend implements it without Arduino headers |
| 3.4 | Extract each GPIO, debounce, button, polling, sequencing or PWM unit separately | <250 each plus moves | Existing behavior validated against explicit mock backend |
| 3.5 | Separate portable timer request vocabulary from AVR extensions | <400 | Portable build contains no AVR enums/includes; unsupported-request policy explicit |
| 3.6 | Make repeated module instance identity explicit | <400 | Two instances have distinct state/bindings; intentional sharing still checked |

The full allocator and interrupt generator are not prerequisites to releasing
existing checked manual bindings. Keep their absence explicit in the API docs.

### Phase 4 — First complete extracted Uno/Nano integration

Milestone: Arduino Uno/Nano applications use Grevir composition, GPIO and existing
AVR timer functionality through directly installed libraries.

| Step | One goal | Approximate authored LOC | Completion evidence |
| --- | --- | ---: | --- |
| 4.1 | Separate generated AVR facts from manual device semantics | <200 tooling plus moves | All 302 raw files mapped; hashes/provenance retained; normal builds need no generator |
| 4.2 | Extract AVR register access and GPIO | <350 plus moves | Register access/port behavior checked, including address offsets |
| 4.3 | Reconcile one timer API group per change between the two legacy implementations | <400 each plus moves | Same requested modes produce expected register fields and dynamic updates |
| 4.4 | Establish ATmega328P physical resource inventory | <400 plus moves | Pin/peripheral/timer relationships and conflicts tested independently of Arduino numbering |
| 4.5 | Extract one shared Arduino service adapter per change | <250 each plus moves | Pin/time, serial, PWM and optional EEPROM consumer tests |
| 4.6 | Establish Uno/Nano board maps and Arduino reservations | <350 plus moves | Board facts audited; reserved core timer cannot be allocated accidentally |
| 4.7 | Port each small legacy Arduino example in its API owner's repo | <200 each plus moves | Common examples stay in grevir-arduino; initial Uno/Nano build passes, followed by ESP32 validation in Phase 5 |
| 4.8 | Compose the ATmega328P native fixture, adding one peripheral model per increment | <400 each | AVR backend and Arduino AVR integration tests execute in normal processes on Linux, macOS and Windows |

Generated fact headers and coherent device tables may exceed the hand-authored file
size target; record them as explicit data exceptions. Split substantial handwritten
algorithms by responsibility. Preserve all attributes, scopes and conditional
boundaries during moves; inspect disabled branches with a syntax-aware source check.

### Phase 5 — Early ESP32 proof and reusable modules

Milestone: the same portable module example runs on one named AVR board and one
named ESP32 Xtensa board with only composition-root bindings changed. Useful drivers
are separately installable.

| Step | One goal | Approximate authored LOC | Completion evidence |
| --- | --- | ---: | --- |
| 5.1 | Implement the selected ESP32 chip's initial GPIO/clock backend | <450 | Named hardware/SDK test and explicit concurrency policy |
| 5.2 | Implement one ESP32 timer capability/configuration path | <450 | Same declarative requirement used for the AVR comparison |
| 5.3 | Add selected ESP32 Arduino board policy and reservations | <350 | Installed build uses no AVR package; no assumed legacy UART pin defaults |
| 5.4 | Extract encoder, stepper, FastLED and pulse I/O, one component per step | <350 each plus moves | Independent consumer examples with only real runtime dependencies |
| 5.5 | Establish a cross-MCU example and optimization comparison | <250 | Identical reusable module source; map/disassembly evidence for static configuration and measured flash/RAM |
| 5.6 | Compose the selected ESP32 native fixture, adding one SDK/peripheral boundary per increment | <400 each | ESP32 backend and Arduino ESP32 integration tests execute on all three host operating systems; reuse applicable AVR behavior tests |

The ESP32 contract experiment should start alongside Phase 4 once Phase 3 contracts
are available; do not wait for every AVR peripheral to be finished. Driver extraction
can also proceed independently once those contracts stabilize.

### Phase 6 — Allocation and a complete peripheral story

Milestone: a composed Uno/Nano application requests timers and other peripherals,
gets valid bindings at compile time, and reports useful errors when requests cannot
be satisfied; the same contracts have an ESP32 implementation proof.

| Step | One goal | Approximate authored LOC | Completion evidence |
| --- | --- | ---: | --- |
| 6.1 | Implement candidate filtering and deterministic allocation over a small synthetic capability graph | <450 | Solvable, unsatisfiable, shared and reserved resource compile cases |
| 6.2 | Bind AVR timer candidates and module requirements to that allocator | <450 | Multi-module timer selection and transitive pin/channel conflicts diagnosed |
| 6.3 | Bind the initial ESP32 timer candidates to the same allocation contract | <400 | Cross-MCU case proves the solver does not encode AVR assumptions |
| 6.4 | Add each UART/I2C/SPI/ADC capability contract independently | <350 each | Synthetic-backend contract checks |
| 6.5 | Implement each AVR peripheral backend independently | <450 per coherent driver increment | Host register model plus hardware exercise and interaction/collision tests |
| 6.6 | Define interrupt binding contract and implement one target/vector path per change | <400 each | Correct handler ABI, lifecycle, reservations and conflict reporting |
| 6.7 | Add generated interrupt glue/build integration if the compiler-only approach cannot express the binding | <400 per generator or platform increment | Normal Arduino application build generates/links the correct per-application glue |

Boot-selected multiple applications remains a distinct design experiment after
initialization, shared services and interrupt ownership semantics are agreed. It
belongs to core if adopted; it should not delay extracting useful libraries.

### Phase 7 — Reproducible independent releases

Milestone: each ready component has a repeatable artifact, accurate dependencies and
an installation path demonstrated outside the contributor workspace.

| Step | One goal | Approximate authored LOC | Completion evidence |
| --- | --- | ---: | --- |
| 7.1 | Establish each ready repo as a GWZ member through the CLI | <100 metadata each | Workspace membership records agree; no manual config edits |
| 7.2 | Validate one direct-clone dependency closure per change | <250 each | Fresh sketchbook, selected compiler platform, case-sensitive include check and no source/test leakage |
| 7.3 | Add each additional package-channel adapter independently | <300 each | Same sources consumed through the packaging-plan channel with accurate dependency/version mapping |
| 7.4 | Record a tested release set and consumer requirements | <150 | Reproducible versions/hashes plus language, standard-header and target matrix |

Create member repos when their earlier milestones need them; 7.1 is the repeatable
membership procedure, not a requirement to defer all repository creation until the
end. Phase 7 can be applied to each mature component as it becomes ready. Publishing,
commits, tags and pushes follow explicit instructions separately.

## Acceptance checks for extraction

- Every inventoried source has a destination or an explicit historical disposition;
  mapped source hashes are checked before applying moves.
- Installed production dependencies form the documented acyclic graph. Portable
  libraries compile without backend headers or Arduino macros.
- Direct clones into a clean sketchbook work with the documented dependency closure
  and compiler platform; one target does not install the other target backend.
- Public root headers, metadata names and actual filename case agree. Library source
  compilation excludes tests, legacy experiments and host-only translation units.
- AVR and ESP32 backend/application suites run as native Linux, macOS and Windows
  processes using explicit fixtures. Controlled time, I/O and interrupt events are
  repeatable, fixture state resets between tests, and production C++ logic is shared
  with target builds. All six host-OS/MCU-family combinations are covered as the
  corresponding backend becomes supported.
- Existing resource conflict diagnostics survive extraction. New allocation tests
  distinguish capabilities, reservations, sharing, transitive dependencies and
  multiple instances.
- AVR timer configuration/register effects match validated reference cases; dynamic
  configuration still works where supported. Whole-application specialization and
  LTO results are measured without assuming object-file size equals final flash use.
- Standard-library fallbacks, disabled platform branches and brace/scope rules are
  checked. Modernizing them is deliberate work, not a side effect of file moves.

This planning pass validates inventory coverage and proposed dependency structure.
Compiler probes, source extraction and hardware/build acceptance belong to the
implementation milestones above.

Planning checks completed: all 736 source hashes match the surveyed checkout; the
796 mapping rows cover every inventoried file; the 16 runtime dependency nodes are
acyclic; AVR and ESP32 installation closures remain separate; 11 shared destination
paths are accounted for as explicit reconciliations; all local document links resolve.

## File-by-file appendix

The following tables expose the non-bulk mappings for convenient review. Paths in
the left column are relative to the Ardoinus checkout; right-column paths are
relative to the named destination repo. SQLite contains the fuller API-unit notes,
hashes, all 302 generated headers, all 301 retained development layers and all 38
board headers. A file shown more than once is intentionally split or preserved as a
historical reference alongside its extracted APIs.

### grevir-base

| Existing source | Planned destination | Action |
| --- | --- | --- |
| [ardOinus/devel/code/color_space_test.cxx](../../ardoinus/ardOinus/devel/code/color_space_test.cxx) | `tests/color_test.cpp` | adapt |
| [ardOinus/src/ardo_color_space.h](../../ardoinus/ardOinus/src/ardo_color_space.h) | `src/grevir/base/color.hpp` | extract |
| [ardOinus/src/circular_buffer.h](../../ardoinus/ardOinus/src/circular_buffer.h) | `src/grevir/base/circular_buffer.hpp` | extract |
| [ardOinus/src/setl_cyclic_int.h](../../ardoinus/ardOinus/src/setl_cyclic_int.h) | `src/grevir/base/cyclic_int.hpp` | extract |
| [ardOinus/src/setl_int_scaler.h](../../ardoinus/ardOinus/src/setl_int_scaler.h) | `src/grevir/base/int_scaler.hpp` | extract |
| [ardOinus/src/setl_integers.h](../../ardoinus/ardOinus/src/setl_integers.h) | `src/grevir/base/integers.hpp` | extract |
| [ardOinus/src/setl_optional.h](../../ardoinus/ardOinus/src/setl_optional.h) | `src/grevir/base/optional.hpp` | extract |
| [ardOinus/src/setl_support.h](../../ardoinus/ardOinus/src/setl_support.h) | `src/grevir/base/compat/config.hpp` | extract |
| [ardOinus/src/setl_system.cpp](../../ardoinus/ardOinus/src/setl_system.cpp) | `src/grevir/base/diagnostics.cpp` | adapt |
| [ardOinus/src/setl_system.h](../../ardoinus/ardOinus/src/setl_system.h) | `src/grevir/base/diagnostics.hpp` | split |
| [ardOinus/src/setl_system.h](../../ardoinus/ardOinus/src/setl_system.h) | `src/grevir/base/memory_policy.hpp` | split |
| [ardOinus/src/setl_templ_utils.h](../../ardoinus/ardOinus/src/setl_templ_utils.h) | `src/grevir/base/meta/type_algorithms.hpp` | extract |
| [ardOinus/src/setl_tuple_helpers.h](../../ardoinus/ardOinus/src/setl_tuple_helpers.h) | `src/grevir/base/meta/tuple_algorithms.hpp` | split |
| [ardOinus/src/setl_tuple_helpers.h](../../ardoinus/ardOinus/src/setl_tuple_helpers.h) | `src/grevir/base/meta/tuple_types.hpp` | split |
| [ardOinus/src/setl_tuple_helpers.h](../../ardoinus/ardOinus/src/setl_tuple_helpers.h) | `tests/tuple_static_tests.cpp` | split |
| [ardOinus/src/setl_utils.h](../../ardoinus/ardOinus/src/setl_utils.h) | `src/grevir/base/utility.hpp` | extract |
| [ardOinus/src/setlx_array.h](../../ardoinus/ardOinus/src/setlx_array.h) | `src/grevir/base/compat/array.hpp` | adapt |
| [ardOinus/src/setlx_cassert.h](../../ardoinus/ardOinus/src/setlx_cassert.h) | `src/grevir/base/compat/cassert.hpp` | adapt |
| [ardOinus/src/setlx_cstddef.h](../../ardoinus/ardOinus/src/setlx_cstddef.h) | `src/grevir/base/compat/cstddef.hpp` | adapt |
| [ardOinus/src/setlx_cstdint.h](../../ardoinus/ardOinus/src/setlx_cstdint.h) | `src/grevir/base/compat/cstdint.hpp` | adapt |
| [ardOinus/src/setlx_cstdlib.h](../../ardoinus/ardOinus/src/setlx_cstdlib.h) | `src/grevir/base/compat/cstdlib.hpp` | adapt |
| [ardOinus/src/setlx_limits.h](../../ardoinus/ardOinus/src/setlx_limits.h) | `src/grevir/base/compat/limits.hpp` | adapt |
| [ardOinus/src/setlx_tuple.h](../../ardoinus/ardOinus/src/setlx_tuple.h) | `src/grevir/base/compat/tuple.hpp` | adapt |
| [ardOinus/src/setlx_type_traits.h](../../ardoinus/ardOinus/src/setlx_type_traits.h) | `src/grevir/base/compat/type_traits.hpp` | adapt |
| [ardOinus/src/type_for_size.h](../../ardoinus/ardOinus/src/type_for_size.h) | `src/grevir/base/type_for_size.hpp` | extract |
| [ardOinus/tests/circular_buffer_test.h](../../ardoinus/ardOinus/tests/circular_buffer_test.h) | `tests/circular_buffer_test.cpp` | adapt |
| [ardOinus/tests/setl_cyclic_int_test.h](../../ardoinus/ardOinus/tests/setl_cyclic_int_test.h) | `tests/cyclic_int_test.cpp` | adapt |
| [ardOinus/tests/setl_templ_utils_test.h](../../ardoinus/ardOinus/tests/setl_templ_utils_test.h) | `tests/type_algorithms_test.cpp` | adapt |
| [ardOinus/tests/type_for_size_test.h](../../ardoinus/ardOinus/tests/type_for_size_test.h) | `tests/type_for_size_test.cpp` | adapt |

### grevir-time

| Existing source | Planned destination | Action |
| --- | --- | --- |
| [ardOinus/src/setl_interactive_scaling.h](../../ardoinus/ardOinus/src/setl_interactive_scaling.h) | `src/grevir/time/interactive_scaling.hpp` | extract |
| [ardOinus/src/setl_time.h](../../ardoinus/ardOinus/src/setl_time.h) | `src/grevir/time/time.hpp` | extract |
| [ardOinus/src/setl_time_unit.h](../../ardoinus/ardOinus/src/setl_time_unit.h) | `src/grevir/time/units.hpp` | extract |
| [ardOinus/tests/setl_time_test.h](../../ardoinus/ardOinus/tests/setl_time_test.h) | `tests/time_test.cpp` | adapt |
| [ardOinus/tests/setl_time_unit_test.h](../../ardoinus/ardOinus/tests/setl_time_unit_test.h) | `tests/units_test.cpp` | adapt |

### grevir-registers

| Existing source | Planned destination | Action |
| --- | --- | --- |
| [ardOinus/src/setl_bit_fields.h](../../ardoinus/ardOinus/src/setl_bit_fields.h) | `src/grevir/registers/access.hpp` | split |
| [ardOinus/src/setl_bit_fields.h](../../ardoinus/ardOinus/src/setl_bit_fields.h) | `src/grevir/registers/apply.hpp` | split |
| [ardOinus/src/setl_bit_fields.h](../../ardoinus/ardOinus/src/setl_bit_fields.h) | `src/grevir/registers/bit_mapping.hpp` | split |
| [ardOinus/src/setl_bit_fields.h](../../ardoinus/ardOinus/src/setl_bit_fields.h) | `src/grevir/registers/bit_values.hpp` | split |
| [ardOinus/src/setl_bit_fields.h](../../ardoinus/ardOinus/src/setl_bit_fields.h) | `src/grevir/registers/fields.hpp` | split |
| [ardOinus/src/setl_bit_fields.h](../../ardoinus/ardOinus/src/setl_bit_fields.h) | `src/grevir/registers/selection.hpp` | split |
| [ardOinus/tests/setl_bit_fields_test.cxx](../../ardoinus/ardOinus/tests/setl_bit_fields_test.cxx) | `tests/bit_fields_test.cpp` | adapt |

### grevir-core

| Existing source | Planned destination | Action |
| --- | --- | --- |
| [ardOinus/src/ardOinus.h](../../ardoinus/ardOinus/src/ardOinus.h) | `src/grevir/core/application.hpp` | split |
| [ardOinus/src/ardOinus.h](../../ardoinus/ardOinus/src/ardOinus.h) | `src/grevir/core/resource_checks.hpp` | split |
| [ardOinus/src/ardo_params_modules.h](../../ardoinus/ardOinus/src/ardo_params_modules.h) | `src/grevir/core/module.hpp` | extract |
| [ardOinus/src/ardo_resources.h](../../ardoinus/ardOinus/src/ardo_resources.h) | `src/grevir/core/resource_claims.hpp` | extract |
| [ardOinus/src/ardo_singleton.h](../../ardoinus/ardOinus/src/ardo_singleton.h) | `src/grevir/core/singleton.hpp` | extract |
| [ardOinus/src/ardo_sys_defs.h](../../ardoinus/ardOinus/src/ardo_sys_defs.h) | `src/grevir/core/board.hpp` | split |
| [ardOinus/src/ardo_timers.h](../../ardoinus/ardOinus/src/ardo_timers.h) | `src/grevir/core/allocation.hpp` | split |
| [ardOinus/src/setl_device_map.h](../../ardoinus/ardOinus/src/setl_device_map.h) | `src/grevir/core/device_map.hpp` | extract |
| [ardOinus/src/sys/mcu/avr/base/ardo_avr_base_register.h](../../ardoinus/ardOinus/src/sys/mcu/avr/base/ardo_avr_base_register.h) | `src/grevir/core/resource_graph.hpp` | split |
| [ardOinus/tests/ardOinus_test.h](../../ardoinus/ardOinus/tests/ardOinus_test.h) | `tests/resource_claims_test.cpp` | split |
| [ardOinus/tests/ardo_singleton_test.h](../../ardoinus/ardOinus/tests/ardo_singleton_test.h) | `tests/singleton_test.cpp` | adapt |
| [ardOinus/tests/dependent_module.cxx](../../ardoinus/ardOinus/tests/dependent_module.cxx) | `tests/dependent_module_test.cpp` | adapt |

### grevir-peripherals

| Existing source | Planned destination | Action |
| --- | --- | --- |
| [ardOinus/src/ardOinus.h](../../ardoinus/ardOinus/src/ardOinus.h) | `src/grevir/peripherals/gpio/debounce.hpp` | split |
| [ardOinus/src/ardOinus.h](../../ardoinus/ardOinus/src/ardOinus.h) | `src/grevir/peripherals/gpio/input.hpp` | split |
| [ardOinus/src/ardOinus.h](../../ardoinus/ardOinus/src/ardOinus.h) | `src/grevir/peripherals/gpio/interfaces.hpp` | split |
| [ardOinus/src/ardOinus.h](../../ardoinus/ardOinus/src/ardOinus.h) | `src/grevir/peripherals/gpio/output.hpp` | split |
| [ardOinus/src/ardo_button_events.h](../../ardoinus/ardOinus/src/ardo_button_events.h) | `src/grevir/peripherals/button_events.hpp` | adapt |
| [ardOinus/src/ardo_eeprom.h](../../ardoinus/ardOinus/src/ardo_eeprom.h) | `src/grevir/peripherals/storage_region.hpp` | split |
| [ardOinus/src/ardo_pwm_output.h](../../ardoinus/ardOinus/src/ardo_pwm_output.h) | `src/grevir/peripherals/pwm_output.hpp` | adapt |
| [ardOinus/src/ardo_sequencer.h](../../ardoinus/ardOinus/src/ardo_sequencer.h) | Superseded by `src/grevir/peripherals/time_poller.hpp`; no `sequencer.hpp` created | superseded |
| [ardOinus/src/ardo_time_poller.h](../../ardoinus/ardOinus/src/ardo_time_poller.h) | `src/grevir/peripherals/time_poller.hpp` | adapt |
| [ardOinus/src/ardo_timers.h](../../ardoinus/ardOinus/src/ardo_timers.h) | `src/grevir/peripherals/timer/requirements.hpp` | split |
| [ardOinus/src/ardo_timers.h](../../ardoinus/ardOinus/src/ardo_timers.h) | `src/grevir/peripherals/timer/selection.hpp` | split |
| [ardOinus/src/ardo_timers.h](../../ardoinus/ardOinus/src/ardo_timers.h) | `tests/timer_config_static_tests.cpp` | split |
| [ardOinus/tests/ardOinus_test.h](../../ardoinus/ardOinus/tests/ardOinus_test.h) | `tests/gpio_application_test.cpp` | split |

### grevir-avr

| Existing source | Planned destination | Action |
| --- | --- | --- |
| [ardOinus/devel/code/servo_test.cxx](../../ardoinus/ardOinus/devel/code/servo_test.cxx) | `tests/servo_test.cpp` | adapt |
| [ardOinus/src/ardo_avr_bit_io_defs.h](../../ardoinus/ardOinus/src/ardo_avr_bit_io_defs.h) | `src/grevir/avr/register_definitions.hpp` | adapt |
| [ardOinus/src/ardo_mcu_avr.h](../../ardoinus/ardOinus/src/ardo_mcu_avr.h) | `src/grevir/avr/mcu.hpp` | extract |
| [ardOinus/src/ardo_timers.h](../../ardoinus/ardOinus/src/ardo_timers.h) | `src/grevir/avr/timer/options.hpp` | split |
| [ardOinus/src/setl_bit_io_defs.h](../../ardoinus/ardOinus/src/setl_bit_io_defs.h) | `src/grevir/avr/legacy_register_definitions.hpp` | adapt |
| [ardOinus/src/setl_system.h](../../ardoinus/ardOinus/src/setl_system.h) | `src/grevir/avr/memory_policy.hpp` | split |
| [ardOinus/src/sys/mcu/avr/ardo_supplemental_atmega328p_dev.h](../../ardoinus/ardOinus/src/sys/mcu/avr/ardo_supplemental_atmega328p_dev.h) | `extras/legacy/ardOinus/src/sys/mcu/avr/ardo_supplemental_atmega328p_dev.h` | preserve |
| [ardOinus/src/sys/mcu/avr/ardo_supplemental_atmega328p_dev.h](../../ardoinus/ardOinus/src/sys/mcu/avr/ardo_supplemental_atmega328p_dev.h) | `src/grevir/avr/devices/atmega328p/gpio_fields.hpp` | split |
| [ardOinus/src/sys/mcu/avr/ardo_supplemental_atmega328p_dev.h](../../ardoinus/ardOinus/src/sys/mcu/avr/ardo_supplemental_atmega328p_dev.h) | `src/grevir/avr/devices/atmega328p/peripheral_fields.hpp` | split |
| [ardOinus/src/sys/mcu/avr/ardo_supplemental_atmega328p_dev.h](../../ardoinus/ardOinus/src/sys/mcu/avr/ardo_supplemental_atmega328p_dev.h) | `src/grevir/avr/devices/atmega328p/resources.hpp` | split |
| [ardOinus/src/sys/mcu/avr/ardo_supplemental_atmega328p_dev.h](../../ardoinus/ardOinus/src/sys/mcu/avr/ardo_supplemental_atmega328p_dev.h) | `src/grevir/avr/devices/atmega328p/timer_fields.hpp` | split |
| [ardOinus/src/sys/mcu/avr/ardo_supplemental_atmega328p_dev.h](../../ardoinus/ardOinus/src/sys/mcu/avr/ardo_supplemental_atmega328p_dev.h) | `src/grevir/avr/devices/atmega328p/timers.hpp` | split |
| [ardOinus/src/sys/mcu/avr/ardo_supplemental_atmega328p_dev.h](../../ardoinus/ardOinus/src/sys/mcu/avr/ardo_supplemental_atmega328p_dev.h) | `src/grevir/avr/gpio.hpp` | split |
| [ardOinus/src/sys/mcu/avr/ardo_supplemental_atmega328p_dev.h](../../ardoinus/ardOinus/src/sys/mcu/avr/ardo_supplemental_atmega328p_dev.h) | `src/grevir/avr/timer/clock.hpp` | split |
| [ardOinus/src/sys/mcu/avr/ardo_supplemental_atmega328p_dev.h](../../ardoinus/ardOinus/src/sys/mcu/avr/ardo_supplemental_atmega328p_dev.h) | `src/grevir/avr/timer/configuration.hpp` | split |
| [ardOinus/src/sys/mcu/avr/ardo_supplemental_atmega328p_dev.h](../../ardoinus/ardOinus/src/sys/mcu/avr/ardo_supplemental_atmega328p_dev.h) | `src/grevir/avr/timer/definition.hpp` | split |
| [ardOinus/src/sys/mcu/avr/ardo_supplemental_atmega328p_dev.h](../../ardoinus/ardOinus/src/sys/mcu/avr/ardo_supplemental_atmega328p_dev.h) | `src/grevir/avr/timer/mode.hpp` | split |
| [ardOinus/src/sys/mcu/avr/ardo_supplemental_atmega328p_dev.h](../../ardoinus/ardOinus/src/sys/mcu/avr/ardo_supplemental_atmega328p_dev.h) | `src/grevir/avr/timer/output.hpp` | split |
| [ardOinus/src/sys/mcu/avr/ardo_sys_avr_mcu_defs.h](../../ardoinus/ardOinus/src/sys/mcu/avr/ardo_sys_avr_mcu_defs.h) | `extras/legacy/ardOinus/src/sys/mcu/avr/ardo_sys_avr_mcu_defs.h` | preserve |
| [ardOinus/src/sys/mcu/avr/ardo_sys_avr_mcu_defs.h](../../ardoinus/ardOinus/src/sys/mcu/avr/ardo_sys_avr_mcu_defs.h) | `src/grevir/avr/selected_mcu.hpp` | split |
| [ardOinus/src/sys/mcu/avr/base/ardo_avr_base.h](../../ardoinus/ardOinus/src/sys/mcu/avr/base/ardo_avr_base.h) | `src/grevir/avr/base.hpp` | extract |
| [ardOinus/src/sys/mcu/avr/base/ardo_avr_base_gpio.h](../../ardoinus/ardOinus/src/sys/mcu/avr/base/ardo_avr_base_gpio.h) | `src/grevir/avr/gpio.hpp` | reconcile |
| [ardOinus/src/sys/mcu/avr/base/ardo_avr_base_register.h](../../ardoinus/ardOinus/src/sys/mcu/avr/base/ardo_avr_base_register.h) | `src/grevir/avr/register.hpp` | split |
| [ardOinus/src/sys/mcu/avr/base/ardo_avr_base_timer.h](../../ardoinus/ardOinus/src/sys/mcu/avr/base/ardo_avr_base_timer.h) | `extras/legacy/ardOinus/src/sys/mcu/avr/base/ardo_avr_base_timer.h` | preserve |
| [ardOinus/src/sys/mcu/avr/base/ardo_avr_base_timer.h](../../ardoinus/ardOinus/src/sys/mcu/avr/base/ardo_avr_base_timer.h) | `src/grevir/avr/devices/atmega328p/timer_fields.hpp` | split |
| [ardOinus/src/sys/mcu/avr/base/ardo_avr_base_timer.h](../../ardoinus/ardOinus/src/sys/mcu/avr/base/ardo_avr_base_timer.h) | `src/grevir/avr/timer/clock.hpp` | split |
| [ardOinus/src/sys/mcu/avr/base/ardo_avr_base_timer.h](../../ardoinus/ardOinus/src/sys/mcu/avr/base/ardo_avr_base_timer.h) | `src/grevir/avr/timer/configuration.hpp` | split |
| [ardOinus/src/sys/mcu/avr/base/ardo_avr_base_timer.h](../../ardoinus/ardOinus/src/sys/mcu/avr/base/ardo_avr_base_timer.h) | `src/grevir/avr/timer/definition.hpp` | split |
| [ardOinus/src/sys/mcu/avr/base/ardo_avr_base_timer.h](../../ardoinus/ardOinus/src/sys/mcu/avr/base/ardo_avr_base_timer.h) | `src/grevir/avr/timer/mode.hpp` | split |
| [ardOinus/src/sys/mcu/avr/base/ardo_avr_base_timer.h](../../ardoinus/ardOinus/src/sys/mcu/avr/base/ardo_avr_base_timer.h) | `src/grevir/avr/timer/output.hpp` | split |
| [ardOinus/tests/ardo_avr_timers_test.h](../../ardoinus/ardOinus/tests/ardo_avr_timers_test.h) | `tests/timer_test.cpp` | adapt |

### grevir-esp32

| Existing source | Planned destination | Action |
| --- | --- | --- |
| [ardOinus/src/setl_system.h](../../ardoinus/ardOinus/src/setl_system.h) | `src/grevir/esp32/memory_policy.hpp` | split |

### grevir-arduino

| Existing source | Planned destination | Action |
| --- | --- | --- |
| [ardOinus/devel/code/blink_test.cxx](../../ardoinus/ardOinus/devel/code/blink_test.cxx) | `tests/blink_test.cpp` | adapt |
| [ardOinus/examples/Basics/ArdoBareMinimum/ArdoBareMinimum.ino](../../ardoinus/ardOinus/examples/Basics/ArdoBareMinimum/ArdoBareMinimum.ino) | `examples/BareMinimum/BareMinimum.ino` | adapt |
| [ardOinus/examples/Basics/ArdoBetterBlink/ArdoBetterBlink.ino](../../ardoinus/ardOinus/examples/Basics/ArdoBetterBlink/ArdoBetterBlink.ino) | `examples/BetterBlink/BetterBlink.ino` | adapt |
| [ardOinus/examples/Basics/ArdoBlink/ArdoBlink.ino](../../ardoinus/ardOinus/examples/Basics/ArdoBlink/ArdoBlink.ino) | `examples/Blink/Blink.ino` | adapt |
| [ardOinus/examples/Basics/ArdoDependentModules/ArdoDependentModules.ino](../../ardoinus/ardOinus/examples/Basics/ArdoDependentModules/ArdoDependentModules.ino) | `examples/DependentModules/DependentModules.ino` | adapt |
| [ardOinus/examples/Basics/ArdoDigitalInputSerial/ArdoDigitalInputSerial.ino](../../ardoinus/ardOinus/examples/Basics/ArdoDigitalInputSerial/ArdoDigitalInputSerial.ino) | `examples/DigitalInputSerial/DigitalInputSerial.ino` | adapt |
| [ardOinus/examples/Basics/ArdoSerial/ArdoSerial.ino](../../ardoinus/ardOinus/examples/Basics/ArdoSerial/ArdoSerial.ino) | `examples/Serial/Serial.ino` | adapt |
| [ardOinus/examples/Control/ArdOArrays/ArdOArrays.ino](../../ardoinus/ardOinus/examples/Control/ArdOArrays/ArdOArrays.ino) | `examples/Arrays/Arrays.ino` | adapt |
| [ardOinus/src/ardo_eeprom.h](../../ardoinus/ardOinus/src/ardo_eeprom.h) | `src/grevir/arduino/eeprom.hpp` | split |
| [ardOinus/src/ardo_sys_defs.h](../../ardoinus/ardOinus/src/ardo_sys_defs.h) | `src/grevir/arduino/core.hpp` | split |
| [ardOinus/src/ardo_sys_defs.h](../../ardoinus/ardOinus/src/ardo_sys_defs.h) | `src/grevir/arduino/pwm.hpp` | split |
| [ardOinus/src/ardo_sys_defs.h](../../ardoinus/ardOinus/src/ardo_sys_defs.h) | `src/grevir/arduino/serial.hpp` | split |

### grevir-arduino-avr

| Existing source | Planned destination | Action |
| --- | --- | --- |
| [ardOinus/examples/RawAvr/FixedFreqencyCounter/FixedFrequencyCounter.ino](../../ardoinus/ardOinus/examples/RawAvr/FixedFreqencyCounter/FixedFrequencyCounter.ino) | `examples/FixedFrequencyCounter/FixedFrequencyCounter.ino` | adapt |
| [ardOinus/src/ardo_sys_defs.h](../../ardoinus/ardOinus/src/ardo_sys_defs.h) | `src/grevir/arduino_avr/selected_board.hpp` | split |
| [ardOinus/src/ardo_timers.h](../../ardoinus/ardOinus/src/ardo_timers.h) | `src/grevir/arduino_avr/timer_policy.hpp` | split |
| [ardOinus/src/sys/ardo_sys_atmega328.h](../../ardoinus/ardOinus/src/sys/ardo_sys_atmega328.h) | `src/grevir/arduino_avr/pin_bindings.hpp` | split |
| [ardOinus/src/sys/boards/arduino/ardo_board_nano_atmega328.h](../../ardoinus/ardOinus/src/sys/boards/arduino/ardo_board_nano_atmega328.h) | `src/grevir/arduino_avr/boards/nano.hpp` | split |
| [ardOinus/src/sys/boards/arduino/ardo_board_nano_atmega328old.h](../../ardoinus/ardOinus/src/sys/boards/arduino/ardo_board_nano_atmega328old.h) | `src/grevir/arduino_avr/boards/nano_old_bootloader.hpp` | split |
| [ardOinus/src/sys/boards/arduino/ardo_board_uno_atmega328p.h](../../ardoinus/ardOinus/src/sys/boards/arduino/ardo_board_uno_atmega328p.h) | `src/grevir/arduino_avr/boards/uno.hpp` | split |
| [ardOinus/src/sys/boards/arduino/arduino_boards.h](../../ardoinus/ardOinus/src/sys/boards/arduino/arduino_boards.h) | `extras/legacy/ardOinus/src/sys/boards/arduino/arduino_boards.h` | preserve |
| [ardOinus/src/sys/boards/arduino/arduino_boards.h](../../ardoinus/ardOinus/src/sys/boards/arduino/arduino_boards.h) | `src/grevir/arduino_avr/selected_board.hpp` | split |
| [ardOinus/src/sys/mcu/avr/ardo_supplemental_atmega328p_dev.h](../../ardoinus/ardOinus/src/sys/mcu/avr/ardo_supplemental_atmega328p_dev.h) | `src/grevir/arduino_avr/boards/nano.hpp` | split |
| [ardOinus/src/sys/mcu/avr/ardo_supplemental_atmega328p_dev.h](../../ardoinus/ardOinus/src/sys/mcu/avr/ardo_supplemental_atmega328p_dev.h) | `src/grevir/arduino_avr/boards/uno.hpp` | split |
| [ardOinus/src/sys/mcu/avr/ardo_supplemental_atmega328p_dev.h](../../ardoinus/ardOinus/src/sys/mcu/avr/ardo_supplemental_atmega328p_dev.h) | `src/grevir/arduino_avr/timer_policy.hpp` | split |

### grevir-arduino-esp32

| Existing source | Planned destination | Action |
| --- | --- | --- |
| [ardOinus/src/ardo_sys_defs.h](../../ardoinus/ardOinus/src/ardo_sys_defs.h) | `src/grevir/arduino_esp32/selected_board.hpp` | split |
| [ardOinus/src/sys/ardo_sys_esp32.h](../../ardoinus/ardOinus/src/sys/ardo_sys_esp32.h) | `extras/legacy/ardOinus/src/sys/ardo_sys_esp32.h` | preserve |
| [ardOinus/src/sys/ardo_sys_esp32.h](../../ardoinus/ardOinus/src/sys/ardo_sys_esp32.h) | `src/grevir/arduino_esp32/pin_bindings.hpp` | split |

### grevir-encoder

| Existing source | Planned destination | Action |
| --- | --- | --- |
| [ardOQuadEncoder/devel/code/ardOQuadEncoderTest.cxx](../../ardoinus/ardOQuadEncoder/devel/code/ardOQuadEncoderTest.cxx) | `tests/encoder_test.cpp` | adapt |
| [ardOQuadEncoder/src/ArdoQuadEncoder.h](../../ardoinus/ardOQuadEncoder/src/ArdoQuadEncoder.h) | `src/grevir/encoder/encoder.hpp` | adapt |

### grevir-stepper

| Existing source | Planned destination | Action |
| --- | --- | --- |
| [ardOStepper/examples/ardOStepperEncoder/ardOStepperEncoder.ino](../../ardoinus/ardOStepper/examples/ardOStepperEncoder/ardOStepperEncoder.ino) | `examples/StepperEncoder/StepperEncoder.ino` | adapt |
| [ardOStepper/src/ardOStepper.h](../../ardoinus/ardOStepper/src/ardOStepper.h) | `src/grevir/stepper/stepper.hpp` | adapt |
| [ardOStepper/src/devel/code/ardOStepperTest.cxx](../../ardoinus/ardOStepper/src/devel/code/ardOStepperTest.cxx) | `tests/stepper_test.cpp` | adapt |

### grevir-fastled

| Existing source | Planned destination | Action |
| --- | --- | --- |
| [ardOFastLED/examples/FastLedQuadEncoder/FastLedQuadEncoder.ino](../../ardoinus/ardOFastLED/examples/FastLedQuadEncoder/FastLedQuadEncoder.ino) | `examples/FastLedQuadEncoder/FastLedQuadEncoder.ino` | adapt |
| [ardOFastLED/src/ArdoFastLED.h](../../ardoinus/ardOFastLED/src/ArdoFastLED.h) | `src/grevir/fastled/strip.hpp` | adapt |
| [ardOinus/examples/ParkLightsV2/ParkLightsV2.ino](../../ardoinus/ardOinus/examples/ParkLightsV2/ParkLightsV2.ino) | `extras/legacy/examples/ParkLightsV2/ParkLightsV2.ino` | preserve |
| [ardOinus/examples/ParkLightsV2/default_image.h](../../ardoinus/ardOinus/examples/ParkLightsV2/default_image.h) | `extras/legacy/examples/ParkLightsV2/default_image.h` | preserve |
| [ardOinus/examples/ParkLightsV2/parklights.h](../../ardoinus/ardOinus/examples/ParkLightsV2/parklights.h) | `extras/legacy/examples/ParkLightsV2/parklights.h` | preserve |

### grevir-pulse-codec

| Existing source | Planned destination | Action |
| --- | --- | --- |
| [ardOinus/src/pwe_serial.h](../../ardoinus/ardOinus/src/pwe_serial.h) | `src/grevir/pulse_codec/bits.hpp` | split |
| [ardOinus/src/pwe_serial.h](../../ardoinus/ardOinus/src/pwe_serial.h) | `src/grevir/pulse_codec/decoder.hpp` | split |
| [ardOinus/src/pwe_serial.h](../../ardoinus/ardOinus/src/pwe_serial.h) | `src/grevir/pulse_codec/encoder.hpp` | split |
| [ardOinus/src/pwe_serial.h](../../ardoinus/ardOinus/src/pwe_serial.h) | `src/grevir/pulse_codec/waveform.hpp` | split |
| [ardOinus/tests/pwe_serial_test.h](../../ardoinus/ardOinus/tests/pwe_serial_test.h) | `tests/codec_test.cpp` | adapt |

### grevir-pulse-io

| Existing source | Planned destination | Action |
| --- | --- | --- |
| [ardOinus/devel/code/gammil_repeater.cxx](../../ardoinus/ardOinus/devel/code/gammil_repeater.cxx) | `extras/legacy/ardOinus/devel/code/gammil_repeater.cxx` | preserve |
| [ardOinus/devel/code/pwe_test_module.cxx](../../ardoinus/ardOinus/devel/code/pwe_test_module.cxx) | `tests/module_test.cpp` | adapt |
| [ardOinus/src/pwm_serial_comms.h](../../ardoinus/ardOinus/src/pwm_serial_comms.h) | `src/grevir/pulse_io/modules.hpp` | extract |

### grevir-packet

| Existing source | Planned destination | Action |
| --- | --- | --- |
| [ardOnet/src/ardo_packet_reassembler.h](../../ardoinus/ardOnet/src/ardo_packet_reassembler.h) | `src/grevir/packet/header.hpp` | split |
| [ardOnet/src/ardo_packet_reassembler.h](../../ardoinus/ardOnet/src/ardo_packet_reassembler.h) | `src/grevir/packet/manager.hpp` | split |
| [ardOnet/src/ardo_packet_reassembler.h](../../ardoinus/ardOnet/src/ardo_packet_reassembler.h) | `src/grevir/packet/reassembler.hpp` | split |
| [ardOnet/src/ardo_packet_reassembler.h](../../ardoinus/ardOnet/src/ardo_packet_reassembler.h) | `src/grevir/packet/sender.hpp` | split |
| [ardOnet/tests/PaketReassemblerTest.cxx](../../ardoinus/ardOnet/tests/PaketReassemblerTest.cxx) | `tests/packet_reassembler_test.cpp` | adapt |

### grevir-test-support

| Existing source | Planned destination | Action |
| --- | --- | --- |
| [ardOinus/devel/code/ardOinus_test.cxx](../../ardoinus/ardOinus/devel/code/ardOinus_test.cxx) | `extras/legacy/ardOinus/devel/code/ardOinus_test.cxx` | preserve |
| [ardOinus/devel/code/emulate_arduino.cxx](../../ardoinus/ardOinus/devel/code/emulate_arduino.cxx) | `src/arduino_globals.cpp` | extract |
| [ardOinus/src/ardo_sys_defs.h](../../ardoinus/ardOinus/src/ardo_sys_defs.h) | `include/grevir/test/select_mock.hpp` | split |
| [ardOinus/src/assert_that.h](../../ardoinus/ardOinus/src/assert_that.h) | `include/grevir/test/assert_that.hpp` | extract |
| [ardOinus/src/mock_arduino.h](../../ardoinus/ardOinus/src/mock_arduino.h) | `include/grevir/test/arduino.hpp` | extract |
| [ardOinus/src/setl_bit_fields.h](../../ardoinus/ardOinus/src/setl_bit_fields.h) | `include/grevir/test/register_memory.hpp` | split |
| [ardOinus/src/setl_test_framework.h](../../ardoinus/ardOinus/src/setl_test_framework.h) | `include/grevir/test/framework.hpp` | extract |
| [ardOinus/src/sys/ardo_sys_mock.h](../../ardoinus/ardOinus/src/sys/ardo_sys_mock.h) | `include/grevir/test/mock_core.hpp` | extract |
| [ardOinus/src/sys/mcu/avr/base/ardo_avr_base_register.h](../../ardoinus/ardOinus/src/sys/mcu/avr/base/ardo_avr_base_register.h) | `include/grevir/test/avr_registers.hpp` | split |
| [ardOinus/tests/adro_avr_mock_registers.cxx](../../ardoinus/ardOinus/tests/adro_avr_mock_registers.cxx) | `src/avr_registers.cpp` | extract |

### Bulk mapping rules (fully expanded in SQLite)

| Existing path relative to Ardoinus | Destination repo | Exact path transformation | Count |
| --- | --- | --- | ---: |
| `ardOinus/src/sys/mcu/avr/ardo_supplemental_<mcu>.h` excluding `_dev.h` | `grevir-avr` | `src/grevir/avr/generated/<mcu>.hpp` | 302 |
| `ardOinus/src/sys/mcu/avr/ardo_supplemental_<mcu>_dev.h` excluding ATmega328P | `grevir-avr` | `extras/legacy/mcu/ardo_supplemental_<mcu>_dev.h` | 301 |
| `ardOinus/src/sys/boards/arduino/ardo_board_<board>.h` excluding the three initial board entries above | `grevir-arduino-avr` | `extras/legacy/boards/ardo_board_<board>.h` | 35 |

The `<mcu>` and `<board>` strings are preserved exactly from the source filename;
no case normalization or heuristic chip-name substitution is part of these rules.
