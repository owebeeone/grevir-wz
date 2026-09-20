# Grevir — potential goals and planning ideas

Recorded: 18 September 2026.

This is an ideas record, not an approved implementation plan. **Grevir is the
selected name for the new project**, and its **GWZ-managed multi-repository
workspace, `grevir-wz`**, has been initialized. The proposed direction is to
extract constituent modules from the existing Ardoinus monorepo into separate
projects. The exact boundaries remain open. See the [naming decision](README.md).

The consolidated [guiding principles](ArdoPrinciples.md) cover the API philosophy,
static state, resource-bound module instances, whole-application compilation, and
portability. They also distinguish the boot-selection idea and AI discussion
from settled implementation decisions.

## Purpose: push the embedded C++ experiment further

Gianni describes the original Ardoinus as an experiment demonstrating that C++
can provide embedded programming capabilities that standard C does not provide
directly: declarative composition, typed hardware descriptions, template-driven
operations, and compile-time checking of resource relationships. He considers
that original hypothesis proven and wants the revamp to push further and explore
what else becomes possible.

The multi-repository workspace supports that continuing experiment. Restoring
builds, extracting components, and comparing existing projects are useful
foundations, but the purpose includes discovering and demonstrating stronger
ways to express and check embedded applications.

This is a statement about language-level expression and guarantees. It does not
claim that equivalent machine behavior could never be implemented using C,
external generators, or additional analysis tools.

### Candidate next research question

The following is a discussion proposal, not an agreed feature set:

> How much of an embedded application's hardware configuration, resource
> allocation, interrupt binding, and initialization can be derived and checked
> from a composable C++ declaration of intent?

Potential experiments include:

- Extending conflict detection toward selecting valid allocations from declared
  requirements, building on the existing timer-selection machinery.
- Integrating interrupts into the same module and resource model, using generated
  platform bindings where appropriate.
- Expressing shared-resource compatibility and initialization dependencies so
  that more invalid combinations can be rejected before execution.
- Producing useful explanations of allocation failures and of the selected
  hardware configuration.

Each experiment should identify the property it establishes and its boundary:
which facts are known at compile time, which depend on declarations supplied by
adapters, and which still require runtime checks or hardware measurements.
Generated code is an acceptable avenue to investigate; it does not diminish the
goal of a generic declarative C++ interface.

Experiments can inform the eventual compiler baseline. Arduino compatibility
and a seamless AVR/ESP32 application API guide the work from the outset. Classic
AVR remains the first complete peripheral milestone, with initial ESP32 MCU work
as needed to validate the shared API before broader ESP32 support. The first
ESP32 architecture (Xtensa or RISC-V) and stock-toolchain support remain explicit
open decisions. Neither universal backward compatibility nor a newest-compiler
requirement has been chosen.

## Potential goal: independently maintained components in one workspace

The main reason for the multi-repository direction is to make small subsets
independently usable with only their actual dependencies. Delivery and consumer
validation now have a [standalone packaging plan](ArdoPackagingPlan.md), separate
from peripheral API development and repository/history migration.

The existing monorepo contains several Arduino libraries plus supporting tools.
The revamp could give each cohesive library or tool its own repository, while
the workspace coordinates compatible revisions and cross-project development.

Here, a repository component is distinct from an individual C++ `ModuleBase`
specialization. This idea does not require one repository per application module
class, header, or hardware device.

Initial extraction candidates, based on the current directory boundaries:

| Existing component | Potential independent project |
| --- | --- |
| `ardOinus` | Core framework, initially retaining its current internal layers |
| `ardOQuadEncoder` | Quadrature encoder library |
| `ardOStepper` | Stepper control library |
| `ardOFastLED` | FastLED integration library |
| `ardOnet` | Packet fragmentation/reassembly library |
| `ardoExtract` and the supplied `avr_api_gen.py` | Hardware-definition generation tooling; their relationship needs examination before deciding ownership |

