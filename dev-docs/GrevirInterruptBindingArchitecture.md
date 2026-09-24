# Interrupt binding architecture

Status: production design specification. The build integration and target
emitters described here are **not implemented**. The host scratch probes prove
individual language and validation steps, not an end-to-end firmware build.
This document is the authority for the proposed implementation; the earlier
[design investigation](GrevirInterruptBindingDesign.md) records alternatives and
prototype evidence. The [architecture review](GrevirInterruptBindingReview.md)
records corrected design defects and remaining implementation gates.

## Decision and boundary

One application binding plan is the source of truth for interrupt ownership.
The application defines a specialization of `grevir::on_interrupt<Event>()` to
request handling of a logical event. Grevir discovers these specializations
among the finite event types offered by the application's module closure,
feeds the detected events into the target allocator, and emits target entry
code from the resulting plan. The same checked plan drives source configuration,
registration, dispatch and enabling. Applications do not author an additional
interrupt claim, vector list or registration table.

The pipeline is common to the mock MCU, AVR and ESP32. Only the final target
entry code differs: the mock MCU installs a deterministic simulated source
callback, AVR defines the selected vector entry, and ESP32 installs one callback
for each owned peripheral source. A future target supplies its own entry
adapter, not a new discovery or ownership policy. This design guarantees
exclusivity within one Grevir-managed firmware application. It cannot prove that unrelated
libraries or hand-written target code do not also manipulate a source; those
uses require explicit reservations or target runtime failure handling.

## Names and ownership

| Name | Role | Owned by |
| --- | --- | --- |
| `grevir::ApplicationSpec` | Application's board and module dependency closure, before hardware selection | `grevir-core` |
| `grevir::Application<Spec>` | Final firmware runner; `start()` returns a setup result and `runLoop()` runs modules | `grevir-core` |
| `grevir::on_interrupt<Event>() noexcept` | One public specialization is the handler request | Application; declaration in `grevir-core` |
| `grevir::interrupt::EventCatalog<Spec>` | All logical events the active module closure could offer | `grevir-core` and peripheral packages |
| `grevir::interrupt::BindingPlan<Spec>` | Detected demands plus the selected, validated target bindings | `grevir-core` contract; target allocator implementation |
| `grevir::interrupt::detail::BindingGate<Event>` | Generic strict-build check of catalog membership and binding; not public API | `grevir-core` |
| `grevir::interrupt::detail::BoundEventKey<Key>` | Generated specialization keyed by stable event identity; not public API | Generated header |
| `grevir-irqgen` | Host build tool: convert a probe object to a validated JSON plan, emit binding files from that plan, and orchestrate Arduino CLI builds | `grevir-core/tools/grevir_irqgen` |
| `grevir_add_interrupt_bindings()` | CMake function that owns the probe and generated-source dependency chain | `grevir-core/cmake` |

The names above supersede the scratch names `DetectorImpl`, `BindingId`,
`generate.py`, and handwritten `selected_plan.json`. `BindingGate` makes an
unmapped or noncatalog specialization fail during the final build; it does not
discover handlers. The public handler keeps one template argument; its `Event`
type identifies the module instance, peripheral request and event kind.

The first application integration should use a self-contained C++ header,
passed to the build tool as `--application-header`. It declares the module
instances and `using GrevirApplication = grevir::ApplicationSpec<...>;`, then
includes the handler API and the user's handler-specialization header. For
example (spelling of the module declaration is illustrative):

```cpp
// application.hpp: included by both the probe and final firmware.
using GrevirApplication = grevir::ApplicationSpec<Board, MotorModule>;
#include <grevir/interrupt/handler.hpp>
#include "interrupt_handlers.hpp"

// interrupt_handlers.hpp
template <>
inline void grevir::on_interrupt<MotorInstance::Timer::PeriodElapsed>() noexcept {
  MotorInstance::tick();
}
```

