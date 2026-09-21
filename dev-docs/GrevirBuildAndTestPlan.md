# Grevir build and test framework

Selected 18 September 2026; updated 21 September for the foundation test migration
and portable GPIO/timing, buttons, PWM, storage, timer requirements and register access. **Hardware validation is on hold.**

Implemented: opt-in CMake/CTest host tests with pinned Catch2 3.8.1 and shared
setup in Grevir Test Support. 123 cases pass: Base 5, Time 4, Core 10,
Peripherals 29, Registers 23, Test Support 3, AVR 49. All seven retained Base/Time files build (including static-only
type algorithms); their old test framework dependency is removed. GPIO fixtures
record logical pin operations, and a controlled clock exercises production pollers,
a blinking application and debounced button events. Period-division and inherited
debounce/setup defects have reproduced failures and now pass. The fixtures do not
model electrical or MCU behavior. Five PWM cases validate scaling and pin lifecycle;
two reproduced an input-narrowing bug, now fixed. Five storage cases validate
byte representation, offsets, updates and region isolation. Twenty-three register cases
validate sparse mappings, typed reads/writes, selection, grouped operations and
explicit barrier scopes against a byte array.
They reproduce and fix unnecessary full-width reads and writes outside a mask;
Five cases now replace printed output in the legacy portable register exercises
with assertions. Shared register storage adds three cases and one valid/three
invalid bounds probes. Nine AVR cases cover explicit offsets, GPIO ordering and
directional wrappers; dynamic configuration was corrected to match typed output
ordering. Seven additional AVR timer-clock cases cover prescaler lookup/rounding,
count/frequency examples, invalid requests, an independent capacity model, explicit
traits and computed mock-register writes. Eleven legacy static assertions and one
valid/seven rejected clock-map probes pass. MCU side effects, electrical behavior
and interrupts remain unmodeled. AVR compiler validation is on hold alongside
hardware validation.

Arithmetic corrections add three AVR boundary cases and a standalone Base
fallback-random case. Integer timer paths use integer arithmetic; native optimized
IR confirms no floating or 64-bit arithmetic for the inspected dynamic 32-bit
calls. Explicit floating calls keep their selected precision and checked result
conversions. Native UBSan checks pass for the unsigned fallback recurrence and all
256 byte-to-32-bit scaler inputs. These do not establish AVR instruction cost or
validate the full no-standard-library configuration.

Waveform-mode extraction adds four runtime cases and 28 compile-time assertions
(six retained from Ardoinus). They exercise selection, unsupported/empty queries,
all 21 legacy fixture modes, reserved encodings and split mock register writes.
The two source groups match after whitespace normalization. ATmega328P tables
remain fixtures; production algorithms accept caller-provided metadata. Isolated
AVR packages and a consumer with Catch2/Test Support disabled pass. This does not
validate waveform timing, generated device facts or a complete timer configuration.

Timer-definition extraction adds five runtime cases and thirteen static assertions,
including native-width TOP read traces, optional validity, capture-control masks,
empty/restricted source lists and absent-feature no-ops. Both legacy source groups
match after whitespace normalization. Independent headers, isolated packages and
the installed consumer pass with explicit mode traits and register access policies.
Capture interrupts, physical filtering and target register atomicity are unvalidated.

Timer-configuration extraction adds six runtime cases and twelve static assertions
for programmable and built-in TOP setup, divider/capacity selection, preserved bits,
native register widths, invalid dynamic requests without IO and live frequency
reads. One valid and seven rejected standalone probes check mandatory requests.
Isolated packages and the installed consumer pass; nine AVR headers compile alone.
Optimized host IR for dynamic integer configuration has no floating or 64-bit
arithmetic. Hardware PWM count conventions and live-update atomicity remain outside
this native evidence. AVR compiler and hardware validation remain on hold.

