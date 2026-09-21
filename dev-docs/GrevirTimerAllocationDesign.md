# Deterministic timer allocation — design draft

Status: proposal for discussion, 21 September 2026. A
[host-only prototype](../experiments/timer-allocation/README.md) exercises a bounded
subset; the full design and production integration are not implemented.
The user has required deterministic allocation independent of module declaration
order. The user also requires common requirements and configurations for different
MCU targets to coexist: common requirements and the resident target's options
apply; nonresident target options are ignored. There are no users of the portable
API and no backward-compatibility requirement. Its existing design is direction,
not a contract to preserve. Stable identities, the selection algorithm and other
choices below remain recommendations. This is a focused design effort alongside
the Ardoinus migration.

[Worked API examples](GrevirTimerApiExamples.md) exercise this proposal with
target selection, reusable modules, allocation outcomes, numeric guarantees and
sharing. They recommend concrete semantics without claiming user acceptance or
an implemented API.

## Purpose and first scope

Turn a complete application's static PWM requests into concrete timer/channel/pin
bindings, or a useful compile-time failure. Application modules state what they
need; MCU backends describe realizable choices; board policy supplies reservations.

The first implementation should support fixed-frequency PWM, explicit pin
requirements, optional explicit timer/channel constraints, and explicit groups
of outputs sharing one timer configuration. A backend may offer several legal
routes for a pin. Automatic choice among unrelated application pins is deferred.
Runtime allocation, interrupt/capture scheduling, a general peripheral optimizer,
and an ESP32 backend are outside this first scope. A synthetic non-AVR inventory
should still demonstrate that the solver does not depend on AVR register concepts.

Variable-frequency requests already exist as metadata, without constraining the
new API. Until their operational contract is designed, the first allocator must
reject active variable-frequency requirements explicitly. In
particular, an initial frequency alone says nothing about the required operating
range or which owner is allowed to change a shared timer.

## What exists today

- Peripherals supplies frequency/resolution request metadata, explicit filtering,
  and `CheckedTimerConfig`. The checker delegates realizability to a backend;
  current tests use synthetic backends.
- `HardwarePwm` scales and forwards duty values to a supplied backend and combines
  resource claims. Its lifecycle sets up the pin, not the timer backend.
- AVR supplies timer configuration/output operations and ATmega328P bindings,
  exercised through host register mocks. Waveform-specific TOP corrections remain
  separate outstanding work.
- Core's `SelectionResolver` is explicitly a placeholder. There is no working
  request-to-AVR adapter or global timer allocator to preserve.

## Proposed observable guarantees

1. **Repeatability.** Identical normalized requests, capabilities, reservations,
   clock configuration and allocation-policy version produce identical bindings
   or the same structured failure. No pointer addresses, compiler type names,
   registration order, hash iteration order or runtime state influence selection.
2. **Order independence, required.** Permuting declarations of the same requests,
   inventory entries or reservations leaves the result unchanged. Each request has
   an explicit stable identity, proposed as `(module instance key, local key)`.
   Duplicate identities are errors, including two declarations of an identical
   request. Resource and configuration choices also have stable backend keys.
3. **Completeness within the supported model.** Find an assignment whenever one
   exists among the backend's declared, supported candidates. A greedy dead end
   is not proof of impossibility. This does not claim support for hardware modes
   omitted from the backend capability model. This is mathematical completeness
   when search runs to completion; resource-limited execution may instead report
   exhaustion. A practical guaranteed problem envelope remains a prototype gate.
4. **Applicable mandatory requirements stay mandatory.** First select the common
   and resident-target requirements as described below. Unsupported categories,
   impossible values, routing conflicts and reservation conflicts in that active
   configuration cause failure. No implicit dropping, frequency approximation or
   resolution reduction is permitted. Ignoring explicitly nonresident sections
   is required target selection, not a failure to satisfy an active requirement.
5. **Static allocation.** Resolving a plan requires no runtime search, allocation
   tables or startup-order decisions. The result is usable as typed bindings and
   compile-time metadata; executing setup is a separate operation.

Repeatability does not promise unchanged assignments after adding/removing a
request, changing the board policy, changing capabilities or revising the selection
policy. Applications requiring a particular binding can constrain it explicitly.
No optimality claim about power, timer scarcity or number of timers is made in v1.