The actual header must declare the event type before the specialization. The
specializations must be visible to the probe and to the generated binding
translation unit. They may have inline definitions in that header, or visible
explicit-specialization declarations with exactly one definition linked into
the firmware. A specialization hidden solely in an arbitrary `.cpp` or `.ino`
cannot be discovered in standard C++ without an additional registry and is
outside this contract. The build checks a declared-but-undefined handler by
the final link; inline definitions are the recommended form. There is no
compatibility requirement for the current portable timer API.

The probe also parses any visible inline handler body before allocation has
finished. Therefore a handler must be able to call a preallocation module
facade whose interface is declared independently of the selected binding; its
implementation may use the final binding later. A handler body that directly
instantiates the not-yet-selected allocation would create a cycle. This is an
integration gate: prove a representative handler using an allocated mock timer
without adding a second user-authored handler declaration. If that cannot be
made to compile, revise the declaration/definition contract before treating
this API as settled.

The firmware entry point uses the final runner, for example
`grevir::Application<GrevirApplication>::start()`. Its `StartResult` contains
two independent fields: a settled setup outcome (`success`, peripheral
configuration failure, target registration failure, or cleanup failure with
the originating failure retained) and a per-call disposition (`initiated`,
`waited`, or `replayed`). A repeated call never replaces the setup outcome with
a generic repeated-start error. The application chooses its failure policy;
the runner never silently continues with an uninstalled requested interrupt.

## Event catalog, demand and allocation

`ApplicationSpec` must expose the complete dependency closure **before**
hardware selection. Each peripheral request contributes a finite list of
possible logical event types and target-independent semantics. The catalog is
derived from the closure; the user does not list handlers twice. An event has a
stable key derived from `(module instance key, local request key, event kind)`.
Keys are explicit identity components, validated for uniqueness and sorted
canonically. Type names, declaration order, addresses and object-file symbol
order are never identity. The existing PWM design's named request keys are the
starting point for this rule.

For each catalog event `E`, the probe evaluates whether a visible
`on_interrupt<E>()` specialization is callable. It then supplies exactly the
detected set `D` as mandatory interrupt demands to the selected target
allocator. Common configuration and sections applicable to the resident MCU
constrain allocation; nonresident target sections are inert. An active event
that the target cannot provide, or that conflicts with a selected timer mode,
must fail allocation. A missing specialization causes no interrupt demand or
entry. An attempted specialization of an event absent from the catalog fails
the strict build at `BindingGate<Event>`; it is not silently ignored.

The allocator produces one complete configuration and a binding record for
each handled event. A binding record contains:

- logical event key and catalog type lookup key;
- owning module/peripheral instance and selected request/configuration key;
- physical peripheral interrupt source key and, when needed, distinguishable
  status selector within that source;
- target entry key (AVR vector identity or ESP32 registration policy), plus
  priority/affinity/IRAM constraints where the target supports them;
- backend dispatch, masking and acknowledgement policy identity.

These are compile-time metadata. Keys and validation records do not imply
runtime strings or wide arithmetic in an AVR ISR. The backend defines the
physical meaning of a source; an ESP32 CPU interrupt line is not itself the
source key, since different peripherals may share a line.

Plan validation is mandatory and happens before any source is emitted:

1. The set of bound event keys equals `D`, and each appears once. No binding
   exists for an unhandled event.
2. Every bound event belongs to the active catalog and its selected owner
   belongs to the application's module closure.
3. Each selected binding is offered by the selected peripheral configuration;
   mode, TOP register, channel, pins, reservations and other capability
   constraints remain in force.
4. All records for one physical source have one owner and one entry. Multiple
   logical events on a source are accepted only if the backend provides one
   status snapshot, distinguishable selectors, one acknowledgement policy and
   a defined dispatch order. Indistinguishable competing consumers, including
   two UART FIFO readers, fail.