These are candidates rather than agreed new repository names. The supplied
generator must be preserved; it has not been established that the two generator
sources can simply replace one another.

Additional possible boundaries inside the current core are:

- Embedded utilities and missing-standard-library support (`setl`/`setlx`).
- Application composition, dependency closure, and resource claims.
- Declarative register/field abstractions and hardware-specific definitions.
- Arduino/platform integration and host testing support.

Internal layers need not all become repositories immediately. Extraction should
follow cohesive ownership and a workable dependency graph. Comparisons with
existing libraries and generators may change what we choose to maintain.

## Constraints and behavior to preserve

- **Arduino compatibility:** published libraries must be usable with their
  declared dependencies without requiring GWZ. The workspace is the contributor
  environment, not a new prerequisite for sketch users.
- **Seamless AVR/ESP32 API:** reusable modules should express shared peripheral
  behavior through the same declarations and operations on both families. Target
  backends supply the hardware allocation, configuration, and binding details.
- **Target order:** classic ATmega328P-based Arduino Uno, Nano, and related AVR
  boards are the first complete peripheral milestone. Initial ESP32 MCU work may
  happen during that development to validate the shared API, followed by broader
  ESP32 coverage. Xtensa versus RISC-V remains undecided. Concrete board variants,
  compiler versions, and core versions still need selection.
- **Declarative hardware API:** use namespaced constants, types, field accessors,
  and template-managed transformations rather than exposing a macro interface.
- **Whole-application compilation:** preserve visibility of the complete module
  graph, resource bindings, and implementations so the compiler can specialize
  and optimize the final firmware across library/repository boundaries.
- **Static state and reusable instances:** preserve predictable allocation and
  resource-bound module specializations, with dependency sharing and instance
  identity made explicit; see [the principles](ArdoPrinciples.md).
- **Compile-time resource collision detection:** preserve the existing ability
  to reject incompatible declared resource assignments before an application
  can run. This is an existing capability, not a proposed future invention.
- **Compatibility-layer intent:** evaluate language support and standard-header
  availability separately; do not discard `setlx` merely because newer desktop
  compilers provide equivalent names.
- Preserve the distinction between regenerable hardware facts and manually
  maintained device semantics in `_dev.h` files.

Potential workspace responsibilities include shared documentation, coordinated
member revisions, integration sketches, and target build checks. Each member
would own its own sources, public interface, and relevant tests/examples.

## Resource checking is part of the revamp's identity

The current implementation combines module dependency closure with resource
claims. Its declared conflict rules include exclusive resources, intersecting
ranges, and shared use of the same resource identity with incompatible
configuration types. Identically configured shared claims are permitted.

Useful behavior examples to preserve and verify after extraction:

| Declaration | Intended result under the corresponding claim model |
| --- | --- |
| Two distinct modules each exclusively claim GPIO 7 | Compile-time error |
| Two storage claims cover overlapping ranges of the same resource type | Compile-time error |
| Two clients share the same resource/ID with the same configuration type | Allowed |
| Two clients share the same resource/ID with different configuration types | Compile-time error |
| A conflict arrives through a dependent module | Detected when the application includes that dependency closure |

These examples describe the existing design and future preservation criteria;
they are not newly executed tests. Checks depend on accurate declarations of
resource use, including adapters for Arduino services and third-party libraries.