Timer-output extraction adds eight runtime cases and ten static assertions for
endpoint polarity, COM/OCR/GPIO order, preserved channels, fractional bounds,
integer rescaling, rejected updates and timer facade reads. One valid/seven rejected
standalone probes cover channel/GPIO conflicts, TOP sharing, unconfigured writes
and count capacity. Ten public AVR headers and isolated packages/consumer pass.
Native sanitizer checks pass for fractional conversion and 16-bit boundary scaling;
optimized integer output IR has no floating or 64-bit arithmetic. COM encodings and
8-/16-bit register layouts are synthetic fixtures, not a concrete device backend.

ATmega328P bindings add seven runtime cases and nineteen static assertions. A byte
fixture models Timer1's shared latch, W1C flags and force-compare strobes. It checks
all three timers, physical output routes, byte order, barrier scopes, preserved
fields and concrete frequency/duty updates. 276 raw facts match upstream avr-libc;
one valid/three rejected native device probes pass. Seventeen AVR headers and the
isolated production/host/install/consumer checks pass. Device selection and policies
are explicit. This does not validate hardware waveforms, asynchronous Timer2 clocks,
interrupt execution, target barrier code or board resource ownership.

Core compiler probes cover six valid applications, seventeen expected resource
failures, two dependency cycles and parameter-index selection/bounds. Peripheral
probes accept distinct pins and reject conflicts for raw and debounced inputs.
PWM adds two valid compositions and six expected pin/timer/range conflicts.
Storage adds three valid and eight invalid region/type/claim probes; timer
requirements add twelve static assertions and two valid/twelve invalid backend
configuration probes. Registers add six accepted and fifteen rejected field/access/selection/applier
probes plus relocated mapping assertions. Native packages
and host suites work in isolated checkouts with installed dependencies; an installed
peripheral consumer and a register consumer build and run without Catch2. See
[extraction progress](GrevirExtractionProgress.md).

The installed Make executor runs these builds; Ninja remains the selected
alternative when available. Shared presets, CI, interrupt fixtures, concrete MCU
inventories and hardware validation remain future work. Register memory is now
shared through the development-only `grevir::test_support` target. A fixture-only
consumer builds without Catch2; AVR and Registers production consumers build with
both Catch2 and Test Support discovery disabled.

## Selected stack

Code reviews follow the [cross-MCU policy](review-policies/CrossMcu.md), with
the [AVR supplement](review-policies/Avr.md) for AVR code and instantiations.
Host validation establishes neither AVR costs nor the appropriate numeric
representation for every target of the shared API.

Use **CMake + Ninja + CTest**, with **Catch2 for host test assertions/reporting** and
Grevir's own hardware fixtures as those tests become necessary. Core now has a
behavioral mock fixture; GPIO/clock fixtures now accompany the first peripheral
increment. Registers and AVR use the shared byte-array access fixture; interrupt
models remain planned.

Gianni accepted this recommendation on 18 September 2026. CMake, Ninja, CTest,
host-only Catch2 and the Grevir fixture approach are selected for the eventual
framework. His initial scope correction limited the first step to compilation.
On 20 September he authorized mock behavior tests and deferred hardware validation.
No MCU compiler, board selection or CI matrix is required for this Core increment.

| Choice | Fit for Grevir | Recommendation |
| --- | --- | --- |
| CMake | Per-library targets, exported dependencies, native tests and explicit cross-compilation toolchains; fits the existing packaging direction | Primary build description |
| Ninja | Executes the builds generated by CMake | Default executor; no handwritten Ninja files |
| Handwritten Makefiles | Viable, but we would maintain compiler/platform conventions, dependency exports and test orchestration ourselves | Keep old Makefiles as references; no second maintained build description |
| Bazel | A viable declarative build/test system; adopting it here also entails owning MCU toolchain rules and SDK/build integrations | Revisit if measured build scale or an established Bazel consumer justifies it |