5. No two source groups claim an exclusive vector or other exclusive target
   entry. Any target-supported sharing of an ESP32 CPU line is checked against
   target capability metadata; it is not mistaken for shared source ownership.
6. The canonical plan is independent of module and option declaration order.
   Duplicate stable identities, incomplete capability metadata and allocator
   search exhaustion produce distinct failures rather than arbitrary choices.

If a timer has both PWM and an interrupt demand, the solver chooses one legal
mode for the whole timer. An event does not acquire a separate resource claim
that could disagree with that choice. Existing Grevir resource claims may still
reserve pins, timers or external peripherals; any interrupt-related claim used
by legacy conflict checking must be **derived from this plan**, never supplied
as a second binding authority.

## Probe object and generator protocol

The production probe is compiled with the **resident target compiler and the
same board, language mode, includes and relevant definitions** as the final
application. It includes the application header in probe mode, evaluates the
catalog, handler presence, allocator and validation at compile time, and
serializes the selected plan into one object-file section. The probe object is
never linked into firmware. This replaces parsing compiler diagnostics or
demangled C++ names and removes the scratch generator's handwritten catalog
and allocation-plan JSON inputs.

The proposed section is `.grevir_irq_plan`. Its contents are a pointer-free,
relocation-free byte stream with an explicit magic, schema number, target and
board identity, counts, length-prefixed UTF-8 keys, fixed-width little-endian
integers, canonical ordering and a checksum. The serializer writes bytes
explicitly rather than dumping a C++ struct with ABI padding. `grevir-irqgen`
uses an object-format adapter to extract the bytes and rejects a missing,
duplicate, malformed or relocated section. GNU-target adapters may use the
target `objcopy --dump-section`; the mock MCU adapter must handle the supported
host object formats (ELF, Mach-O and COFF) without changing the wire schema.
The probe's section-emission attribute or directive is likewise an adapter
for each supported compiler. No compiler-specific demangler is in the
production protocol. The AVR, ESP32 and host object-section/extraction
behaviors are implementation gates, not yet established by the host scratch
probe.

The probe emits both event presence and the **selected binding plan in the same
compile**. `grevir-irqgen plan` decodes the probe object, validates exact
event-set equality, source grouping and the other plan invariants, then writes
the canonical `grevir_generated_irq_plan_<backend>.json`. This JSON file is the
structured input to `grevir-irqgen emit`: the emitter reads and validates it,
then produces the header and source. The JSON is generated by the probe step,
not authored as a second application declaration; the C++ allocator remains
the sole chooser. The plan includes the schema version, resident target/board,
toolchain and application fingerprints, detected events, selected bindings and
source groups. The emitter rejects a missing, malformed, unsupported or
noncanonical plan, including duplicate keys and incompatible source policies.
It cannot determine whether valid JSON is stale relative to changed application
sources or compiler settings by inspecting that JSON alone. The build adapter
owns freshness: it runs the current probe, checks the recorded target/board
and effective compiler identity against its configuration, then invokes
`emit` on the JSON from that attempt. Strict C++ compilation independently
rejects a changed application binding plan. A direct low-level `emit`
invocation guarantees internal plan validity, not current-source freshness.
The probe object is parsed once to make the JSON; the generated C++ is never
parsed or read back by the generator.

Generated files are named for the selected backend. For the mock MCU, AVR and
classic ESP32 targets, the names are:

| File | Purpose |
| --- | --- |
| `grevir_generated_irq_bindings_<backend>.hpp` | `BoundEventKey` specializations and canonical plan constants for strict compilation |
| `grevir_generated_irq_bindings_<backend>.cpp` | One target entry/registration implementation per source group; dispatches to the appropriate `on_interrupt<Event>()` |
| `grevir_generated_irq_plan_<backend>.json` | Canonical, validated emitter input and readable record of demands, selected sources and diagnostics |

