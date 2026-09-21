# Grevir workspace

Grevir evolves Ardoinus into independently usable embedded C++ libraries.
GWZ manages the member checkouts; CMake compiles the libraries currently extracted.

The extracted members are [grevir-base](grevir-base/README.md),
[grevir-time](grevir-time/README.md), [grevir-core](grevir-core/README.md), and the first
[grevir-peripherals](grevir-peripherals/README.md) and
[grevir-registers](grevir-registers/README.md) and [grevir-avr](grevir-avr/README.md) increments.
[grevir-test-support](grevir-test-support/README.md) supplies shared host-test setup and register-memory fixtures.
Their root `library.properties` and `src/`
directories follow Arduino library layout. Target compiler and Arduino sketch
validation are still pending. Existing `setl` and `ardo` names remain; extracted pin/poller templates now take
explicit GPIO backend and clock bindings.

## Native compile check

From this workspace, using the installed Xcode compiler:

```sh
cmake -S . -B build/native -G "Unix Makefiles" \
  -DCMAKE_CXX_COMPILER="$(xcrun --find clang++)"
cmake --build build/native
```

This compiles the extracted production source, every public header independently,
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

The opt-in Catch2/CTest suite has 123 passing cases: Base 5, Time 4, Core 10,
Peripherals 29, Registers 23, Test Support 3 and AVR 49. All seven retained Base/Time test files now compile; the
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
execute production code.
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

A separate [timer allocation prototype](experiments/timer-allocation/README.md)
now exercises deterministic allocation and common/resident-target configuration
using synthetic inventories and an experimental end-to-end ATmega328P Timer0/1/2
fast-PWM adapter with real register bindings. Its four checks are separate from
the installed libraries and the 123-case production suite. Other timer features
and a real ESP32 backend remain TBD.

See the [repository plan](dev-docs/GrevirRepositoryPlan.md) and
[extraction progress](dev-docs/GrevirExtractionProgress.md). Source ownership and
per-file extraction status are queryable in `dev-docs/GrevirFileMap.sqlite`.
