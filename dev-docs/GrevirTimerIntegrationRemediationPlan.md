# Timer and interrupt integration remediation plan

Status: proposed work plan. This records the architectural review of the
uncommitted timer/interrupt integration; it does not claim that the design
questions below are settled or that the implementation is complete.

Grevir's aim is to derive MCU integration from application intent and
authoritative target facts at build time. The current fixed-PWM path and narrow
interrupt examples demonstrate parts of that approach, but the selected
interrupt configuration does not determine every applied timer setting. Board
hooks still configure named peripherals, and module startup does not yet have a
safe failure policy. A typed register program or successful target build alone
does not close those gaps. See the [declarative do's](GrevirDeclarativeIntegrationDos.md),
[don't-dos](GrevirDeclarativeIntegrationDontDos.md),
[timer allocation design](GrevirTimerAllocationDesign.md), and
[interrupt binding architecture](GrevirInterruptBindingArchitecture.md).

The phases are ordered by dependency. Later implementation must not use the
current one-request examples as the general contract. The portable timer API
has no users requiring backward compatibility; changing it is acceptable when
the resulting model is clearer and more capable.

**Current ownership decision:** allocation is strict. One module instance owns
each selected physical timer and its complete configuration. The allocator may
choose among legal timers for that module, but it does not merge requirements
from independently owning modules onto one timer, even when their frequencies
or modes appear compatible. An application that wants several consumers to use
one timer explicitly composes them around a provider module. That provider
declares and manages the offered functions and internal endpoints; dependent
consumers bind to those offers without claiming or configuring the physical
timer again. Owner identity and offers must be derived from the actual module
closure and validated against the selected configuration. Internal duplicate
channels/pins and external ownership conflicts remain errors. Automatic
cross-module sharing is deferred, not an implicit fallback on allocation
failure. This also supersedes the earlier explicit `SameTimer` grouping of
requests from separate owner modules: an actual provider module in the
application closure owns the timer and offers endpoints to its dependents.
This is the contract for the redesign, not a claim that the installed fixed-PWM
allocator already enforces it.

## Phase 1 — decide the timer capability abstraction

This is a design and discussion phase, not an instruction to add every hardware
timer feature now. Timers can provide PWM, periodic or one-shot alarms,
free-running counts, compare events, input capture, external pulse counting and
other target-specific functions. These are not necessarily independent: mode,
clock source, divider, TOP source, compare registers, channels, routing and
interrupt sources may be shared or mutually exclusive. AVR Timer0/1/2 differ,
and classic ESP32 timer groups, LEDC hardware and other timer-capable blocks need
not share one low-level API. The abstraction must admit those differences without
reducing the common API to a list of AVR modes or pretending all functions are
available on every target.

The design target matrix is deliberately wider than the implementation matrix:

| Design target | Role in this phase | Capability question it must expose |
| --- | --- | --- |
| ATmega328P (Timer0/1/2) | Existing typed timer and fixed-PWM path; narrow interrupt example | Can one selected configuration account for PWM, count/compare/capture functions and their shared mode, TOP, clock and register use? |
| Classic ESP32 | Existing timer-group alarm example; distinct LEDC and other timer-capable blocks remain design inputs | Can common timing intent be fulfilled by different peripheral families without pretending they share a low-level driver or resource graph? |
| Raspberry Pi Pico (RP2040) | **Design stress target only; no Grevir Pico backend or validation is claimed** | Can one model express an RP2040 PWM slice used for output *or* input measurement, programmable PIO state machines offering different functions, and timer alarms, including their shared resources and routing? |
| Synthetic mock | Architecture counterexamples, independent of a physical chip | Does the solver reject coupled conflicts and find legal alternatives without encoding any one MCU's inventory? |

The Pico question is especially useful because a fixed-function name is not a
complete statement of what a resource can do. Raspberry Pi documents that an
RP2040 PWM slice can drive two outputs **or** measure input frequency/duty; its
PIO state machines execute programs for different I/O functions, and its timer
has four alarms. Phase 1 should test how a selected program/configuration offers
functions and consumes slice, state-machine, instruction-memory, pin, clock,
DMA and interrupt resources where applicable. The exact conflicts and sharing
rules must come from a dedicated RP2040 capability study, not assumptions copied
from AVR or ESP32. [Raspberry Pi hardware API documentation](https://www.raspberrypi.com/documentation/pico-sdk/hardware.html)

Resolve these questions with worked configurations and counterexamples before
changing the allocator:

1. **Intent and guarantees.** Which portable requests describe observable
   behavior (frequency or period, accuracy, duty capability, event semantics,
   capture resolution, startup state), and which choices are explicitly
   target-specific constraints? What are the units, acceptable approximations,
   update semantics and failure conditions? Which behaviors are static, and
   which are runtime operations of a selected owner?
2. **Configuration and offered functions.** One provider module owns a complete
   configuration of one hardware block and may offer several endpoints. Decide
   how that owner's requirements for PWM, counting and events combine into one
   legal configuration rather than selecting each function independently. Identify
   configuration-wide state, endpoint state, shared clock domains and resources
   consumed for TOP, duty, compare or capture.
3. **Capabilities and provenance.** Which facts come from extracted device
   definitions, target manuals or SDK capability metadata, and which come from
   board wiring, clocks and reservations? Define canonical identities and alias
   normalization. Do not create a second handwritten inventory merely to fit
   the current `Candidate` shape. A backend must state the subset of hardware
   behavior it models; absence from that subset is not proof of hardware
   impossibility.
4. **Selection and plan representation.** Decide whether the portable plan
   carries typed configuration values, a checked reference to an immutable
   backend configuration, or another representation. In every case, the chosen
   plan must determine the relevant applied settings and resource footprint.
   A name such as `period_1ms_div80` must not be checked while divider and alarm
   counts are supplied independently elsewhere. The plan must remain
   deterministic under declaration reordering and inspectable with reasons for
   selection or failure. Specify a complete search over the supported candidate
   model: a feasible assignment must be found, an exhaustive proof of no
   assignment must report conflict, and a bounded search that stops early must
   report exhaustion separately. Fix the supported search envelope and its
   diagnostic contract before treating allocation failure as impossibility.
5. **Owner and lifecycle contract.** Define how the selected provider module
   configures its block once, activates its offered endpoints and interrupt
   sources, handles dynamic updates, and cleans up partial startup. Specify how
   dependent modules refer to and claim those offers without becoming second
   physical owners. Define dependency order and the boundary between
   compile-time proof and runtime failures. Decide how shared clock domains
   spanning different physical timers are owned without allowing either timer
   endpoint to reconfigure them silently.
6. **Target applicability.** Keep common intent active across MCUs; apply only
   the resident target's specific constraints. Nonresident declarations must
   remain inert without pulling in that target's SDK. A mock inventory must
   exercise the same portable semantics without masquerading as a physical MCU.

Use at least these design cases: (a) one provider module offering both a PWM
output and a compatible period event on the same AVR timer; (b) a PWM
configuration using ICR as TOP alongside
an input-capture request that also needs ICR, which must conflict; (c) two
owner modules with alternative timers where a greedy first choice fails but a joint
solution exists; (d) a classic ESP32 alarm and a PWM requirement, showing where
common behavior ends and distinct hardware blocks begin; (e) a configuration
that changes at runtime, to expose ownership and update guarantees; and (f) a
Pico PIO program offering a timed I/O function alongside PWM-slice input
measurement, to test whether the model handles programmable and multifunction
resources without treating them as interchangeable timer channels. Include a
zero-request case and a two-owner case so the API cannot depend on
`configurations[0]`. Add a three-owner synthetic case in which a first legal
choice blocks a later constrained owner but backtracking finds a solution, and
two owners on different timers requiring one configurable shared clock domain.
For that domain, decide which selected owner performs setup, update and cleanup,
or reject it until such ownership can be represented.

**Exit gate:** a reviewed contract and worked API/plan examples define the
supported first slice, the extension points for other timer functions, the
source of each capability fact, and the static/runtime boundary. An independent
synthetic counterexample should be able to say whether two offered functions
can coexist. The contract identifies canonical resource identities and alias
normalization required by the first implementation slice, a single-writer rule
for every configurable shared domain it admits, and the completeness, conflict
and search-exhaustion outcomes. Check the abstraction against the Pico case before freezing it;
RP2040 implementation is not required to complete this gate. Do not claim the
backend models every AVR, ESP32 or RP2040 timer-capable mode.

## Phase 2 — make selection determine provider configuration

Implement the agreed representation in the portable candidate, solver and
selected-plan path. Generate complete candidates for each declared owner module,
including its internal offered endpoints; select one physical timer per owner and
reject collisions between owners. The allocator must not silently co-locate two
owner modules. Keep the initial vertical slice small: mock, ATmega328P
Timer1 period/PWM coexistence, and the existing classic ESP32 timer-group alarm.
The exact slice may change after Phase 1. Derive every selected owner's setup
input from one validated plan; remove separate board mappings from configuration
names to register settings. Replace `solution.configurations[0]` and
whole-application peripheral switches with dispatch over selected owner
identities. The owner uses its target's established programming facilities;
for AVR that includes typed metadata and `setl::ApplierValues` where applicable.
Before a candidate enters selection, normalize each physical resource in the
supported slice against device facts and board aliases, including explicit
claims; reject a candidate whose identity or routing cannot be established.
For any configurable domain shared by selected timers, derive exactly one
domain setup owner with ordered acquisition, update and cleanup, or reject the
candidate until that ownership can be represented. An individual timer owner
must not become a second writer of the domain.

**Exit gate:** for the supported slice, independently calculate expected
realized timing and offered capability from authoritative target facts, then
compare the selected plan with the applied register/program effects, including
TOP boundaries, interrupt source and pin routing. An incompatible mode/source
combination fails. Changing a selected mode, clock, period or source changes
the owner's applied program through the plan, or fails at compile time. A
board alias and device pin for the same physical GPIO conflict with an
explicit claim in either declaration order. Shared-domain cases prove one
writer through setup and cleanup, or fail explicitly while unsupported;
agreeing and conflicting divider requirements must not pass as independent
writes. Zero, one, two and a backtracking three-owner synthetic case are
covered. An independent small feasibility oracle agrees with selection:
feasible assignments are found, fully searched infeasible assignments report
conflict, and a forced search limit reports exhaustion. The same input produces
the same plan regardless of declaration order. Compile shared common/AVR/ESP32
declarations for each resident target without the nonresident SDK; changing
only an inactive section leaves the active plan and diagnostics unchanged.
Source checks inspect disabled platform branches. AVR, ESP32 and mock each
exercise the common contract only for capabilities they actually implement.

## Phase 3 — close startup, failure and dependency semantics

Derive startup order from selected owners and module dependencies. Define which
modules may initialize before interrupt entry installation and which require an
active provider. Track acquired resources so a registration or pending-policy
failure leaves the application in a defined state; make cleanup the
responsibility of the owners that acquired them. Specify whether independent
services can continue after a timer failure, and prevent dependent modules from
running after failed or partial setup. `Application::runLoop()` must enforce the
chosen policy rather than relying on every sketch to interpret a setup result.

**Exit gate:** forced failure at each fallible stage has a defined module state,
source-mask state and cleanup result. A timer-dependent module never runs after
its provider fails; an independent service behaves according to the documented
policy. Repeated startup follows the existing interrupt start-state contract.

## Phase 4 — complete authoritative GPIO and resource identities

Phase 2 already requires canonical identity and claim interoperability for
every resource in its supported slice. Complete that migration for the broader
GPIO and peripheral inventory: replace any remaining `pad_b1 = 101`-style
identities and parallel reservation mapping with canonical identities derived
from typed device facts and board bindings. Normalize aliases before conflict
checks. Keep whole-timer, subresource, shared-domain and pin-routing
relationships explicit; do not infer independence because two resources have
different names. The portable claim API may need to change to express these
identities, since it has no compatibility obligation.

**Exit gate:** a board alias and device pin resolve to the same resource; PWM
allocation and explicit GPIO claims detect the same conflict; repeated and
overlapping claims fail without invented numeric pin tables.

## Phase 5 — restrict generation to target lowering

Keep `grevir-irqgen` as the toolchain bridge for constructs such as AVR
`ISR(...)`, using the canonical, validated plan and final-build agreement
checks. Move fixed AVR interrupt-state implementation out of Python-emitted
strings and into the AVR backend. Make additions to the curated device register
inventory reproducible from their source; the current `rrSREG` address matches
the Ardoinus extraction but was inserted manually into a generated header.
The generator must not choose timer parameters, infer new owners or maintain a
parallel configuration table.

**Exit gate:** each emitted entry has one selected source/owner in the plan;
stale or divergent generated output is rejected; fixed backend operations are
owned and tested with the backend. The generated file is still write-and-forget:
the tool does not parse it back as an authority.

## Validation and review boundary

Review the Phase 1 contract before using target builds as evidence for the
architecture. Later checks should distinguish synthetic allocator proofs,
host mock lifecycle behavior, source checks of inactive platform branches,
AVR and ESP32 target compile/link, simulation, and physical hardware behavior.
Compilation proves that a particular program can be built; it does not prove
capability completeness, physical timing or owner modularity. Preserve the
existing hold on silicon validation. Apply the [cross-MCU review policy](review-policies/CrossMcu.md)
and the relevant [AVR](review-policies/Avr.md) or
[ESP32](review-policies/Esp32.md) supplement to each change.