## Common requirements and target-specific configuration

One source configuration must be able to express portable intent together with
MCU-specific choices for several targets. A conceptual example, not settled C++
syntax or a claim about an implemented ESP32 backend:

```text
PWM request "motor"
  Common: exact frequency 1 kHz, maximum duty step 1/256
  For AVR: fast PWM, TOP from ICR
  For ESP32: target-specific timer/clock options
```

On AVR, the common requirements and AVR options apply; the ESP32 options have no
effect. On ESP32, the common requirements and ESP32 options apply; the AVR options
have no effect. A target with no matching section still receives the common
requirements and uses its documented backend defaults where needed.

Proposed representation: explicit target-scoped sections around requirements/options,
with unscoped requirements belonging to the common contract. The precise spelling,
target tags and extension mechanism are open. Do not preserve the existing enum,
template signatures, filtering behavior or `CheckedTimerConfig` interface merely
for compatibility. A replacement may reuse useful implementation pieces.

Target selection precedes capability validation and allocation:

1. Identify the resident target from explicit build/application configuration.
2. Retain common requirements and all sections applicable to that target.
3. Ignore nonresident sections without instantiating their capability checks,
   drivers, resource claims or configuration computations.
4. Validate and normalize the resulting active request, then allocate it.

Ignoring an inactive section means it cannot affect assignments, diagnostics about
hardware capabilities, ownership or generated hardware operations. It must still
be well-formed C++ when written as C++; unknown names or syntax errors are not
hidden by this rule. Target option vocabulary must therefore be available as
lightweight declarative metadata without importing that target's SDK, register
implementation or compiler-only headers. Realization belongs to the resident
backend and must be instantiated lazily.

The distinction is applicability, not support: an option inside a nonresident AVR
section on ESP32 is ignored; an unsupported common requirement on ESP32 is an
error; an unsupported
ESP32-specific option on ESP32 is also an error. Do not infer non-applicability
merely because a backend does not recognize an option.

On an active section, also check the option's declared domain: an AVR option
accidentally placed in an ESP32 section must be diagnosed on ESP32, not silently
filtered out. A missing or ambiguous resident target is an input error. Target
matching uses explicit identities/traits, never whether an SDK happens to be
installed. An unknown active backend cannot fall back to another target.

Proposed composition rule: active target options refine the implementation while
common requirements remain in force. Incompatible active constraints fail rather
than silently overriding one another. If family and device-specific sections can
both match, combine their constraints under the same rule; source order never
chooses a winner. Any future override mechanism would need explicit semantics.
This composition rule and the target-selector hierarchy remain design proposals.
The worked examples use explicit family/device tags and constraint intersection.
Target sections may contain generic requirements as well as backend-specific
options, allowing a value that intentionally differs by target to live there.

Changing or reordering only inactive sections must leave the active allocation
unchanged. Cross-target assignments need not match: each target has its own
capabilities and receives its own applicable constraints.

## Requests and capability contract

Before target selection, a request can name a logical application endpoint such
as `MotorOutput`. Board policy or a target section binds it to a physical pin for
the resident target. A target-specific physical pin can instead be specified in
that target's section. A raw pin number in common configuration is not portable:
the same number on two MCUs does not establish the same identity or routing.
The first scope requires an explicit resident binding, not automatic pin choice.
Missing or contradictory active pin bindings fail; inactive bindings have no
effect. Equivalent board aliases normalize to the same physical identity.

After target selection and pin resolution, an allocation input carries request
identity, operation (initially PWM), physical pin, frequency requirement, minimum
duty resolution, optional binding constraints, and optional explicit sharing-group
identity. Clock sources/rates are explicit input facts.

Following the worked examples, recommend an explicit accuracy policy alongside a
positive rational frequency: exact, or a bounded relative error in parts per
million. Include both in the first scope. Exact requests fail if unrealizable;
tolerant requests accept only rates inside the declared inclusive bound. Equivalent
ratios normalize identically. Rates are nominal, derived from declared clocks;
oscillator accuracy and jitter are not covered by this numeric contract. Tolerance
is an acceptance constraint, not an implicit closest-frequency preference.
The new API need not retain `Frequency<Hertz, Divider, Type>`. Numeric representation
must express the selected accuracy contract without incidental runtime floating
point in integer paths. This revises the earlier exact-only first-scope proposal;
the user has not yet accepted either the scope or the proposed spelling.

