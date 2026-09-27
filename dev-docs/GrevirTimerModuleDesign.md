# Module-owned timer configuration — Phase 1 design

Status: **design proposal for review**. This document gives a concrete answer
to Phase 1 of the [timer integration remediation plan](GrevirTimerIntegrationRemediationPlan.md).
It is not an implementation claim or a freeze of the illustrative C++ names.
The current fixed-PWM allocator, AVR `TimerConfiguration`, and narrow interrupt
examples do not yet implement this contract. No silicon validation is implied.

The settled constraints are: allocation is deterministic under declaration
reordering; common intent and resident-target options coexist while nonresident
options are inert; there is no backward-compatibility requirement for the
portable timer API; and one actual module instance owns each physical timer.
All timer uses belong to that module; another module cannot claim one of its
channels, events or other timer endpoints. This design chooses an
**immutable selected configuration with a typed
backend payload**. Target-specific configurators construct candidate payloads;
one common plan contract connects them to allocation, lifecycle and interrupt
binding. The [declarative do's](GrevirDeclarativeIntegrationDos.md) and
[don't-dos](GrevirDeclarativeIntegrationDontDos.md) govern those boundaries.

## 1. Authoring boundary: one module, one entire timer

A timer-owning module instance declares one hardware-timer need, all the ways
**that module itself** will use the timer, and any target constraints. The
application or board binds logical pins and supplies clock and reservation
facts. The allocator assigns the entire physical timer to that module. It does
not allocate individual channels, events or other timer endpoints to another
module. A timer-owning module has one selected physical timer in this contract;
a feature needing two independent timers composes two timer-owning modules.
The owner is responsible for configuring, activating, updating and
cleaning up its selected timer. Other modules may depend on the owner as an
application service, but they cannot make separate hardware claims on its
timer through that dependency.

This sketch shows relationships, not accepted spelling or compilable code:

```cpp
struct Motor {
  using Timer = timer::Own<
      timer::PwmUse<"drive", /* logical output + duty guarantee */>,
      timer::EventUse<"period", /* period + event semantics */>,
      timer::Common</* common timing constraints */>,
      timer::For<target::Avr, timer::CounterBitsAtLeast<16>>,
      timer::For<target::Atmega328P,
                 timer::RequireTimer<device::avr::atmega328p::Timer1>>,
      timer::For<target::Esp32, /* ESP32 constraints */>>;
};
```

`Own` is the module's **capability demand**: its PWM use, period event use and
all active constraints must be satisfied by one selected configuration. The
two named uses are internal to `Motor`; they are not resources offered for
other modules to claim. The period event may use the same counter as PWM only
if the selected mode can supply both. The owner configures the timer once. The
closure resolves the module instance, not just its C++ type: two `Motor`
instances require distinct stable instance keys and different physical timers.
Each internal use has a stable key under its owner; duplicate keys, duplicate
physical channels and duplicate pins fail.
The exact declaration syntax must fit Grevir's module and dependency machinery
without asking the author to repeat resource claims, setup calls, event
registrations or selected hardware bindings.
An application-level `on_interrupt` or `on_event` specialization for the
module's declared event may invoke application behavior. It creates an event
demand, not another timer owner or a right to configure the timer; the event
source remains bound to the owning module's selected configuration.

`CounterBitsAtLeast<16>` illustrates an explicit hardware-capability
constraint, distinct from a functional period or duty-resolution guarantee.
`RequireTimer<device::avr::atmega328p::Timer1>` pins this module to that physical
timer **only** when ATmega328P is resident. Both the AVR-family and device
sections apply there and intersect; neither overrides the other. On ESP32,
both AVR constraints are inert. The proposed public `device::...::Timer1`
identity must resolve to the authoritative typed ATmega328P timer definition;
the current backend calls that definition
`ardo::sys::avr::arch_atmega328p::Timer1Def`. Its exact public spelling is not
settled, and a timer with the same short name on another device is a different
resource. A device-specific timer identity used in the wrong active target
section is an input error, not a request to find a similarly named timer.

The exact-timer constraint filters candidates to that timer and is mandatory.
If Timer1 is reserved or cannot satisfy every internal use, selection fails instead
of silently choosing Timer0 or another timer. If the device section instead
pins `Timer0` while the AVR section requires at least 16 counter bits, the
active requirements contradict the typed ATmega328P facts: Timer0's counter
field is 8 bits and Timer1's is 16 bits. That is a **compile-time error** with
the module key, target, selected timer and conflicting requirement in the
diagnostic. Width comes from the typed counter field/capability metadata, not
a handwritten table, register macro or the storage type's name. A caller that
cares only about a realizable interval or duty step should express that
functional guarantee; the backend can then choose any timer that meets it.
A 16-bit physical counter may still be selected into a mode with a smaller
effective TOP or duty range; the width constraint alone does not promise
16-bit effective PWM resolution.

An internal use is a **requirement**, not an independent timer configuration.
Possible uses include PWM output, periodic or one-shot event, free-running
count, compare event, capture, and external pulse count. The backend may
determine that several individually possible uses cannot coexist in one mode.
Two modules never share one physical timer merely because their desired
frequencies happen to agree. The old `SameTimer` grouping and the later
`OfferRef` example for cross-module timer endpoints are both superseded. A
module that internally needs two PWM outputs may declare both, but it must own
the whole timer and expose any application-level behavior through its own
methods, not through separate timer-resource claims by other modules.

## 2. One complete candidate per possible owner configuration

The resident backend derives finite, complete candidates for each timer-owning module
from active intent and authoritative target facts. Candidate construction is
mode-specific and may use different calculators for PWM, counter, compare,
capture or alarm behavior. It must consider every internal use together, including
interrupt demand detected from the module's event catalog. It cannot add a
period interrupt by configuring the timer a second time after allocation.
An available event without a handler creates no interrupt-entry demand; a
detected handler makes the event's source and delivery constraints mandatory.

Each candidate has four connected parts:

| Part | Required content |
| --- | --- |
| Identity and provenance | Stable owner, physical block, configuration and endpoint keys; applicable target, source of clock/device/board facts and documented preference rank. |
| Guarantees and uses | Realized frequency or period and error, duty capability, event and startup semantics, update envelope, plus the exact internal functions and events actually available. |
| Footprint | Whole-timer owner; occupied channels, pins, registers or roles, interrupt sources and entries; shared configurable domains with exact settings; board reservations and containment relationships. |
| Typed backend payload | The complete mode, clock, TOP, routing, interrupt and startup settings needed by that backend's owner operations, or a checked immutable reference to those settings. |

The payload is not a configuration name that the board later translates into
settings. A name may be a diagnostic key, but changing a relevant setting must
change the selected payload and canonical plan identity. A C++ type-level tuple
or equivalent heterogeneous compile-time structure may hold target-specific
payloads; the common contract does not require one universal register layout,
runtime type erasure, virtual dispatch or firmware strings. Any JSON used by
the interrupt tool is a validated lowering record of this C++-derived plan,
not another selection authority.

Conceptually, the result is a list of
`SelectedOwner<instance-key, physical-block, backend-payload, uses, footprint>`
records plus selected shared-domain and interrupt bindings. `backend-payload`
is typed by the resident backend; the other fields have common inspection and
validation semantics. A selected owner record is immutable after allocation.
An owner-local use lookup returns its channel or event binding from that record,
not a fresh hardware search or a public claim for another module. This is a
conceptual type shape, not a proposed public
template signature.

For AVR, the present `TimerPwmConfigutation` can remain a PWM candidate
calculator if corrected where needed; other modes need their own calculators.
The current `TimerConfiguration` is PWM-shaped and must not be renamed into a
universal configurator merely by adding mode parameters. An AVR selected
payload can instead carry derived typed field values and an owner operation
that applies them through existing register and `setl::ApplierValues` machinery.
Timer definitions supply their actual compare channels and mode traits, so
generic allocation cannot assume exactly two channels or a Timer1-specific
setup path. An ATmega328PB or three-compare-channel AVR timer can reuse the
same contract with its own authoritative device description.

ESP32 timer-group alarms and LEDC PWM are distinct backend candidate families.
They may satisfy different common timing intents without pretending to share
an AVR register model or one low-level driver. The planned ESP32 backend may
program LEDC hardware without using the Arduino/IDF LEDC software API; this
design does not claim that driver is implemented. A module needing functions
from two unrelated hardware blocks requires explicit composition of two
owners, not a candidate that secretly owns both under one timer key.
The common selected-owner envelope can later be used for other peripheral
providers. A Pico PIO state-machine provider would own its own program and
resource footprint; it would not be represented as an AVR-like timer or made a
child of a PWM slice merely because it performs timed I/O.

## 3. Resource and capability model

One module owns its selected whole timer. Its internal uses receive
distinct endpoint assignments under that owner; the child assignments are
not additional whole-timer claims. Any external timer claim conflicts with
that owner, including a claim on an unused child channel. Internal duplicate
channels, pins or overlapping exclusive ranges fail. Pin aliases normalize to
one physical identity before candidate selection; an unresolvable identity is
a model error, never evidence of independence.

The backend describes coupled roles, not merely a flat list of channel names.
For example, if a candidate uses a register as TOP, that register cannot also
be used for independent capture or compare unless target facts establish
that the particular mode supports both. Likewise, a period event is available
only when its selected mode, source, pending policy and acknowledgement rules
can meet the declared semantics. Interrupt bindings are derived from these
uses and the same selected candidate. The existing IRQ generator may lower
an AVR vector or ESP32 registration from that binding; it does not choose the
timer or invent an event source.

A configurable domain shared by different timers, such as a divider, is a
separate resource from either whole timer. Each candidate states its exact
required domain setting. Compatible uses compose one selected domain setup
owner with a stable identity and one writer across setup, update and cleanup.
The domain owner is derived from backend domain facts and selected uses; it
does not grant the timer-owning modules independent write access. If a backend
cannot represent that owner and lifecycle yet, it rejects candidates needing
the domain. This shared-domain rule does not permit two modules to share a
physical timer. An immutable input clock is a board/device fact, not a mutable
domain to configure. Conflicting domain settings require another candidate or
allocation failure.

Capability metadata must say what subset of hardware behavior the backend
models. A missing candidate means unsupported in that model, not that the MCU
cannot physically do it. AVR port/bit and timer field identities come from
typed device facts, supplemented only by documented board wiring, clocks and
reservations. Other architectures need their own authoritative inventories;
they must not inherit invented AVR numeric IDs or AVR operating costs.
Nominal frequency and error can be evaluated as exact or bounded rational
metadata at build time. The public numeric contract does not require runtime
floating-point work on an AVR owner; runtime update operations need their own
range, rounding and target-cost analysis. Generic metadata need not be narrowed
to AVR word size when doing so would change its meaning on ESP32 or another MCU.

## 4. Selection and diagnostic contract

Selection proceeds at compile time from the application closure:

1. Resolve stable module-instance and internal-use keys. Reject duplicate identities
   before comparing their request contents. Detect the events demanded by
   visible `on_interrupt` or `on_event` handlers from the finite event catalog.
2. Select common and resident-target requirements. Nonresident option metadata
   remains parseable without its SDK but does not instantiate capability or
   driver code. Validate active units, references, target domains and aliases.
3. Ask the resident backend for complete candidates for each module's joint
   uses. Validate candidate keys, guarantees, footprint and provenance.
4. Sort owners and candidates by documented stable keys and preference policy.
   Search complete assignments with backtracking, checking whole ownership,
   internal/external claims, shared domains and interrupt-source uniqueness.
5. Return one canonical plan or a distinct error: invalid input/model,
   unsupported active capability, no suitable candidate, proven joint conflict
   or search exhaustion. Exhaustion never masquerades as impossibility.

The solver must find an assignment whenever one exists in the declared
supported candidate model and the search completes. A bounded compiler budget
may interrupt search but must report that separately. Reordering declarations,
backend inventory entries or claims cannot change the plan or structured
diagnostic. Stable identity changes or changed facts may legitimately change
the result. The plan exposes chosen and rejected alternatives, realized
guarantees, resource footprints, provenance and the reason for selection so
that application authors can inspect it without decoding generated source.

The no-request case has an empty plan and no timer setup. A one-owner case
has one selected owner operation. With several timer-owning modules, dispatch follows
selected owner identities and dependencies, never `configurations[0]` or a
board switch on Timer1. The module instance path, not an incidental array
position, connects each module's internal use to its selected binding.

## 5. Runtime boundary

Selection proves only modeled static facts. At runtime, each selected owner
applies its payload exactly once. The application orders shared-domain setup,
timer-provider setup, dependent-module setup, target interrupt installation,
pending-event handling and source enablement according to the validated plan.
Owners perform their own hardware effects and cleanup; the board supplies
facts and reservations, not timer register writes. A failed stage leaves owned
sources masked, records what was acquired and prevents dependent modules from
running. The detailed independent-service policy and reverse cleanup state
machine belong to Phase 3 and must agree with the existing interrupt start
contract before this lifecycle is implemented.

A runtime update belongs to the selected owner, never to another module or
board hook. Its selected payload states the allowed update envelope
and which internal uses' guarantees change together. Updating duty within a fixed
PWM configuration may be supported first. Dynamic frequency, mode changes or
reallocation are unsupported until the owner's contract defines joint effects,
concurrency and failure behavior; an initial frequency alone does not grant
that capability. This design makes no glitch-free or physical-timing guarantee.

## 6. Worked design counterexamples

| Case | Required result and reason |
| --- | --- |
| One AVR module uses PWM and a period event | Legal only if one selected mode, clock, TOP and source supplies both; one timer setup and one IRQ source owner. The event demand constrains candidate selection. |
| PWM uses the input-capture register as TOP while another internal use requires capture | Reject that candidate because the register roles overlap; select another legal candidate if one exists. |
| One module uses channels A and B, or a later device has A/B/C | Enumerate actual channels from device facts. Internal uses can coexist only if one configuration satisfies all guarantees and each assigned channel/pin is unique. |
| Two `MotorModule` instances each declare a timer need | Assign different timers or fail. One cannot claim an endpoint of the other's timer. A different single module may own a timer and internally use two channels; that is a different module boundary. |
| ATmega328P module requires at least 16 counter bits and pins Timer1 | Restrict to the authoritative Timer1 candidate; succeed only if all internal uses and claims fit. The corresponding Timer0 pin is a compile-time contradiction, not permission to relax the width or choose another timer. |
| A flexible module can use T0/T1; a constrained module needs T0 | Backtrack to flexible→T1 and constrained→T0, independent of declaration order. For three owners, compare solver output with an exhaustive synthetic oracle; a stopped search reports exhaustion. |
| Two timers require shared divider D=8 | Compose one domain writer or reject until supported. D=8 versus D=64 needs a different candidate or fails; neither timer writes D independently. |
| A PWM pin route and a board GPIO alias name the same pin | Normalize to the same device identity and reject the conflict before selection, regardless of declaration order. |
| AVR declaration includes common and ESP32 options | On AVR, ESP32 options are inert and require no ESP32 SDK. On ESP32, the converse holds; active unsupported options fail. |
| Classic ESP32 alarm and LEDC PWM | They are separate candidate families and physical owners. Common period/PWM intent does not imply one hardware block or driver. |
| RP2040 PWM slice measurement and PIO timed I/O | Design stress only: represent the selected program and footprints without assuming all timed functions are timer channels. No RP2040 backend or conflict table is asserted here. |
| Empty application and partial startup failure | Empty plan has no timer effects; failed provider/registration cannot leave a dependent consumer running. |

## 7. First implementation slice and review gate

The recommended first slice is deliberately narrow: the synthetic mock proves
the solver and module-ownership contract; ATmega328P Timer1 proves a compatible
PWM-plus-period-event configuration and a conflicting role; classic ESP32
timer-group alarm exercises the same portable plan envelope through a different
backend family. Fixed-PWM support for ATmega328P Timer0/1/2 remains relevant,
but no claim of every AVR timer mode or every ESP32 LEDC variant is needed.
RP2040 remains a design stress target until a dedicated capability study.

Before Phase 2, review this contract with worked typed plan examples for zero,
one and several timer-owning modules. Show at least one complete candidate and its applied
effect inventory for each supported family; independently derive timing and
capability expectations, including TOP boundaries. Check the ICR conflict,
pin alias, shared-domain, backtracking and inactive-SDK counterexamples above.
Then choose exact public declaration syntax and the first supported operations.
Document the capability source and unsupported modes for each backend. Phase 2
can implement only the reviewed slice; it cannot pass by silently omitting an
active internal use or by substituting target builds for these architecture checks.

Open syntax and policy details are: the exact module timer-use C++
spelling; precise frequency, duty and event guarantee types; configuration
preference order; how the selected typed payload is represented and serialized
without duplicate authority; and whether the first slice admits any mutable
shared domain. These decisions must preserve the boundaries above and be made
with concrete plan examples, not inferred from the old PWM-shaped API.
