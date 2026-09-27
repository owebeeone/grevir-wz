# Declarative MCU integration: dos

## Purpose and status

Grevir should let an application state its intent once and derive its integration.
From module requirements, constraints, preferences and authoritative target facts,
C++ templates and constant evaluation should derive every integration decision
knowable at build time: dependency closure, resource allocation and ownership,
complete peripheral configuration, startup order and event bindings. Deriving the
assembly itself is the goal, beyond supplying defaults or expressing manual
wiring with templates.

The resulting plan is data that can be inspected and validated before effects.
Build-time generators lower that plan into required toolchain syntax. Selected
runtime owners apply it and handle inputs, failures and physical behavior that
static facts cannot settle. These are design goals, not a claim that current
Grevir implements every rule. In particular, the existing interrupt examples'
board-owned setup remains an open architectural issue.

This is the positive companion to
[Declarative MCU integration: don't-dos](GrevirDeclarativeIntegrationDontDos.md).
It draws on the [timer allocation design](GrevirTimerAllocationDesign.md),
[interrupt binding architecture](GrevirInterruptBindingArchitecture.md), and the
external design instrument
[`DeclarativeApiEvaluatorV2.md`](../../bizscad/dev-docs/DeclarativeApiEvaluatorV2.md)
(internally **v3**, despite its filename). That instrument treats gap richness,
generativity and the correctness envelope as separate dimensions. These rules
express goals; they assign no scores or blanket guarantee that errors cannot ship.

Examples below illustrate desired behavior, not settled API spelling or an
implementation-status report. Apply each general rule across resources and
architectures; an AVR example does not prescribe another target's mechanism.

## 1. Do define meaning independently of the chosen implementation

Specify the observable contract of each requirement: units, range, accuracy,
sharing, ordering and failure behavior. Give configuration choices meanings that
can be checked independently of the allocator's output. Make approximation,
preferences and defaults explicit parts of that contract.

**Example:** a PWM requirement can demand a nominal frequency within an explicit
error bound and a maximum attainable duty step. A backend supplies a concrete
candidate whose clock, mode and duty behavior satisfy those requirements;
register width alone does not define the duty guarantee. Oscillator accuracy
and transition behavior require their own contracts.

**Review check:** write what would make a selected result wrong without referring
to what the current engine happens to select. Identify all implicit defaults.

## 2. Do derive the integration graph from intent

Use C++ templates and constant evaluation to construct dependencies, owners,
bindings and execution order from declared requirements and capabilities. Authors
should supply decisions that express intent or constrain choice; derive the
mechanical consequences. Keep the complete static integration path available
before hardware effects begin.

**Example:** in the interrupt design, a handler specialization for a catalog
event creates demand. The module closure supplies the finite catalog; allocation
derives a compatible peripheral configuration, source owner and entry binding.
The author does not also maintain a vector list or registration table.

**Review check:** enumerate what the author states and what the engine derives.
For every manually supplied dependency or binding, explain which intent it adds
that cannot already be derived.

## 3. Do keep hardware facts authoritative and identities canonical

Derive capabilities, routes, encodings and resource identities from maintained
device metadata, with board wiring and reservations added at their owning scope.
Normalize aliases before checking conflicts. Preserve relationships between
whole resources, subresources and shared configuration domains.

**Example:** an AVR GPIO identity derives from its typed port and bit definition
within the device scope. A board alias for that pin resolves to the same physical
identity, so a timer output and a manually reserved GPIO cannot evade a conflict
through different names.

**Review check:** trace each selected resource and encoded setting to a fact
source. Check alias collisions and parent/child conflicts as well as exact keys.

## 4. Do select complete, jointly legal configurations

Model the coupled behavior and full resource footprint of each candidate. Solve
all active requirements together, deriving ownership and endpoint bindings from
the same selection. Keep mandatory constraints distinct from preferences; state
the supported search envelope and distinguish resource exhaustion from a proven
absence of solutions.

**Example:** a timer used for PWM and an interrupt must offer both functions in
one legal mode. A configuration consuming the input-capture register as TOP
cannot independently offer that register for capture. Two equal-frequency
requests share only through a declared sharing contract and compatible complete
configuration.

**Review check:** combine individually valid requests that compete for hidden
shared state. Verify that the modeled solver can explore legal alternatives and
that its failures distinguish conflict, unsupported capability and exhaustion.

## 5. Do make composition deterministic and its changes explainable

Normalize inputs and use stable identities, explicit dependencies and documented
selection policy. Equivalent inputs must produce the same canonical plan,
independent of incidental declaration or enumeration order. Define intentional
ordering locally. Make semantic defaults and allocation-policy versions part of
the reproducible input.

**Example:** reordering independent motor and indicator modules leaves their
bindings and dependency order unchanged. Adding another consumer may change
allocation; a plan diff should explain the new constraint and reassignment.
An application needing a fixed physical binding can declare that constraint.

**Review check:** consider permutations, reusable module instances and composition
with a third consumer. Identify which input changes may legitimately alter the
plan and how an author can inspect that change.

## 6. Do make modeled errors fail before effects

Require static validation of active references, units, identities, dependency
cycles, resource conflicts and candidate compatibility. Structure APIs so invalid
combinations are difficult to express and remaining modeled errors produce
actionable diagnostics. Validate boundaries between discovery, selection,
serialization, lowering and execution as well as each stage in isolation.

**Example:** every detected interrupt demand must have exactly one selected event
binding, while a shared physical source has one owner and entry with defined
dispatch semantics. An unknown event, duplicate identity or conflicting vector
fails the enforced build gate with the relevant request and resource keys.

**Review check:** name the check and enforcement stage for each claimed error
class. Include valid cases that must remain expressible; rejecting legitimate
intent is a correctness cost, not a stronger guarantee.

## 7. Do expose the plan and the reasons behind it

Provide an inspectable canonical plan connecting each result to its requirement,
capability, constraint and policy. Explain both choices and failures using stable
application identities. Support meaningful plan comparison across intent, target
or policy changes without requiring an author to reverse-engineer generated code.

**Example:** a timer selection report identifies the requesting module, chosen
mode, realized frequency, source of the clock fact and the reservation excluding
an alternative. A changed reservation produces a diff of affected bindings and
their reasons.

**Review check:** answer “why this binding?” and “what caused this change?” from
the plan and its provenance. A selected name alone is insufficient explanation.

## 8. Do keep lowering faithful to one validated plan

Use generators only to translate validated C++-derived decisions into syntax or
artifacts required by the toolchain. Carry target, schema and relevant build
identity through that boundary. Enforce freshness and agreement with the final
firmware so stale or partially produced artifacts cannot become a second truth.

**Example:** the interrupt probe emits selected bindings; the generator validates
the record and emits AVR `ISR(...)` entries or a target's callback bridge. It
does not rediscover handlers or choose timers. The final build checks agreement
with the current application plan before accepting those artifacts.

**Review check:** trace each generated decision back to the plan. Consider a
changed handler, changed build definition and interrupted generation attempt.

## 9. Do let selected owners apply their configurations

Derive lifecycle dispatch for every selected owner from identities and dependency
edges. Owners interpret their selected configuration through their own facilities
and own setup, activation, shutdown and failure cleanup. Coordination orders
these operations. Extend a provider contract when it lacks a required operation.

**Example:** a selected AVR timer owner derives typed field values and uses its
configuration/applier machinery for setup. A board declares facts and
reservations; startup reaches every selected timer owner after its dependencies
are ready. Cleanup follows the resources actually acquired.

**Review check:** trace zero, one and many selected owners through configuration,
activation and partial failure. Every hardware effect must follow the selected
plan and belong to the resource owner performing it.

## 10. Do preserve portable intent with explicit target capabilities

Select common and resident-target requirements before capability validation.
Keep common semantics intact while backends supply distinct inventories and
implementations. Make nonresident sections inert without requiring their SDKs;
make unsupported active requirements explicit errors. Assess representation and
runtime costs for the named target and operation.

**Example:** a common PWM accuracy requirement applies alongside AVR options on
AVR and ESP32 options on the named ESP32 target. Inactive options remain
well-formed declarative metadata. An ESP32 GPIO/clock/serial adapter alone does
not establish timer or interrupt support, and AVR cost constraints do not narrow
the shared numeric contract.

**Review check:** distinguish “inactive” from “unsupported” and inspect both
target branches. Apply [Cross-MCU](review-policies/CrossMcu.md) plus
[AVR](review-policies/Avr.md) or [ESP32](review-policies/Esp32.md) as appropriate;
use a dedicated supplement for a new architecture.

## 11. Do minimize authoring burden while keeping intent visible

Let reusable modules compose as parts with declared interfaces and requirements.
Require information once at its owning boundary, derive repetitions and give
defaults documented semantics. Keep exceptional constraints local and imperative
escapes visible, with declared effects and resource claims contained by an owner.

**Example:** adding a second instance of a module requires its distinct identity
and application-specific constraints; dependency lists, registrations and setup
calls follow automatically. A custom device operation exposes a provider
contract and resource footprint rather than silently changing shared timer state.

**Review check:** compare the edit needed for an intent change with the wiring it
creates. Look for repeated declarations, surprising defaults, and escape effects
that bypass the plan. Check representative intents beyond the initial example.

## 12. Do state runtime obligations and evidence limits precisely

Resolve static decisions at build time; specify runtime behavior for dynamic
inputs, registration failures, concurrency and hardware state. Separate modeled
correctness from physical guarantees. Validate claims with evidence suited to the
boundary, including independent expectations and negative cases when evaluating
the implementation.

**Example:** an interrupt plan can determine the owner and selected dispatch
policy. The runtime owner still handles pending events, failed registration and
repeated startup according to its contract. Host mocks exercise those modeled
transitions; target compilation checks toolchain integration; neither establishes
silicon timing or electrical behavior.

**Review check:** report architectural inspection, source checks, host tests,
target compilation and hardware evidence separately, preserving validation
holds. Record unsupported cases and unverified claims. If using the evaluator,
score coherent surfaces and their seams with its required evidence; keep
unmeasured dimensions unscored rather than inferring success from design intent.