Following the worked examples, recommend `DutyStepAtMost<N, D>`: exact 0%/100%
endpoints and no gap larger than N/D of full scale between consecutive attainable
steady duty values. Merely counting distinct values does not guarantee spacing or
coverage. A backend must expose enough information to check the step guarantee;
register width alone is insufficient. Defer any bits-based shorthand until its
ratio convention is explicitly defined. Duty-write rounding and transitions need
a separate endpoint contract before integration. This recommendation is not yet
accepted. Boundary comparisons must avoid overflow. Existing metadata and tests
do not establish hardware semantics that the new API must preserve.

Duty capability belongs to one fixed concrete candidate, not a union across
different timer configurations. For a group, all combinations of the declared
endpoint duty values must be simultaneously realizable. Updating one endpoint
must preserve the other endpoints' selected steady duties and the common timing
configuration. Distinct channel names backed by one inseparable compare value
cannot satisfy this independent-output contract. This does not promise atomic
multi-output updates or glitch-free transitions.

A candidate supplies:

- Stable timer, channel, pin and configuration identities plus an explicit,
  documented backend preference rank; ties use the stable identities.
- Actual frequency as a rational value and effective duty capability, independently
  checkable against the requested values.
- The complete resource footprint: exclusive resources, occupied ranges and any
  shared configuration domain. Register names and encodings stay in the backend.
- A concrete configuration type and endpoint binding operations, with no hardware
  access performed while enumerating or selecting candidates.

The backend must represent all relevant alternatives in its supported model,
including choices that consume different channels or resources. It may collapse
alternatives only when doing so preserves every possible joint assignment under
the declared policy and the canonical selected result. Candidate production must
be finite; deriving valid counts algebraically is preferable to enumerating every
possible counter value. Backend
completeness and generic solver completeness require separate validation.

## Sharing and ownership

Independent requests are exclusive by default, even if their frequencies match.
Explicit fixed-frequency groups may share a timer only when the backend can
produce one common concrete configuration satisfying every member. Equal nominal
frequency alone does not prove compatible waveform, TOP, clock or channel usage.
Every physical pin and output channel remains exclusive within the group.
The worked examples recommend that an explicit group requires a single timer;
failure to find a common configuration does not fall back to separate timers.
Whether to include this sharing feature in the first implementation remains open.

Represent a sharing group as one configuration owner with several endpoints.
The owner claims/configures the timer once; endpoints claim their own distinct
pins/channels. The group's complete footprint conflicts with external timer,
channel and pin users, including manually bound users and board reservations.
Repeated exclusive claims and overlapping ranges remain errors. The design does
not relax that rule or silently deduplicate two user requests.

Existing `shared_use_claim` is not by itself a complete timer ownership model.
The following ownership rules are a refined proposal, not an implementation or a
user-approved extension to ordinary claims. Final application checks must see each
owner and endpoint once; wrapper forwarding must not introduce a second declaration
of ownership. A group is an explicit composition, not an allocator loophole.

### Proposed resource compatibility rules

Backend metadata identifies physical resources and explicit containment relations,
such as T0 containing channels T0/A and T0/B. Pin routing is a connection, not
containment: a pin does not become a child of every timer that can drive it.
Normalize aliases before checking ownership, and reject cyclic containment.

Each exclusive request or sharing group is one allocation unit with one timer
owner. The backend proposes a structured footprint, checked before search:

- Exactly one whole-timer ownership declaration per unit, plus explicit internal
  endpoint assignments. A child assignment beneath that owner is authorized
  internal use, not a second independent whole-timer claim.
- Every endpoint needs distinct exclusive channel and pin resources. Repeated
  endpoint assignments and overlapping exclusive ranges fail within the unit as
  well as between units. Authorization under a common parent does not excuse
  duplicate children, duplicate whole-owner declarations or overlapping ranges.
- Whole ownership conflicts with any external ownership or reservation of the
  same resource, an ancestor, or a descendant. Thus an external T0/A claimant
  conflicts with the unit owning T0 even when the unit currently uses only T0/B.