The backend suffix is `mock`, `avr` or `esp32` for the first three adapters.
For example, the mock build emits `grevir_generated_irq_bindings_mock.cpp`,
while the AVR build emits `grevir_generated_irq_bindings_avr.cpp`.

The suffix denotes the selected backend/toolchain contract, not a user-chosen
label. The build adapter selects the matching generated header and source; an
application never writes or includes a target filename manually. In strict
compilation the adapter sets `GREVIR_GENERATED_IRQ_HEADER` to the quoted name
of the selected header, which `grevir/interrupt/handler.hpp` includes. Probe
compilation uses the permissive gate and does not include a generated file. A
future backend with different peripheral or entry semantics gets its own
suffix.

The generated source refers to catalog events by stable key and resolves their
C++ types through `EventCatalog<GrevirApplication>` at compile time. The generated
header specializes `BoundEventKey<Key>`, never `BindingGate<Event>`. Each
specialization names its expected C++ type through the catalog's compile-time
`ByKey<Key>` lookup, so the generator emits only a stable key and no type
spelling. The generic `BindingGate<Event>` checks that `Event` is exactly that
catalog type and that the key was bound. A different type reusing a valid key
therefore fails. A host syntax probe in
[`scratch/interrupt-detector`](../scratch/interrupt-detector/README.md) verifies
the mapped, unbound and same-key foreign-type cases. The target package
owns the mapping from selected entry keys to actual AVR vector names or ESP32
source/flag values. The generated source invokes that package's entry API; the
core tool does not duplicate MCU register inventories.

Each emitted C++ file carries the plan fingerprint and emitter/protocol identity
from the JSON-driven emission. The strict compile compares generated constants
with the plan recomputed from the current application header and requires the
header and source to have the same emission identity. These are compiler checks,
not generator read-back of emitted C++. The JSON-to-output mapping is
deterministic for a given emitter version.

The build adapter owns one generation attempt, assigns it a fresh attempt ID
whenever generation runs, and gives it a private output directory. An
incremental build with unchanged inputs may reuse the previous ready attempt.
Before `plan` writes JSON, it removes that directory's `grevir_irq_ready.json`
completion marker. `plan` and `emit` write JSON, header and source through
temporary files in that directory. After all three writes succeed, `emit`
atomically publishes the ready marker, containing that attempt ID, the plan
fingerprint, emitter/protocol identity and output-set identity. The marker is the commit
point: an interrupted or failed `plan`/`emit` leaves no ready set, even if old
or partial files remain. CMake and Arduino adapters require the marker for the
current attempt and matching target/toolchain/emitter identity before compiling
or exporting firmware. They never fall back to a previous generation or accept
a marker from an earlier attempt. Identity checks use values recorded during
generation, without opening or parsing emitted C++. Build directories are
isolated per target/configuration, and one adapter owns each directory so
concurrent builds cannot interleave their writes.

The generated unit defines the only `install_bindings<GrevirApplication>()`
specialization, including when there are zero handled events. The application
runner always calls it, so bypassing generation causes an undefined-symbol
link failure rather than silently producing firmware without entries. Each
generated source group also emits a strong, source-specific C-linkage guard
symbol derived from the plan. Linking two Grevir binding units for the same
physical source then fails even if they came from separate application
closures. These symbols are consequences of the binding set, not another
source of ownership declarations.

In the strict build, a handler specialization without a generated gate fails
at its declaration. A removed or missing handler makes generated dispatch fail
to compile or link. The generated binding unit also recomputes the canonical
plan from the current application header and compares it with the emitted plan
at compile time. A changed request, mode, event set, board or target therefore
rejects stale output even if build dependency tracking misses the change.
Build failure must leave the current attempt unready; previous successful
generated files must never be reused as fallback for that attempt.

## Firmware lifecycle and target adapters