Other systems perform related static checks. The important comparison is the
scope and expressiveness of those checks, not a blanket claim that Ardoinus is
the only implementation. See the focused comparison in the
[ecosystem review](ArdoEcosystemReviewSep26.md#focused-follow-up-compile-time-resource-collisions).

## Goal: seamless AVR/ESP32 API with AVR as the first complete target

Gianni extends the timer-allocation goal to **all AVR peripherals**, explicitly
including I2C and UARTs. Modules should declare their peripheral requirements,
and application composition should determine compatible hardware allocations
at compile time using the same overall resource model as timers.

Gianni clarifies that the application-facing Ardoinus API should be seamless
between AVR and ESP32. Modules using capabilities supported on both targets
should retain their declarations and peripheral operations when changing target;
MCU-specific allocation and register or SDK details belong in the backends.
Initial ESP32 MCU work may therefore be needed while developing the AVR support
to establish that the abstractions work across both architectures.

The first complete milestone is one coherent story for classic ATmega328P-based
Arduino Uno, Nano, and related AVR boards. The peripheral inventory should cover
timers, I2C/TWI, UART/USART, SPI, GPIO, ADC, and the remaining on-chip peripherals,
together with their relevant pins, clocks, interrupts, and resource relationships.
Establish that inventory as the completeness checklist; this is a whole-target
goal, not a claim that every peripheral API already exists.

Each peripheral retains the operations and capability requirements appropriate
to it, while participating in common module composition, allocation, and conflict
checking. Selection and validation still matter when a target has only one
suitable hardware instance: its ownership, configuration, pins, and permitted
sharing must satisfy the application's combined requirements.

The model must account for dependencies and conflicts across peripheral kinds,
including multiplexed pins and shared interrupt or clock resources where present.
Bus sharing must have explicit compatibility rules, and Arduino core services
or declared third-party use must participate in the reservation model. Allocation
should connect to declarative configuration and interrupt binding so the result
is usable by an application without manually completing those relationships.

### Agreed direction and sequencing

1. Develop the shared API and resource model with AVR and ESP32 requirements
   in view. Use focused initial ESP32 backend work where needed to validate
   representative module declarations and operations before the API is settled.
2. Complete and validate the AVR peripheral model on the classic Uno/Nano target
   family, with examples combining several peripheral kinds and compile-time
   diagnostics for incompatible declarations.
3. Expand ESP32 peripheral coverage using the shared API. **Xtensa or RISC-V
   remains to be decided**, along with the concrete device and board; that choice
   is needed for the initial ESP32 experiments, not only after AVR completion.

Portability validation should include the same reusable module source exercised
on both targets, with target and board configuration supplied separately. Early
ESP32 results should feed back into the API during AVR development. The detailed
APIs, allocation mechanism, and implementation phases have not yet been selected.

## Remaining timer challenge: compile-time allocation across MCUs

Gianni identifies the remaining timer API challenge as **letting a module declare
that it needs a timer resource, then determining the concrete allocation at
compile time across different MCUs**. Record this as a goal for the revamp; the
requirement vocabulary, allocation algorithm, and public API remain undecided.

The AVR ATmega328P development header already demonstrates deriving timer mode,
prescaler, TOP, and register settings from a declaration, with runtime frequency
and duty-cycle changes where supported. The next challenge is choosing which
hardware resource will satisfy that declaration in the complete application.
Existing collision detection provides a foundation, but detecting conflicting
assignments does not by itself solve allocation.

### Intended behavior

- A reusable module declares the timer behavior and capabilities it needs without
  having to name a concrete timer on each MCU.
- The target supplies a description of available resources, their capabilities,
  and their constraints. AVR is the first complete implementation; initial
  work on the selected ESP32 architecture (Xtensa or RISC-V, still to be decided)
  may be needed during AVR development to validate the shared timer API.
- Application composition considers requirements across the complete module
  dependency closure and determines a compatible assignment at compile time.
- The resulting assignment binds each module to the appropriate target timer
  implementation and its declarative configuration machinery.
- If no valid assignment exists, compilation explains the unsatisfied requirement
  or conflicting claims. Portability does not imply that every MCU can satisfy
  every declaration.

### Why this is a hard problem

Timer resources are heterogeneous. Requirements may involve frequency and its
acceptable error, resolution, waveform behavior, compare channels, capture,
interrupts, or particular output pins. A portable description must express the
required behavior while allowing each backend to represent its actual hardware;
identical timer numbers or register layouts are not a portability model.

Allocation must account for relationships between resources. Channels may share
a counter or clock configuration, output choices may constrain pins, and timer
selection may determine interrupt needs. Arduino services, explicit application
assignments, and declared third-party ownership can reserve resources. Sharing
is valid only when the required behavior and configuration are compatible.

The application must be considered as a whole: giving a flexible module the only
timer capable of satisfying another module can prevent an otherwise feasible
assignment. Whether to use search, another compile-time strategy, or generated
support remains a design question, as do determinism and compiler resource cost.

Runtime reconfiguration also needs a contract. A module that changes frequency
or duty cycle must remain within the capabilities and sharing constraints of its
statically allocated resource. How modules declare those permitted changes is
open; static allocation does not require all operating parameters to be fixed.

### Evidence to seek when implementation is planned

Demonstrate allocation on classic AVR boards and use focused early ESP32
experiments as needed to validate the same module requirement declarations on
both targets, with concrete assignments derived at compile time. Include a
multi-module case where allocation must preserve a scarce capability, a declared
reservation, compatible and incompatible sharing, and a target that cannot
satisfy the requirements. Verify that failures are useful compile-time diagnostics
and that fixed setup uses precomputed configuration values. These are proposed
validation cases, not completed experiments or a chosen implementation design.

This goal crosses module composition, resource claims, timer backends, and the
interrupt work below, so it should inform the eventual component boundaries.

## Major missing capability: declarative interrupts

Gianni identifies **interrupt support as the major gap in Ardoinus**. He has
struggled to implement it through a pure C++ compiler approach and suspects that
generating some C++ may be necessary to make the application-facing mechanism
fully generic. This is a working hypothesis to investigate, not a settled
conclusion that all compiler-based approaches are impossible.

The initial source inspection found interrupt-related register definitions,
resource terminology, and buffer comments, but did not identify a general
declarative ISR registration/dispatch layer in the inspected framework sources.

The intended direction is for modules to declare interrupt needs and handlers,
with platform bindings supplied behind that interface. The application should
not need to manually reproduce vector names, trampoline functions, or routing
boilerplate for each target.

### Why the platform boundary matters

Standard C++ does not specify hardware vector registration or interrupt calling
conventions. Existing AVR integration uses predetermined vector symbols and
compiler-specific ISR handling. A generic C++ handler still needs a target
entry point with the correct binding and calling convention.
[AVR-LibC interrupt interface](https://avrdudes.github.io/avr-libc/avr-libc-user-manual/group__avr__interrupts.html).

There is a newer AVR option worth testing before declaring generation essential:
AVR-LibC documents `ISR_N` for GCC 15 and later, and GCC supports `signal(num)`
and `interrupt(num)` attributes. These bind an IRQ number without requiring the
handler's C++ identifier to be the vector symbol. Whether the desired template
instantiations work correctly with these attributes, and survive linking and
optimization, has not been tested. Stock Arduino AVR GCC 7.3 cannot be assumed
to provide this mechanism.
[GCC AVR attributes](https://gcc.gnu.org/onlinedocs/gcc/AVR-Attributes.html).

ESP32 offers another route: Arduino GPIO has `attachInterruptArg`, accepting a
handler and context pointer, and ESP-IDF has an interrupt allocation API.
These permit callback bridges for supported interrupt sources; they do not
establish one uniform binding mechanism for every source or priority level.
[Arduino ESP32 GPIO interrupts](https://docs.espressif.com/projects/arduino-esp32/en/latest/api/gpio.html),
[ESP-IDF interrupt allocation](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/system/intr_alloc.html).

### Generation as a potential implementation tool

One candidate is generated C++ entry-point/registration glue that forwards into
the generic module/claim machinery. The generated layer could contain the
target-specific symbols, attributes, or SDK registration calls, keeping those
details out of the public declaration API. Any unavoidable platform macros
would be confined to that boundary.

Two alternatives need comparison: reusable per-device bindings, and bindings
generated for the selected application. The latter introduces a significant
question: how does the generator obtain the instantiated module closure without
requiring a second manually maintained description of the same application?
Textually scanning arbitrary template declarations is not assumed sufficient.

Issues that the eventual design must address include:

- Interrupt source, vector/CPU line, peripheral, and pin claims are related but
  not interchangeable resources. Shared vectors need explicit dispatch rules.
- Compile-time rejection of incompatible declared owners or configurations
  should extend the existing claim system. Runtime SDK allocation failures
  remain possible where the framework allocates resources dynamically.
- Arduino core or third-party ownership must be accounted for; generated ISRs
  must not silently replace core handlers or create duplicate definitions.
- Handler lifetime, initialization-before-enable, acknowledgement, and shared
  state access need defined behavior.
- On ESP32, IRAM-safe execution concerns all reachable code and data, not merely
  the entry function's attribute; interrupt allocation also has core-affinity
  constraints. These requirements belong to the backend contract.
- Generated sources must fit the intended Arduino workflow. Requiring each
  sketch user to install and invoke a separate generator is not automatically
  compatible with the desired installation experience.

No interrupt API, generation strategy, compiler baseline, or repository boundary
is selected by these notes. No interrupt implementation or target experiment
has been performed. Interrupt integration should be considered before freezing
the component split because it crosses composition, claims, and platform support.

## Open idea: boot-selected applications in one image

Gianni asks about compiling multiple applications into one firmware image and
selecting the active application at boot. This is recorded as an exploration
candidate, not an implemented feature or a chosen API.

Each application would have its own checked dependency graph and resource
allocation. Mutually exclusive applications may reuse the same hardware;
persistent firmware services must be compatible with every selectable application.
The current Application type checks its own module closure, providing a foundation.
Initialization, interrupt dispatch, module identity, and storage lifetimes still
need design. All selectable code occupies flash, and inactive static module state
can consume RAM unless storage reuse is deliberately implemented.

Selecting once per boot was suggested as a simpler initial scope, but remains
undecided. The fuller discussion and its relationship to whole-application
optimization are in [the principles record](ArdoPrinciples.md#open-idea-multiple-applications-in-one-firmware-image).

## Decisions still needed before a phased implementation plan

- Confirm the first repository boundaries and names.
- Define module instance identity and shared dependency/state semantics, including
  otherwise identical specializations and application boundaries.
- Decide whether to pursue boot-selected applications, their activation lifetime,
  interrupt routing, and RAM storage policy.
- Select a build approach that preserves whole-application optimization visibility
  and define how generated code size and memory use will be evaluated.
- Decide whether stock Arduino AVR compiler compatibility is required, or a
  newer AVR compiler is an acceptable prerequisite.
- Decide which utilities and hardware data pipelines remain local and which,
  if any, should reuse other projects.
- Define public-header and sketch compatibility expectations during extraction.
- Establish a working baseline so existing failures are distinguishable from
  extraction regressions.
- Establish the full peripheral inventory and completion criteria for the
  classic AVR Uno/Nano target family.
- Define portable peripheral requirements and the compile-time allocation
  contract, including timers, sharing, reservations, runtime reconfiguration,
  and diagnostics.
- Select Xtensa or RISC-V, and a concrete device/board, for initial ESP32
  validation during AVR development; identify the small backend experiments
  needed to establish a seamless API before expanding ESP32 coverage.
- Resolve the interrupt binding approach, including whether generation is
  per-device, per-application, or unnecessary on particular supported backends.
- Decide how repository history and independently released component versions
  should be handled. No Git history changes or release operations are implied
  by this ideas document.

When implementation planning is requested, turn agreed ideas into foundational
milestones with small, independently workable goals, aiming for less than
500 changed lines per step where practical.

## Related records

- [Standalone packaging plan](ArdoPackagingPlan.md)
- [Consolidated guiding principles](ArdoPrinciples.md)
- [State of play and Gianni's direction](ArdoStateOfPlayAug26.md)
- [Ecosystem catch-up and comparisons](ArdoEcosystemReviewSep26.md)
- [AVR API generator supplied by Gianni](avr_api_gen.py)
