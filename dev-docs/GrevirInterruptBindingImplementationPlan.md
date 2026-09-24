# Interrupt binding implementation plan

Status: implementation plan for the [accepted interrupt binding
architecture](GrevirInterruptBindingArchitecture.md), reviewed at Git blob
`2dbe0d63fa8227cd34c17c69f37917bdb3d73a18` in the
[review outcome](GrevirInterruptBindingArchitecture-ReviewOutcome.md). Nothing
in this plan is evidence that an interrupt backend or generator exists today.
This is development documentation; the finished public API and examples belong
under `/docs` in the relevant package repositories.

## Result to deliver

A firmware application can specialize one
`grevir::on_interrupt<Event>() noexcept` function in its application header.
Grevir detects that specialization from the finite module event catalog,
selects one legal peripheral configuration and interrupt binding, writes a
canonical JSON plan, and emits and compiles one resident-target binding unit.
There is no second user-authored interrupt claim or registration list. The
generated C++ is write-and-forget for `grevir-irqgen`: the tool never parses it
back. A build accepts only a complete, current-attempt output set. The first
complete proof uses the mock MCU; AVR and classic ESP32 then exercise the same
contract with different entry implementations.

The first supported event is a timer period/alarm event with no event payload.
This first implementation slice rejects multiple logical events on one physical
source at C++ and JSON validation. The accepted architecture's shared-source
rules remain an extension contract, not a claim that a shared dispatcher is
implemented in this slice.
The mock inventory must also include a PWM configuration that can coexist with
the event and one that conflicts with it. The first AVR target is ATmega328P
Timer1 overflow; the first ESP32 target is the classic ESP32 Dev Module with
Arduino-ESP32 3.3.11. Timer Group 0 / Timer 0 alarm is a **provisional** ESP32
source until its register, interrupt-source and Arduino-core ownership are
verified. Do not substitute LEDC PWM for this timer event or imply support for
ESP32-S2/S3, ESP32-C3, other AVR timer events, UART or pin interrupts.

## Existing code and ownership

| Area | Current seam | Planned owner |
| --- | --- | --- |
| Application closure and setup | `grevir-core/src/grevir/core/allocated_application.hpp` collects `RequestedModule` requests and calls `Allocation::setup()` and module setup without a one-shot/result guard. `ardo::Application` supplies dependency order. | `grevir-core`: preallocation `ApplicationSpec`, event catalog, demand detection, strict gate, final runner and `StartResult`. Keep the existing non-interrupt path working while its users are migrated. |
| Search and timer requests | `grevir-core/src/grevir/core/allocation/search.hpp` supplies bounded search; `grevir-peripherals/src/grevir/peripherals/pwm` models PWM candidates and stable request keys. No selected interrupt binding is represented. | `grevir-peripherals`: event-capable complete timer candidates and binding validation, using core search. |
| Mock target | `grevir-test-support` has host fixtures but no interrupt controller or generated-source path. | `grevir-test-support`: synthetic inventory, controller, fault injection and end-to-end tests. |
| AVR | `grevir-avr` has ATmega328P timer definitions, PWM candidate generation and register/flag helpers; it does not install Grevir ISRs. | `grevir-avr`: one Timer1 event inventory, masking/status/acknowledgement policy and AVR entry adapter. |
| ESP32 | `grevir-arduino-esp32` currently provides GPIO, clock and serial adapters, not a timer/interrupt allocator. | `grevir-arduino-esp32`: named classic-ESP32 timer source inventory, owner callback bridge and registration/cleanup adapter. |
| Generation/build | Scratch Python probes use `nm` and a handwritten selected plan; `grevir-core` has no installed irq generator or binding CMake function. | `grevir-core/tools/grevir_irqgen` and `grevir-core/cmake`: target probe, JSON schema/validator, target emitters and build transactions. The scratch generator is evidence, not production code. |

Use the [cross-MCU policy](review-policies/CrossMcu.md) for shared code, plus
the [AVR](review-policies/Avr.md) or [ESP32](review-policies/Esp32.md) supplement
for each target. Host tooling and mock tests have no firmware runtime budget.
AVR runtime paths remain integer unless a fractional contract requires
otherwise. Review ESP32 setup and callback concurrency explicitly.

## Work sequence

### 0. Prove the two risky seams before broad refactoring

1. Build a small, real `RequestedModule`/dependency-closure example with a
   stable nested timer event and an inline handler that calls a preallocation
   module facade. Compile demand detection **before** allocation and the same
   handler in a strict final build. No second handler declaration or handwritten
   event registration may be needed. If the facade creates a template cycle,
   revise the public declaration/definition contract and re-review that change
   before continuing.