The selected `grevir_generated_irq_bindings_<backend>.cpp` is the only Grevir
entry registration unit for the application. Each source group produces
exactly one owner entry. The final application setup has explicit phases:
keep sources masked; configure selected peripherals and owner state; initialize
dependent modules; install target entries; handle pending conditions according
to each event's declared startup policy; then enable sources. There is no
unconditional pending-bit clear. Each offered event's backend metadata must
specify either preservation/delivery of a pending condition or intentional
discard with a quiescent precondition stated to application owners. A
preserve/deliver policy must snapshot, dispatch and acknowledge without losing
an assertion that arrives during that sequence. A backend unable to guarantee
its declared policy cannot offer that event. The mock tests inject an event
after dependent-module initialization and before enable, and check the chosen
policy; named target checks verify the corresponding status/acknowledgement
rules separately.

A setup failure keeps **all** owned sources masked and reports an error. The
runner enters a terminal failed-start state for that firmware session: there
is no implicit retry or second registration attempt. It releases already
acquired target registration handles in reverse order where possible; any
handle whose release fails remains owned and recorded in the failed state,
with that cleanup failure reported. On ESP32 the start state machine is
task-safe, not an unsynchronized Boolean. The initiating caller returns the
settled outcome with `initiated`; callers arriving while setup is in progress
wait for that same outcome and return it with `waited`; later calls return it
with `replayed`, without repeating setup. This applies after success and after
terminal failure, including the originating error and any cleanup error. The
mock test covers a first success followed by a repeated call, concurrent
success, and second-registration failure followed by concurrent and later
calls; it asserts both result fields, masking and handle ownership. The current
`AllocatedApplication::runSetup()` is not sufficient as written; its backend
setup call is not protected against repeated invocation.

The mock MCU backend lives in `grevir-test-support`, separate from production
firmware packages. It supplies a synthetic inventory of modes, sources,
selectors and reservations; the generated mock entry registers with a
deterministic in-memory interrupt controller. Tests raise a named source with
status bits and observe masking, one owner, one registration, a single status
snapshot and acknowledgement, dispatch order, pending-event startup policy,
setup failure and repeated-start
behavior. The mock's binding set is selected by the same allocator contract and
is emitted from the same probe wire record. It never reads a handwritten plan
fixture in place of discovery. It models Grevir's interrupt semantics, not AVR
vector timing or ESP32 CPU routing. Common requirements remain active on the
mock target; AVR and ESP32 sections remain inert, while explicit mock-specific
sections may constrain its synthetic inventory.

On AVR, the generated entry uses the target backend's vector mapping and ISR
ABI, then calls the owner's single dispatch routine. It must not replace a
core-owned vector silently. The selected backend supplies register masking,
status and acknowledgement behavior. On ESP32, the generated unit asks the
target backend to install one callback bridge for each source group, records
the handle and checks allocation errors. The backend owns affinity, interrupt
level, IRAM eligibility, acknowledgement and lifetime rules. Neither target
emitter may silently choose a weaker mode than the plan selected.

The handler runs in interrupt context and is `void`/`noexcept`. It must obey
the named target's interrupt-context rules; a later deferred-delivery API must
be a distinct event delivery contract. A peripheral owner may publish data or
events to dependent modules after it has read hardware state once. Dependent
modules do not register another hardware handler for the same source.

## Toolchain integration

`grevir-irqgen` is a host tool shipped with `grevir-core` so its wire schema,
C++ probe API and generated gate stay version-coupled. It has one plan-validation
and emission engine, with target entry adapters supplied by the MCU packages.
`grevir-irqgen plan` and `grevir-irqgen emit` are low-level adapter commands;
`grevir-irqgen arduino-build` is the user-facing build command. The normal
build command should be a thin
project script that calls `arduino-build`; users should not have to run probe,
`objcopy` and emit manually.

The proposed Arduino entry point is:

```sh
grevir-irqgen arduino-build \
  --sketch ParkLightsV2 \
  --application-header application.hpp \
  --fqbn esp32:esp32:esp32 \
  --output-dir build/esp32 \
  --library ../grevir-core --library ../grevir-peripherals
```