- Ordinary exclusive and range claims are external reservations during allocation.
  Their physical identities must normalize to the same inventory identities as
  automatic bindings. Missing identity/containment metadata is a model error,
  not permission to assume compatibility. The adapter from existing claim types
  remains implementation work.

For a configurable domain shared by otherwise separate timers, such as a common
divider, candidates declare `(domain identity, exact normalized setting)`.
Declarations for the same domain are compatible only if their settings agree;
exclusive ownership/reservation of that domain conflicts with either declaration.
Candidate generation must expose all permitted setting alternatives, so search
can backtrack to a common setting. After selection, compose one domain setup owner
from these agreeing uses; endpoint/timer owners cannot independently reconfigure
it. This composes an explicitly shared domain, not repeated exclusive ownership.
An immutable input clock is an input fact rather than a configurable shared claim.

These fixed-setting rules must suffice for every domain admitted by the first
backend model. A backend needing additional aggregate constraints must expose a
defined whole-plan compatibility rule before support is claimed; matching pairwise
labels alone is insufficient. The solver must never assume that unknown resource
relationships are independent. The worked examples include a compatibility matrix
for internal children, external claims, shared domains and coupled duty outputs.

## Deterministic selection procedure

For the proposed order-independent contract:

1. Select the common and resident-target configuration. Validate and normalize
   active identities, requests, reservations and fixed claims.
   Validate the active identity structure first. If identities are duplicated,
   stop before validating request contents and report canonical duplicate-identity
   errors, one per repeated full identity with its occurrence count. Sort those
   errors by identity; do not choose a first declaration or attach its contents.
   For later phases, order diagnostic records by category, identity and normalized
   constraint/resource details, not source position. Identical records may be
   collapsed with an occurrence count. Compiler traces remain outside this promise.
2. Treat each exclusive request or explicit sharing group as an allocation unit.
   Order units by stable identity; use the least member request identity as the
   group ordering key. A request belongs to at most one group. Sort group members
   by request identity too. A group candidate contains all member endpoint bindings
   in that canonical order, not their source declaration order.
3. Ask the backend for candidates satisfying each unit's complete requirements.
   Remove candidates conflicting with reservations or fixed claims. Order them by
   documented preference rank, then timer/configuration/endpoint identities.
4. Search in that order with backtracking. Select the first complete compatible
   assignment. This defines the lexicographically first feasible candidate vector
   and prevents an early flexible request from blocking a later constrained one.
5. Emit bindings, their actual capabilities and the combined ownership plan.

All identity comparisons require documented total orders. Candidate keys must
uniquely identify their semantics within an allocation unit; two entries with the
same key but different capabilities, bindings or footprints are a backend-model
error. Identical repeated candidate entries may be normalized once. This is
inventory normalization, not permission to merge repeated user resource claims.
Stable request identity is separate from allocation preference: renaming an
identity may change a tie-break result, but reordering its declaration may not.
The worked examples recommend module-local request keys under explicit named
instance paths, using restricted ASCII components ordered lexicographically as
tuples. References to an existing instance do not declare it again; application
composition must distinguish references from duplicate instance declarations.

Search optimizations are allowed only if they preserve this selected result, not
merely find some feasible result. A work limit, if needed, must report search
exhaustion separately from unsatisfiability. Compiler resource-limit failures also
do not prove that no allocation exists. A guaranteed supported problem size and
practical compile-time budget must be established with the first solver prototype.

Example using a synthetic inventory: request A can use T0 or T1; B can use only
T0. Both need exclusive timers and T0 is preferred. The solver first tries A=T0,
backtracks when B cannot fit, then returns A=T1 and B=T0. Reordering A and B in
source must preserve that result under the proposed identity-based policy.

## Result, diagnostics and application lifecycle

A successful plan permits lookup by request identity and exposes the selected
timer/channel/pin, actual frequency, effective resolution and resource footprint.
The exact C++ spelling is deferred; it must be inspectable through static assertions
without generating runtime reporting machinery.

Failure categories distinguish malformed/duplicate input, unsupported requirements,
no individually suitable candidate, reservation/fixed-claim conflicts, joint
allocation conflict and search-limit exhaustion. Report the involved request and
resource identities in canonical order. A minimal conflicting subset is useful but
not required for v1. Structured diagnostic content is deterministic; compiler
wording and template trace formatting need not be byte-identical.