These choices reflect project fit, not claims that Make or Bazel cannot handle
embedded C++. Ninja documents CMake generation on Linux, macOS and Windows.
[CMake toolchains](https://cmake.org/cmake/help/latest/manual/cmake-toolchains.7.html),
[Ninja manual](https://ninja-build.org/manual.html),
[Bazel C++ toolchains](https://docs.bazel.build/versions/main/tutorial/cc-toolchain-config.html).

If “Blaze” means Google's build system, Bazel is its public counterpart.
[Bazel glossary](https://bazel.build/versions/7.3.0/reference/glossary?hl=en#blaze).

CMake's presets provide shared configure/build/test settings. CTest runs the test
programs and other registered checks. Catch2 provides the host-side cases, assertions
and reports, with CMake integration for discovering tests. Pin a tested Catch2 v3
release during implementation; keep it a development dependency, absent from MCU
firmware and installed public headers. Do not create another general-purpose test
runner when the custom work we need is the hardware model.
[CMake presets](https://cmake.org/cmake/help/latest/manual/cmake-presets.7.html),
[CTest](https://cmake.org/cmake/help/latest/module/CTest.html),
[Catch2 integration](https://catch2-temp.readthedocs.io/en/latest/cmake-integration.html).

## Initial scope — compile with Xcode Clang (18 September)

Use `xcrun clang++` as the native macOS compiler. Compile existing representative
source with the standard library enabled and the existing Arduino mock where needed.
Instantiate actual template users; produce object files without requiring a running
application, a hardware simulator, Catch2 or an MCU toolchain. Capture missing
includes and host declaration problems as ordinary compilation issues.

The initial local check used Apple Clang 21.0.0, targeting arm64 macOS, with
`-std=c++23 -DHAS_STD_LIB=1 -c` and the original `ardOinus/src` include directory:

| Existing source | Result |
| --- | --- |
| `ardOinus/tests/dependent_module.cxx` | Compiled to a native object without warnings |
| `ardOinus/examples/Control/ArdOArrays/ArdOArrays.ino` | Compiled to a native object using `-x c++ -DARDO_USE_MOCK_ARDUINO=1 -include utility`; existing warnings remain |

The Arrays compile first exposed a missing declaration of `std::move` in
`setl_optional.h`. Explicitly pre-including `<utility>` allowed the probe to compile;
the source header has not been repaired by this check. No MCU identity macros or
target compilers were used. This is evidence for those two translation units, not
a successful build of every legacy file or a finalized cross-MCU language baseline.

The runnable [temporary native compile check](../native-compile-check/README.md)
provides two CMake object-library targets and the minimal mock/include wiring they
need. Object compilation was the acceptance criterion for that initial probe.
Core mock runtime checks now follow it; compiler matrices and broader fixtures
remain planned, with hardware validation on hold.

## What runs without Arduino

The default host build must require only a suitable native C++ compiler, CMake,
Ninja and provisioned test dependencies. It must not require the Arduino IDE/CLI,
an Arduino core, an ESP SDK, an MCU compiler or attached hardware. Dependency
acquisition may happen during an explicit setup step; test execution itself must
not download anything. Pin fetched dependencies and support preinstalled/local
sources for disconnected builds.

Compile the production application composition, allocation, GPIO/timer configuration
and other backend logic against injected hardware/service policies. Use a host
clock, register storage and modeled peripheral events. For Arduino adapter tests,
provide test-only Arduino API declarations/implementations, scoped to those test
targets. Never install those fake headers as production headers or place their
include directories on unrelated targets.

Keep the simulated MCU separate from the host compiler's architecture. Both AVR
and ESP32 fixtures run on Linux, macOS and Windows. Target-specific assembly, ISR
attributes and SDK entry points stay in small hardware binding layers exercised
by real target builds. Do not define `__AVR__` or `__XTENSA__` on a host compiler to
force the hardware path to compile.

The [repository plan's fixture contract](GrevirRepositoryPlan.md#6-run-avr-and-esp32-tests-in-ordinary-host-processes)
remains the source of ownership for clocks, GPIO, serial, storage and interrupt
models. This plan supplies their build and test execution framework.

## Test layers

| Layer | What it proves | Execution |
| --- | --- | --- |
| Positive compile checks | Valid type composition, constants, field encodings and allocation results satisfy `static_assert` contracts | Native compiler on every host; selected cases also built by target compilers |
| Expected compile failures | Conflicting claims, impossible allocation and unsupported mandatory requirements are rejected for the intended reason | Separate compiler-invocation tests driven by CTest |
| Native runtime fixtures | Actual module/backend logic produces expected state, register writes, timing and interrupt behavior | Catch2 executables using AVR and ESP32 fixture models on all three host OSes |
| Target compile/link checks | Real headers, ABI, startup/interrupt bindings, compiler restrictions and flash/RAM requirements work together | AVR toolchain and selected ESP32 SDK/core; no hardware required just to compile/link |
| Package/Arduino integration | Published layout, includes, dependencies and real sketches work outside the workspace | Fresh consumer projects and isolated Arduino CLI builds |
| Focused hardware tests | Actual timing, electrical effects, interrupt delivery and device/SDK behavior agree with the models | Named AVR/ESP32 boards, separately scheduled |

The fast default suite comprises the first three layers. Compiler-only assertions
must be instantiated by actual test translation units; parsing an unused template
is insufficient. For expected failures, first prove the toolchain/setup works,
then require compilation failure and a stable Grevir diagnostic identifier. A
missing include, failed compiler launch or unrelated syntax error must not count as
a passing collision test. Do not match entire compiler-specific error messages.

Native models use explicit target integer widths and register semantics, bounded
virtual time advancement, observable I/O traces and reset between tests. Model
MCU-specific side effects in the MCU fixture. Reuse behavior tests across backends
where their contracts agree; keep register-specific expectations target-specific.
Host sanitizer builds provide an additional development configuration where the
selected compiler supports them. They do not replace target ABI or hardware checks.

A concrete first runtime case: instantiate the production blinking module with
mock GPIO and clock bindings, advance virtual time over several transitions and
assert the output trace. Add a compile-fail companion that assigns two incompatible
claims to one pin. Then exercise a real AVR timer configuration against mock register
access; this proves the fixture reaches backend logic, not just a generic stub.

## Build ownership and multi-repository composition

No additional repository is required for this framework. Use the proposed names
`grevir-mcu-avr` and `grevir-mcu-esp32` below; the existing inventory still uses
`grevir-avr` and `grevir-esp32` pending application of that naming proposal.

| Location | Responsibility |
| --- | --- |
| Each library's root `CMakeLists.txt` | Its production target, explicit dependency targets, include interface and install/export rules |
| Each library's `tests/CMakeLists.txt` and `tests/` | Cases owned by that library; enabled explicitly for development |
| `grevir-test-support/include/grevir/test/` | Shared native fixture primitives |
| `grevir-test-support/cmake/GrevirTesting.cmake` | Small reusable helpers for host cases and expected compiler failures |
| `grevir-test-support/tools/check_compile_failure.py` | Optional cross-platform diagnostic-check helper, if CMake alone becomes cumbersome; Python would then be a documented host-test dependency |
| MCU backend's `tests/support/` | AVR or ESP32 device models and test-only SDK bindings |
| `grevir-wz/CMakeLists.txt` and `CMakePresets.json` | Workspace host build, selected checked-out components and common test presets |
| `grevir-platforms/cmake/toolchains/` | AVR compiler/linker configuration and other applicable target build settings |
| `grevir-platforms/tests/` | ESP-IDF/Arduino consumer projects and orchestration for supported target builds |

Use `INTERFACE` targets for header-only libraries and normal compiled targets where
implementation sources exist. Declare language requirements and dependencies on
targets rather than global include paths or flags. Select the C++ baseline through
the already planned AVR/ESP32 feature probes; this build-system choice does not
lower the modern C++ goal to fit an old default compiler.

Each repo must build independently using explicitly supplied or installed dependencies
and also participate in a workspace build. The workspace can register local
production targets in dependency order, then fixtures and test targets; a library
uses an existing dependency target or resolves its installed package. Test support
must not become a production dependency or a requirement for Arduino consumers.
A normal library-only configure must not fetch/build Catch2 or run host tests.

GWZ continues to own checkout membership and revisions. CMake consumes the selected
checkouts; it must not clone, rename or edit GWZ members. Avoid hardcoded absolute
paths and implicit sibling-directory searches inside independently packaged libraries.
A workspace preset is convenient, not mandatory for consumers.

## Keep each compiler build separate

Use separate build directories for host debug, host sanitizers, AVR and each ESP32
configuration. Never try to switch the C++ compiler inside one configured CMake
build tree. CMake toolchain files describe cross-compilation; target applications
are not native executables to be launched by host test discovery.
[CMake toolchains](https://cmake.org/cmake/help/latest/manual/cmake-toolchains.7.html).

For ESP32 hardware builds, use ESP-IDF's supported CMake/component integration or
the selected Arduino core's real build process. ESP-IDF already has a CMake-based
build system with SDK configuration, component and link integration; our host build
should orchestrate it as a separate build, not attempt to replace it with a compiler
path and a few flags.
[ESP-IDF build system](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-guides/build-system.html).

Arduino CLI builds likewise remain real Arduino builds. The source repos retain
their root `library.properties` and `src/` layout; CMake metadata is additional.
The supported board/compiler package supplies the selected C++ flags and tools.
Compile the common examples with board-specific bindings on both supported targets.
The native suite runs even when all target SDKs and Arduino installations are absent.

## Proposed everyday commands

After the presets are implemented, the normal workspace workflow should be:

```sh
cmake --preset host-debug
cmake --build --preset host-debug
ctest --preset host-debug --output-on-failure
```

Register fixture tests with labels such as `mock-avr` and `mock-esp32`, so a developer
can run just one family using `ctest --preset host-debug -L mock-avr`. Separate CI
jobs run target builds, clean-package consumers and hardware exercises. These are
proposed commands, not a claim that working presets have already been created.

Pin suitable host compiler versions in CI: GCC/Clang on Linux, Clang on macOS and
a validated MSVC or clang-cl environment on Windows. Exercise both target fixtures
in each OS job. Choose compiler versions against the selected language features,
not merely their ability to accept a `-std=` spelling. Do not silently skip an
unsupported compiler case and report the matrix as complete.

## Deferred framework milestones

Each step is one goal, with an aspirational budget below 500 authored/changed lines.
Repeat per component where indicated; mechanical source moves are separate.
These broader milestones follow the immediate compile-only scope; none is an
additional condition for establishing that the code compiles with Xcode Clang.

### Phase 1 — One native test with no Arduino installation

| Step | Goal | Budget | Evidence |
| --- | --- | ---: | --- |
| 1.1 | Add independent CMake target/export conventions for the first portable component | <250 LOC | Standalone and parent-workspace consumers use the same target |
| 1.2 | Wire pinned Catch2 and CTest into the explicit host-test configuration | <200 LOC | A real existing portable test runs without Arduino or target tools |
| 1.3 | Add shared host configure/build/test presets | <200 LOC | Same command sequence works in a clean developer environment |

### Phase 2 — A useful deterministic hardware fixture

| Step | Goal | Budget | Evidence |
| --- | --- | ---: | --- |
| 2.1 | Implement clock and GPIO fixture primitives, one coherent increment each | <300 LOC each | Real blinking module produces the expected trace using virtual time |
| 2.2 | Register positive and expected-failure resource claim cases | <350 LOC | Valid control builds; conflicting claim fails for the expected reason |
| 2.3 | Connect one AVR timer configuration to mock register access | <400 LOC | Production configuration logic produces validated register writes |
| 2.4 | Connect the equivalent ESP32 behavior through an SDK/service fixture | <400 LOC | Same behavior contract exercised through its production backend |
| 2.5 | Establish the six host-OS/target-fixture CI combinations | <250 LOC | Both fixture families run on Linux, macOS and Windows |

Start 2.4 when the initial ESP32 backend exists. Until then, report it as outstanding;
an empty test executable or a duplicate algorithm is not completion. Additional
serial/storage/interrupt models follow their peripheral implementation milestones.

### Phase 3 — Extraction backed by tests

| Step | Goal | Budget | Evidence |
| --- | --- | ---: | --- |
| 3.1 | Add each library's owned tests and explicit fixture dependencies | <250 LOC per library plus moves | Its native tests work in isolation and the workspace |
| 3.2 | Add an installed-package consumer test per dependency closure | <250 LOC each | Build succeeds with contributor workspace unavailable |
| 3.3 | Add supported host sanitizer configuration | <200 LOC | Applicable native suites run with the selected instrumentation |

### Phase 4 — Verify the real delivery targets

| Step | Goal | Budget | Evidence |
| --- | --- | ---: | --- |
| 4.1 | Add one AVR real compile/link configuration | <350 LOC | Representative application links; size/map output retained |
| 4.2 | Add the named ESP32 SDK/core build configuration | <350 LOC per integration increment | Real SDK headers, linkage and startup validated |
| 4.3 | Add isolated Arduino example builds per target | <300 LOC each | Direct-clone library layout and board/compiler package work together |
| 4.4 | Add one focused hardware regression per supported peripheral | <400 LOC each | Device observations checked against defined behavior/model expectations |

The immediate native compile check comes first. Introduce the broader checks as
the corresponding work needs them; they are not part of the current compile-only
acceptance criterion. Their compiler versions, fixtures and results remain future
implementation work.

## Experimental portable timer MVP checkpoint — 21 September 2026

The standalone timer-allocation experiment now adds declaration-generated ATmega328P
Timer0/1/2 fast PWM and typed register setup/duty bindings. Its four sanitizer-backed
CTest checks include the existing allocator/frequency oracles plus all six PWM
routes, shared-channel independence, one-tick/endpoints, full 16-bit period,
invalid writes, reservations/conflicts and 168 exhaustive period-search comparisons.
The production suite remains 123 cases. All 18 AVR public headers and nine experiment
headers compile independently; isolated AVR production/host/install/consumer checks
pass. Raw-token brace checks cover 154 production/test C++ files and 14 experiment
files, including disabled branches. Native optimized setup/duty probes have no
floating or 64-bit arithmetic or runtime allocation; AVR cost is unmeasured.

Only the experimental fixed-frequency fast-PWM path is covered end to end. Its
README records explicit startup preconditions and duty rounding. MCU compiler and
hardware validation remain on hold; additional devices/timer features and a real
ESP32 backend remain TBD.

## Installed PWM integration checkpoint — 21 September 2026

The current production suite has 125 cases (Registers 24, AVR 50; other counts
unchanged). It adds full-width uint32 register behavior and application-level PWM
lifecycle/ownership. The four standalone sanitizer checks now call the installed
implementation through forwarding headers. A production compiler control plus six
expected failures checks existing whole/range/shared timer claims, physical GPIO
claims, duplicate request identities and late bound claims. Four affected packages
(Core, Registers, Peripherals, AVR) pass isolated native production/install/consumer
checks with test-package discovery disabled for consumers. Twenty-one AVR headers
compile independently. Raw-token checks cover 168 production/test C++ files and
14 experiment files including disabled branches. Native optimized PWM setup/duty
probes show no FP, 64-bit arithmetic or dynamic allocation. Target compiler and
hardware validation remain on hold; no extra timer features were added.