The wrapper forwards the project's remaining explicit Arduino build properties
and libraries without changing their meaning. It passes a canonical resident
target/board identity into the probe and checks that identity against the
extracted record before emission. The command shown illustrates the interface;
Parklights does not yet use interrupts or this tool.

For Arduino CLI, `arduino-build` performs the following transaction in an
ignored build directory:

1. Create an isolated attempt directory and invalidate its ready marker. Stage
   a copy of the sketch under a folder retaining the primary `.ino`
   filename, add a probe `.cpp` under `src/`, and select the resident FQBN,
   libraries, C++ mode and toolchain. Application specializations live in a
   normal C++ header rather than relying on `.ino` prototype generation.
2. Ask Arduino CLI for its compilation database in probe mode using the same
   sketch, FQBN, library paths and build properties. Select the probe source's
   compile command and compile only that source to an object, with LTO disabled
   for this metadata object. The wrapper must not guess the platform compiler
   path, include list or board flags.
3. Extract and validate `.grevir_irq_plan` into the canonical JSON plan, then
   emit the strict header/source from that JSON into the staged sketch's
   `src/` tree. Require the current attempt's ready marker before removing the
   probe source and compiling
   the staged sketch normally with Arduino CLI and the same settings, without
   the probe definition. Export the resulting firmware to the requested output
   directory only after the full compile and link succeeds.

Arduino CLI documents `--only-compilation-database`, `--build-path`, explicit
`--library` paths, `.ino` preprocessing and recursive compilation of sketch
`src/` files. These facts support the staged-sketch approach; the exact probe
command extraction, cache behavior and library discovery must still be tested
on the chosen AVR and ESP32 cores. Platform prebuild/prelink hooks are not the
default integration: their availability does not prove that a source created
after discovery is compiled in that invocation. A direct IDE or bare
`arduino-cli compile` invocation is not claimed to generate bindings; the
documented wrapper is the supported entry point until an IDE integration is
implemented. Parklights' current `build-esp32.sh` can delegate to this wrapper
when ESP32 interrupt bindings are actually introduced.

For CMake consumers, the proposed call is:

```cmake
grevir_add_interrupt_bindings(
  TARGET firmware
  CONFIG_TARGET firmware_config
  APPLICATION_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/application.hpp"
  MCU atmega328p)
```

Host tests use the same function with `MCU mock`, link
`grevir::test_support` through `CONFIG_TARGET`, compile the generated
`grevir_generated_irq_bindings_mock.cpp`, and invoke the mock controller.
The mock target is the first end-to-end proof of discovery, allocation,
serialization, generation, strict compilation, linking and runtime dispatch.
There is no mock-only generator or bypass around any of these stages.

`firmware_config` is an INTERFACE target carrying the resident compiler
definitions, include paths, board settings and Grevir dependencies. Both the
probe and firmware consume that same target. The function creates one
target-compiled probe object and one generation custom command that runs `plan`
and `emit` as one transaction. Its primary output is the ready marker; the
JSON, generated header and source are declared byproducts. An explicit
dependency from the firmware target to that command ensures the ready marker
exists before generated source compilation. It adds the generated source and include
directory to the firmware target. The CMake function owns all generated files
in the build tree, declares byproducts/dependencies, and supports parallel and
multi-configuration builds without sharing output paths. Header-only Grevir
libraries remain header-only; generated code belongs to the consuming
application, not to an installed global Grevir library. Application code must
not add plan-affecting compile definitions only to the firmware target; the
strict plan comparison rejects any resulting mismatch. The CMake adapter's
tests include deliberately divergent probe/firmware definitions. The ready
marker is the dependency output for the generation chain; failed `plan` or
`emit` invalidates it, so a later build must rerun generation before compiling
the source. The adapter checks the marker's attempt, target, compiler and
emitter identity before allowing firmware compilation. Tests inject failure
after the JSON and after each C++ output write.