2. Emit a pointer-free, relocation-free `.grevir_irq_plan` section from a
   target-compiled C++23 probe. Exercise extraction and malformed/missing/
   duplicate-section rejection on the host object formats required for mock
   builds (Mach-O, ELF and COFF). The tool may use an object-format adapter;
   neither diagnostic prose nor demangled symbols are a production transport.
   If an object format cannot carry the section reliably, revise the transport
   contract before building the generator around it.

**Exit:** both proofs run from repeatable commands and document their compiler,
object format and limits. A failure here is a design gate, not permission to
add a handwritten plan fixture to the production path.

### 1. Establish the shared event and application contract

Add `grevir::ApplicationSpec`, `EventCatalog<Spec>`, stable event identities,
the deleted-primary `on_interrupt<Event>()` API and the generated-key
`BindingGate<Event>`. Derive the catalog from the module dependency closure
before hardware selection. Detect only visible specializations of catalog
events; an absent specialization creates no demand. Normalize and sort module,
request and event keys, and reject duplicates before allocation. Define the
preallocation facade and the strict include order once, in the public header
and a minimal example. Do not enumerate arbitrary `.cpp` or `.ino`
specializations; the application header is the discovery boundary.

**Exit:** host compile checks cover mapped, unbound, foreign same-key and
noncatalog handlers; a declared-but-undefined handler fails final linking;
reordering module declarations preserves the normalized catalog and demands.
Existing non-interrupt application tests still pass.

### 2. Add interrupt-bearing candidate and binding validation

Extend the peripheral candidate model so a selected timer configuration states
its PWM outputs, offered events, physical source, selector, entry and
acknowledgement policy together. Feed exactly the detected demand set into the
bounded allocator. Validate `events(bindings) == demands`, one binding per
event, legal selected mode, one owner/entry per physical source, exclusive
entry conflicts, reservations and canonical order. Multiple logical events on
one source are allowed only when its backend metadata supplies a single status
snapshot, distinguishable selectors, one acknowledgement policy and stable
dispatch order. Keep physical peripheral sources distinct from ESP32 CPU lines.

**Exit:** synthetic inventories prove PWM/event coexistence and conflict,
impossible demands, duplicate stable identities, source-owner conflicts,
shared-source dispatch legality, allocation exhaustion and declaration-order
independence. No AVR register code is required for this gate.

### 3. Implement the structured probe-to-JSON tool

Implement `grevir-irqgen` as a host Python tool under
`grevir-core/tools/grevir_irqgen`, reusing only suitable ideas from
`scratch/interrupt-detector`. Keep the initial host dependencies to Python's
standard library and named object-extraction tools. Define a versioned binary
section schema and a canonical JSON schema containing resident target/board,
compiler and application fingerprints, detected event keys, selected bindings,
source groups and backend policy IDs. The probe serializer writes explicit
bytes rather than C++ structs with ABI padding. `plan` decodes, validates and
writes JSON; `emit` reads that JSON, validates its internal invariants and
delegates to the selected backend emitter. The allocator, not JSON or Python,
chooses hardware. Document that low-level `emit` alone cannot prove freshness
against changed source files.

**Exit:** byte-for-byte deterministic JSON under declaration reordering;
malformed, stale-schema, mismatched target, duplicate, relocated and tampered
records fail with stable diagnostics. The generator reads the probe object and
JSON, never generated C++.

### 4. Complete one mock firmware round trip

Add a synthetic interrupt-capable timer and deterministic interrupt controller
to `grevir-test-support`. Emit
`grevir_generated_irq_bindings_mock.hpp/.cpp` from the JSON and compile them
into a mock application. Include the zero-handler generated installer and the
source-specific strong guard. Implement `Application<Spec>::start()` with a
settled setup outcome and separate `initiated`/`waited`/`replayed` call
disposition. Setup masks sources, configures owners and modules, installs one
source callback, applies the declared startup pending-event policy, then
enables sources. Failed setup is terminal for that firmware session; it masks
all owned sources and tracks release failures without retrying registration.

**Exit:** an end-to-end mock test invokes the user's handler once for a raised
event. Fault tests cover no handler, missing generated unit, unbound handler,
duplicate generated units, two FIFO-like consumers of one source, a second
registration failure, concurrent/repeated starts, status snapshot/acknowledge
order, and a pending event asserted during startup and during acknowledgement.
Both fields of `StartResult` are asserted for success and failure. These tests
prove Grevir semantics, not AVR timing or ESP32 routing.

### 5. Make generated outputs a build transaction