The application initializes each selected configuration owner exactly once before
dependent modules use its endpoints. Endpoint setup must have a single defined
owner too; it cannot be independently repeated by both the portable wrapper and
the AVR adapter. Allocation does not imply simultaneous or glitch-free hardware
updates. Establish the initialization contract before wiring `HardwarePwm` to AVR.

## Ownership and implementation gates

- Core owns generic candidate compatibility, deterministic search and diagnostics.
- Peripherals owns portable timer request semantics and typed selection/bindings.
- MCU backends own capabilities, routing, realizable configurations and drivers.
- Board/platform policy owns aliases, clock inputs and framework reservations.

Order independence, cross-target configuration coexistence and freedom from API
compatibility are agreed. Next settle target-section composition and vocabulary,
stable identities, frequency/resolution meaning, explicit sharing, and completeness.
Then implement a small synthetic solver proof
before integrating one AVR path. Correct AVR TOP/frequency calculations before
using them as the capability oracle. No new repository or broad allocation
framework is required to begin this work.

Acceptance evidence should cover common plus resident-target selection; inactive
section invariance and absence of nonresident SDK dependencies; rejection of
unsupported active requirements; conflicting active sections; logical pin binding
and aliases; missing/ambiguous target selection; misplaced active option domains;
permutation invariance including group members; candidate-key collisions; stable tie-breaking;
the greedy-trap example; exhaustive agreement with a small independent feasibility
oracle; reserved and fixed resources; aliases of one physical pin; duplicate
identities/claims; overlapping ranges; compatible and incompatible sharing;
unsupported requests; exact-frequency/resolution boundaries; and distinguished
search exhaustion. The AVR integration then needs one portable request through
allocation and lifecycle setup to expected mock register writes and duty updates.

These are proposed future checks, not checks run for this documentation change.
AVR compiler and hardware validation remain on hold. Implementation and review
must follow [CrossMcu.md](review-policies/CrossMcu.md) and, for AVR code or AVR
instantiations, [Avr.md](review-policies/Avr.md).

## Open decisions after the first review loop

Only order independence, common/resident configuration selection and absence of
backward-compatibility obligations have been accepted. The following need design
decisions; descriptive wording elsewhere in this draft does not settle them.

| Decision | Current proposal or alternatives |
| --- | --- |
| Configuration spelling and option packaging | Worked examples propose `For<Target, ...>` with SDK-free option declarations |
| Overlapping target sections | Intersect active constraints; reject contradictions; no implicit overrides |
| Request identity | Named hierarchical instance paths plus local request keys; lexicographic component order |
| Frequency accuracy | Explicit exact or relative-tolerance policy; recommend both in the first scope |
| Duty resolution | Recommend a full-range maximum duty-step ratio, with bits shorthand deferred |
| Sharing | Explicit groups require one common timer owner; first-implementation scope remains open |
| Solver guarantee | Complete over the declared candidate model, with separate search-limit failure |
| Initial scope | Fixed-frequency PWM is a proposed first increment, not the whole portable timer API |

Initial review record: one local review/revision/recheck loop, before the
independent review recorded below. The review covered the shared contract and
AVR integration boundaries under the linked policies. It corrected gaps introduced
by this draft: physical-pin-only input did not explain cross-target application
bindings; counting duty levels overstated precision; group/candidate ordering left
ties unspecified; and active option-domain mistakes could have been silently
discarded. A second pass traced common-only, common-plus-resident, inactive-section
changes, conflicting active sections, reordered groups and colliding candidate
keys through the revised rules. These are design walkthroughs, not executable
tests or evidence that a backend implements the contract. The open decisions
above remain prerequisites to implementation.

An independent adversarial review subsequently identified incomplete resource
compatibility rules and diagnostic ordering for duplicate identities. The revised
proposal above specifies internal ownership versus external exclusion, agreement
for shared settings, and identity validation before request-content validation.
It also makes simultaneous independent duty capability and the distinction between
mathematical completeness and operational limits explicit. See the separate
[review record](GrevirTimerDesignReview.md) for findings, disposition and recheck.