Both adapters use one application entry header and one resident target
selection. They reject missing or ambiguous target/tool versions, mixed probe
and strict compiler settings, unsupported object formats, stale generated
files, and multiple generated binding units for one application. The tool
records the effective compiler identity and, for Arduino, FQBN; it tracks
relevant source, backend metadata, tool version and options as build
dependencies. The current-attempt marker is required before compilation;
strict C++ plan comparison is the final check for plan-affecting changes that
escaped dependency tracking. Files from an earlier generation may remain on
disk after a failure, but cannot be consumed as a ready set.

## Diagnostics and implementation gates

Diagnostics use stable application keys and physical source names, not mangled
types or incidental source order. At minimum, distinguish unknown or duplicate
event identity, handler outside the catalog, unsupported event, impossible
mode combination, source-owner conflict, vector conflict, allocation search
exhaustion, unsupported emitter/toolchain, malformed probe record, stale plan,
missing handler definition and runtime registration failure. The readable plan
must show why each handled event selected its source; inactive target options
must not appear as active constraints.

Implementation proceeds through these evidence gates:

1. Refactor the current application/timer path into preallocation
   `ApplicationSpec`, finite event catalog and handler-demand detection. Prove
   declaration-order independence and strict gate behavior on host compilers.
   Include a handler body that calls an allocated mock timer through the
   preallocation facade, so the probe/allocator cycle is tested rather than
   assumed away.
2. Add one interrupt-capable synthetic timer candidate and a plan validator.
   Prove exact demand/binding equality, unique ownership, shared-source
   dispatch rules, and a PWM/event mode conflict without target hardware.
3. Implement and exercise the mock MCU emitter through the complete CMake
   pipeline on supported host compilers and ELF, Mach-O and COFF object formats.
   Check that the validated JSON drives both generated C++ files without any
   generated-source read-back. Test positive and negative plans, failed
   publication, stale output, multiple application units,
   and simulated source events. This replaces the scratch's handwritten-plan
   fixture.
4. Prove the same byte-section protocol with the named AVR and ESP32 target
   compilers and their object extraction tools, including LTO-off probe builds,
   malformed-record rejection and deterministic output. If a target cannot
   emit the protocol, replace the transport while preserving the same
   catalog/plan/validation contract.
5. Implement AVR and ESP32 emitters from the same allocator-produced plan. Check AVR
   vector symbols and ESP32 one-registration behavior with named toolchains;
   validate external-core reservations. Target compilation does not establish
   physical interrupt behavior.
6. Prove Arduino staged-sketch builds and CMake generated-source dependencies,
   then test changed handlers, changed target options, parallel builds and
   intentionally stale outputs. Silicon validation remains a separate step.

No phase may claim full production readiness from the current host scratch
tests. The architecture does, however, fix the authority boundary now: a
handler demand and its actual ISR entry are linked by one allocator-produced
binding plan, not by two independently maintained declarations.

## Toolchain references

- [Arduino sketch specification](https://docs.arduino.cc/arduino-cli/sketch-specification)
- [Arduino sketch build process](https://docs.arduino.cc/arduino-cli/sketch-build-process)
- [Arduino CLI compile options](https://docs.arduino.cc/arduino-cli/commands-reference/arduino-cli_compile)
- [Arduino platform hooks](https://docs.arduino.cc/arduino-cli/platform-specification/#pre-and-post-build-hooks-since-arduino-ide-165)
- [CMake generated-source commands](https://cmake.org/cmake/help/latest/command/add_custom_command.html)
- [GNU objcopy section extraction](https://sourceware.org/binutils/docs/binutils/objcopy.html)
- [ESP-IDF interrupt allocation](https://docs.espressif.com/projects/esp-idf/en/v5.5/esp32/api-reference/system/intr_alloc.html)
