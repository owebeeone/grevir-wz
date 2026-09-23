# Grevir workspace

Grevir evolves Ardoinus into independently usable embedded C++ libraries.
GWZ manages the member checkouts; CMake compiles the libraries currently extracted.

The extracted members are [grevir-base](grevir-base/README.md),
[grevir-time](grevir-time/README.md), [grevir-core](grevir-core/README.md), and the first
[grevir-peripherals](grevir-peripherals/README.md) and
[grevir-registers](grevir-registers/README.md) and [grevir-avr](grevir-avr/README.md) increments.
[grevir-pulse-codec](grevir-pulse-codec/README.md) supplies portable pulse-width serial encoding and decoding.
[grevir-pulse-io](grevir-pulse-io/README.md) connects the pulse codec to injected GPIO pins and a clock.
[grevir-packet](grevir-packet/README.md) supplies transport-independent fragmentation and reassembly.
[grevir-encoder](grevir-encoder/README.md) supplies a quadrature decoder and Core module wrapper with injected pins and clock.
[grevir-stepper](grevir-stepper/README.md) supplies stepper phase tables, movement state and a Core module wrapper with injected pins and clock.
[grevir-arduino](grevir-arduino/README.md) and [grevir-arduino-avr](grevir-arduino-avr/README.md) supply Arduino services and Uno/Nano policies.
[grevir-fastled](grevir-fastled/README.md) supplies the LED-strip integration.
[grevir-test-support](grevir-test-support/README.md) supplies shared host-test setup and register-memory fixtures.
Their root `library.properties` and `src/`
directories follow Arduino library layout. Selected AVR compiler, simavr and Arduino sketch validation is recorded from
weftpi (`gianni@10.1.1.236`, `/home/gianni/git/grevir-wz`); silicon validation remains
on hold. See the [current audit](dev-docs/GrevirMigrationAudit.md) for scope and evidence. Existing `setl` and `ardo` names remain; extracted pin/poller templates now take
explicit GPIO backend and clock bindings.

## Native compile check

From this workspace, using the installed Xcode compiler:

```sh
cmake -S . -B build/native -G "Unix Makefiles" \
  -DCMAKE_CXX_COMPILER="$(xcrun --find clang++)"
cmake --build build/native
```

**Current check (23 September):** the full all-target native build passes. The
standalone timer-clock probe now uses its target's dependency include directories.

The intended compile-check build compiles the extracted production source, every public header independently,
tuple static assertions and representative template users in C++23 mode with the
standard library enabled. It does not run the historical tests. No Arduino
installation, target compiler, downloaded dependency or simulator is needed.
The Makefiles generator uses the tools already installed here; Ninja can also
execute this CMake build when available.

Core also runs compiler-only checks for six valid applications and nineteen expected
failures (seventeen resource-conflict cases and two dependency cycles), plus
parameter-index selection and bounds checks. Its
installed-package consumer builds and executes independently. Known inherited
Core defects are recorded in its README. Hardware validation is on hold.

## Host mock validation

The opt-in Catch2/CTest suite has 186 passing cases: Base 5, Time 4, Core 10,
Peripherals 29, Registers 24, Test Support 3, AVR 50, Pulse Codec 11, Packet 12,
Pulse IO 2, Encoder 10, Stepper 12, Arduino 6, Arduino AVR 4 and FastLED 4. All seven retained Base/Time test files now compile; the
type-algorithm file supplies static assertions rather than a runtime case. Core
covers application lifecycle/state. Peripheral mocks record input/output and
open-drain operations and control time for expiry, catch-up, wraparound, sequences,
a blinking application, debounce and single/double/long button clicks. Time also
checks the corrected period-division operator. PWM tests cover scaling and pin
lifecycle; storage tests cover byte reads/writes and region offsets. Compile probes
cover combined claims, storage bounds and explicit-backend timer requirements.
Register fixtures cover sparse mappings, typed reads/writes, selection, grouped
operations and explicit barrier scopes; compiler probes check field collisions,
bounds and missing-field diagnostics. AVR mocks check address offsets, GPIO
configuration order, preserved bits and directional/open-drain wrappers; the
dynamic output configuration now agrees with its typed counterpart. Initial timer
clock checks cover divider selection, count/frequency arithmetic and applying
computed settings to mock registers. Waveform checks cover metadata selection,
unsupported combinations, reserved encodings and split register fields using
device tables retained as fixtures. Base also checks the corrected unsigned random
fallback in an isolated executable. Timer-definition checks cover capture control
masks, native-width TOP reads and absent/invalid source handling. Configuration
checks cover programmable/built-in TOP setup, narrower fields, live frequency reads
and rejected dynamic requests without register IO. Timer-output checks cover
endpoint polarity, integer duty rescaling, channel isolation and composed frequency
updates. Concrete ATmega328P checks cover Timer0/1/2 pin routes, actual addresses,
Timer1 byte ordering and interrupt-flag clearing through a byte-access model. These
execute production code. Pulse Codec covers the legacy exhaustive message sweep,
array payloads, both signal senses/bit polarities, clock wraparound, queued sends
and malformed/unread-frame recovery. Its isolated installed consumer and sanitizer
checks also pass. Packet tests cover reordering, duplicate and malformed fragments,
peer identity, bounded pool reuse, bitmap/length boundaries and exact wire bytes.
Its installed consumer needs no other Grevir package.
Encoder covers startup latch, clockwise and counter-clockwise Gray-code steps,
indeterminate doubles, identity and injected-clock scaling, the historical module
harness, and two independent instances. Its installed consumer uses only injected
GPIO and Core composition.
Stepper covers pin-mask order, strict step-period expiry, four-phase wrap, reverse,
coil hold versus timed off, float time scale, target/remaining arithmetic, custom
sequences, the historical module loop, two independent instances and an optional
encoder-follower case. Its installed consumer uses only injected GPIO and Core
composition; Encoder is not a production dependency.
Interrupt execution, waveform timing, asynchronous timer operation and electrical
behavior remain outside the current mock coverage.

Explicit first-time setup can download the pinned Catch2 3.8.1 source archive:

```sh
cmake -S . -B build/host-mock -G "Unix Makefiles" \
  -DGREVIR_BUILD_HOST_TESTS=ON -DGREVIR_FETCH_TEST_DEPENDENCIES=ON \
  -DCMAKE_BUILD_TYPE=Debug
cmake --build build/host-mock
ctest --test-dir build/host-mock --output-on-failure
```

Tests do not download dependencies. For offline setup, use an installed Catch2
3.8.1 package or `-DGREVIR_CATCH2_SOURCE_DIR=/path/to/Catch2-3.8.1` with
`-DGREVIR_FETCH_TEST_DEPENDENCIES=OFF`. Normal library builds do not require Catch2.

The earlier [legacy compile baseline](native-compile-check/README.md) remains
available separately. It deliberately uses the original Ardoinus checkout; the
new foundation build does not.

The [portable PWM MVP](dev-docs/GrevirPwmIntegration.md) is installed across Core,
Peripherals and AVR. Applications collect module requests, account for existing
resource claims and initialize selected ATmega328P Timer0/1/2 drivers automatically.
The former prototype forwards to these libraries; its four oracle checks remain
separate from the 186-case production suite. Uno/Nano Arduino integration exists. A real ESP32 backend
and additional timer features remain future work.

See the [repository plan](dev-docs/GrevirRepositoryPlan.md) and
[extraction progress](dev-docs/GrevirExtractionProgress.md). Source ownership and
per-file extraction status are queryable in `dev-docs/GrevirFileMap.sqlite`.