Add `grevir_add_interrupt_bindings()` to `grevir-core/cmake`. Use the same
application header and `CONFIG_TARGET` for the target-compiled probe and final
firmware. One CMake custom command runs `plan` and `emit`; its primary output
is a ready marker and its byproducts are JSON, header and source. On every
generation attempt, invalidate the old marker first. Publish the new marker
atomically only after the complete output set exists; include attempt,
plan, target, compiler and emitter/protocol identities. The adapter accepts
only that ready attempt and never falls back to older files. Generated C++
contains matching emission constants checked by strict compilation, without
generator read-back. Keep paths isolated by target and CMake configuration.

**Exit:** installed-consumer and multi-configuration builds compile the mock
binding unit on macOS, Linux and Win11/MSVC as applicable. Inject failure
before JSON, after JSON, after each C++ file and before marker publication,
with an older valid set present; none may compile or export a mixed set.
Changing handlers, board, target, compiler flags or emitter version triggers
regeneration or a strict-build failure. Parallel builds do not share output
paths. Use the existing MSVC-specific compile-check command handling rather
than assuming GNU/Clang flags.

### 6. Add one AVR target binding

Extend the ATmega328P timer inventory with one Timer1 period event and its
actual source/vector, legal PWM coexistence, mask/status/acknowledge behavior
and core/external reservations. Emit
`grevir_generated_irq_bindings_avr.hpp/.cpp` through the same JSON schema and
generator engine. The AVR adapter owns the ISR ABI and register operations;
the common tool does not duplicate vector or register inventories. Keep ISR
work bounded and integer-only unless the event contract demands otherwise.

**Exit:** named `avr-g++` compile/link and vector-symbol inspection show one
selected entry, no entry for an absent handler, and a conflict failure for a
reserved/core-owned vector. Use the authorized weftpi AVR toolchain and simavr
for behavior that its model supports; record compiler and device. Hardware
silicon validation remains on hold. Timer0/2 and capture/compare variants are
separate follow-on slices, not prerequisites for this event.

### 7. Add one classic ESP32 target binding

First verify the provisional Timer Group source against the selected classic
ESP32 Arduino core: direct peripheral configuration ownership, interrupt
source identity, status/acknowledgement, core reservations, CPU affinity and
IRAM/callback constraints. If it is unsuitable, select another concrete timer
source and record the reason before coding its backend. Emit
`grevir_generated_irq_bindings_esp32.hpp/.cpp` from the same JSON plan. Install
one callback bridge per owned physical source, retain the handle, check
registration errors and enforce the terminal failed-start/cleanup state. The
callback uses the backend's one-snapshot/one-acknowledgement policy and calls
the specialized handler in its documented interrupt context.

**Exit:** classic ESP32 Dev Module / Arduino-ESP32 3.3.11 target compile/link
accepts one owner, rejects conflicting or unsupported configurations, and
checks generated registration/cleanup paths. A named-target error-path check
covers failure after one of two registrations. Do not claim task safety, IRAM
safety or physical timing solely from host mock results or target compilation.
S2/S3 and RISC-V targets remain separate backends with their own inventories
and policies.

### 8. Integrate Arduino builds and publish API documentation

Implement `grevir-irqgen arduino-build` as the supported staged-sketch
transaction: obtain the probe compile command from Arduino CLI's compilation
database, compile the target probe, generate the ready JSON/header/source set,
remove the probe source, compile the strict staged sketch, and export firmware
only after link success. Preserve FQBN, library paths and build properties
between probe and strict passes. Prove both named AVR and classic ESP32 builds;
do not claim direct IDE or bare `arduino-cli compile` integration until those
paths generate bindings themselves.

After the API and commands actually work, add `/docs` pages and copyable
examples for one common declaration with resident AVR/mock/ESP32 sections,
handler visibility, source ownership, build commands, diagnostics and ISR
context. Update each affected repository README with its implemented support
boundary. Keep design discussion, review reports and implementation evidence
in `dev-docs`.

**Exit:** clean example builds from an installed package set, with negative
checks for changed target options and a stale plan. User docs describe only
implemented backends and distinguish compiler, simulator and silicon evidence.

## Review and completion rule

At each public interface or wire-schema change, review the shared contract
under `CrossMcu.md`; add `Avr.md` or `Esp32.md` for target code. Use the
`review-loop` adversarial review process for interface-freeze and final
acceptance gates, with peer-blind reviewers
given exact scope, target assumptions and validation holds. Record each
checkpoint's exact repository revisions and evidence rather than inferring
readiness from the design review.

The first implementation milestone is the **mock end-to-end round trip through
CMake** (steps 0–5). The first cross-MCU implementation slice adds the named AVR
and classic ESP32 bindings and Arduino wrapper (steps 6–8). Neither milestone
requires every AVR timer mode, LEDC PWM, MCPWM, UART/CAN interrupts, arbitrary
handler discovery or silicon hardware validation.
