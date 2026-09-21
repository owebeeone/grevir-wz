# Portable timer API — worked design examples

Status: proposed API, 21 September 2026. These examples are design specifications,
not compilable examples of an implemented library. All new spellings are open to
change. Read with [the allocation design](GrevirTimerAllocationDesign.md).
The subsequent [host prototype](../experiments/timer-allocation/README.md) provides
compilable examples for a narrower subset and records its differences explicitly.

The examples recommend concrete semantics for discussion. They do not turn those
recommendations into user-approved decisions. Only deterministic allocation,
independence from declaration order, common plus resident-target configuration,
and freedom from backward compatibility have been agreed.

## 1. One request, multiple target sections

Illustrative C++:

```cpp
using MotorPwm = PwmRequest<
    "pwm",
    Pin<Endpoint<"drive">>,
    Frequency<Hertz<1000>, Exact>,
    DutyStepAtMost<1, 256>,
    For<target::Avr, avr::FastPwm, avr::TopFromIcr>,
    For<target::Esp32, esp32::ClockSource<esp32::Apb>>>;
```

`"pwm"` is a module-local request key. `"drive"` is a module-local logical endpoint
that application/board configuration binds to a physical pin. The common portion
requires 1 kHz and no gap larger than 1/256 of full scale between attainable duty
values, including 0% and 100%. Target sections constrain how to implement it.

| Resident target | Active requirements | Inactive requirements |
| --- | --- | --- |
| A supported AVR device | Common + fast PWM + ICR TOP | ESP32 clock option |
| A selected, supported ESP32 device | Common + APB clock option | AVR waveform/TOP options |
| Another supported backend | Common, using that backend's documented defaults | Both target sections |

This table specifies target selection only. It does not promise that any device
can realize the request: the active backend still checks its actual clocks,
waveforms, routing and reservations. No ESP32 variant or driver is selected by
this proposal; the clock option is illustrative declarative vocabulary.

The AVR and ESP32 option declarations must both be available without their SDKs.
On AVR, changing a well-formed ESP32 section to an unrealizable ESP32 configuration
must leave the AVR result unchanged. It must not instantiate ESP32 validation or
claim its resources. Unknown C++ names and invalid syntax remain compiler errors.

Recommend an outer `For<Target, ...>` for applicability, with options retaining
domain tags for checking active sections. Do not rely on an unrecognized option
being discarded. An AVR option accidentally put in an active ESP32 section fails.

## 2. Family and device sections combine as constraints

```cpp
using Request = PwmRequest<
    "pwm",
    Pin<Endpoint<"drive">>,
    Frequency<Hertz<1000>, Exact>,
    DutyStepAtMost<1, 256>,
    For<target::Avr, avr::FastPwm>,
    For<target::Atmega328P, avr::TopFromIcr>>;
```

The target descriptor explicitly declares that ATmega328P matches both its device
tag and the AVR family tag. Both sections apply on that device. Other AVR devices
receive only the family section; ESP32 receives neither. Board selection supplies
pin bindings and reservations separately from this target matching.

Replace the device option with `avr::PhaseCorrectPwm`: on ATmega328P the active
request now requires both fast and phase-correct waveform selection, so it fails
with a conflicting-waveform diagnostic. The device section does not override the
family section. Swapping their source order changes nothing.

When one requirement is intentionally different by target, put its alternatives
in disjoint target sections, leaving only the invariant requirements common. A
common 1 kHz requirement plus an active 2 kHz requirement is a contradiction, not
an override. More general target predicates are a later syntax decision; do not
introduce implicit “most specific wins” precedence.

## 3. Reusable modules have local names, applications name instances

```cpp
struct MotorModule {
    using Requests = RequestList<MotorPwm>;
};

using Motors = Instances<
    Instance<"left", MotorModule>,
    Instance<"right", MotorModule>>;

using LeftDrive = EndpointRef<"left", "drive">;
using RightDrive = EndpointRef<"right", "drive">;
using LeftPwm = RequestRef<"left", "pwm">;
using RightPwm = RequestRef<"right", "pwm">;
```

Full request identities are tuples of path components, here `("left", "pwm")`
and `("right", "pwm")`. A module author names each local request once; the
application names each instance once. Nested instances extend the path. No global
numeric IDs, C++ type-name ordering or declaration-order-generated IDs are needed.

Recommend restricted ASCII identifier components and lexicographic comparison of
the component tuples, with shorter prefixes first. Compare actual keys, not only
hashes. Reversing `Instances` preserves assignments. Renaming an instance can change
tie-breaking; identity changes are not declaration-order changes.

Application/board bindings map `LeftDrive` and `RightDrive` to physical pins for
the active target. Different target maps may use entirely different pins. Missing
bindings fail. Two endpoint aliases bound to the same physical pin conflict even
if their textual names differ.

Two instances named `"left"` under the same parent fail. Two declarations of local
request `"pwm"` in one instance fail. These are different from two modules referring
to one existing dependency: a reference is not a second instance declaration.
Application composition must represent that distinction explicitly.

For duplicate identities, report one canonical duplicate error with the identity
and occurrence count before checking request contents. For example, two
`("left", "pwm")` declarations containing different invalid numeric requirements
still report the same duplicate error when reversed. The allocator does not choose
one declaration as authoritative. Ordinary C++ syntax/name errors remain separate.

## 4. Deterministic selection requires considering the complete assignment

Use this synthetic inventory; these routes are not ATmega328P or ESP32 claims.
All listed candidates meet the requested frequency and duty step. T0 and T1 are
exclusive timers. Both have preference rank zero; stable timer key breaks ties.

| Request | Bound pin | Permitted timer candidates |
| --- | --- | --- |
| `("a", "pwm")` | P0 | T0, T1 |
| `("b", "pwm")` | P1 | T0 |

Expected assignment is `a/pwm → T1`, `b/pwm → T0`. Trying T0 for A first leads to
backtracking, not failure. Reordering instances, requests, candidate entries or
inventory entries must produce exactly this assignment.

Reserve T0: B has no available candidate, so compilation fails and identifies
B and the reservation. There is no partial successful plan for A. Instead give
both A and B only T0 with no reservation: each request is individually possible,
but their combined allocation fails. These are distinct diagnostic situations.

For a tie case where both requests can use either timer, the canonical result is
A→T0 and B→T1. The solver chooses the lexicographically first feasible candidate
vector under documented preference/key ordering. Duplicate keys describing
different candidates are a backend-model error, not permission to use input order.

## 5. Frequency accuracy is explicit and useful on quantized hardware

Recommend supporting explicit tolerance in the first scope, alongside exact
frequency. Deferring all tolerance would make portable requests unnecessarily
fragile across different clock/divider combinations.

```cpp
using Strict = Frequency<Hertz<1000>, Exact>;
using Approximate = Frequency<Hertz<1000>, WithinPpm<1000>>;
```

For synthetic candidates with exact nominal frequencies:

| Candidate frequency | Strict | Approximate (inclusive 999..1001 Hz) |
| --- | --- | --- |
| 1000 Hz | Accept | Accept |
| 999 Hz | Reject | Accept |
| 1001 Hz | Reject | Accept |
| 998 Hz | Reject | Reject |
| 1002 Hz | Reject | Reject |

For positive requested rational frequency F and actual rational A, acceptance is
`abs(A - F) / F <= ppm / 1,000,000`. This is an exact rational comparison; floating
point is not needed, and arithmetic must avoid overflow. Frequencies describe
nominal rates calculated from declared clock inputs, not measured oscillator
accuracy, jitter or drift. A tolerance contract for those is separate work.

Recommend spelling the accuracy policy explicitly rather than hiding a default.
Tolerance filters candidates; it does not by itself request the closest rate.
Among acceptable candidates, the existing documented allocation ordering applies.
The result exposes the actual rational rate, so acceptance cannot hide rounding.

## 6. Duty granularity should have a measurable meaning

Recommend `DutyStepAtMost<N, D>` as the underlying requirement, with positive
ratio at most one. It requires full 0..1 coverage, supported exact endpoints,
and a maximum gap of N/D between consecutive attainable steady duty fractions.
A backend cannot satisfy it merely by having a wide compare register. A future
bits-based shorthand can map to an explicitly documented ratio.

| Synthetic attainable duties | `DutyStepAtMost<1, 256>` |
| --- | --- |
| k/256 for every integer k in 0..256 | Accept: largest gap is 1/256 |
| k/255 for every integer k in 0..255 | Reject: largest gap is 1/255 |
| 0 plus 255 distinct values in 0.99..1 | Reject: many values, but a large gap |
| k/1024 for every integer k in 0..1024 | Accept: finer granularity |

This describes available steady outputs. It does not yet specify rounding of
arbitrary duty writes or their transition behavior. Those must be specified for
the endpoint adapter before end-to-end integration. Endpoint support may include
backend-managed constant low/high output, if that is part of its declared PWM
contract. No claim of glitch-free transitions follows.
The duty set belongs to one fixed candidate. Combining attainable values from
different TOP/clock/waveform configurations cannot establish its granularity.

## 7. Shared timer ownership is explicit

```cpp
using Ownership = SameTimer<
    RequestRef<"left", "pwm">,
    RequestRef<"right", "pwm">>;
```

Recommend that `SameTimer` requires one common configuration owner. It is not
permission to share opportunistically or silently fall back to separate timers.
Without it, the requests own independent timers under the proposed first policy.

For a synthetic timer with channels A and B routed to distinct bound pins, two
1 kHz requests can share if one concrete configuration meets both duty requirements
and all active target constraints. The result has one timer owner and two exclusive
endpoints. Different duty values do not require different timer configurations.

| Variation | Expected result |
| --- | --- |
| Distinct pins/channels, one compatible configuration | One owner, two endpoint bindings |
| Both endpoints need channel A | Fail: channel conflict |
| Both aliases resolve to one physical pin | Fail: pin conflict |
| Exact 1 kHz versus exact 2 kHz | Fail: no common configuration |
| Requested TOP source consumes a required output channel | Fail unless another permitted common candidate exists |
| Another module owns the whole timer | Fail: ownership conflict |
| Same request reference repeated in the group | Fail: duplicate member |

All group members must be resolved, sorted and checked before candidate selection.
Initialize the configuration owner and its endpoints once before dependent modules
run. Members can change their own duty; the fixed-frequency endpoint API exposes
no independent frequency control. Runtime frequency changes need a separate owner
contract and remain outside this first scope.

## 8. Ownership and simultaneous duty counterexamples

These synthetic cases exercise the resource compatibility rules added after the
independent review. Resource identity/containment is explicit backend metadata.

| Configuration | Expected result |
| --- | --- |
| One T0 owner with internal A/P0 and B/P1 endpoints | Valid if both endpoints meet the common configuration |
| Same group plus an external whole-T0 reservation | Conflict with the group owner |
| T0 owner using B, external claim on T0/A | Conflict: whole ownership excludes external child use |
| Two internal endpoints both assigned T0/A | Conflict even though they have the same owner |
| Two pins with different aliases but one physical identity | Conflict after alias normalization |
| Separate T0/T1 owners require shared divider D=8 | Compatible; compose one D setup owner |
| Separate T0/T1 owners require D=8 and D=64 | Incompatible; backtrack to agreeing candidates or fail |
| Exclusive external reservation of D, candidate requires D=8 | Conflict, even if the external setting happens to be 8 |
| Distinct channels A/B driven by one inseparable compare value | Cannot offer independent duty control |

For the last case, each channel separately might support k/256 for k=0..256.
Nevertheless A=25% and B=75% cannot coexist. Such a candidate must not pass by
checking each channel's granularity separately. All declared duty combinations
must be jointly attainable under the fixed candidate, and updating one endpoint
must preserve the other's selected steady duty. A coupled-output API would be a
different contract and is not implicitly substituted.

## Walkthrough outcome and remaining work

These examples make concrete recommendations: explicit target sections with
constraint intersection; hierarchical instance/local identities; explicit exact or
tolerant frequency; a duty-step guarantee; and explicit mandatory sharing groups.
They are ready for user discussion and independent design review, not yet an
implementation specification accepted by the user.

Before coding, review the user-facing forms and decide the initial sharing scope.
Before integration, specify duty-write rounding, initial output state, owner setup
ordering and failure behavior. Prototype evidence must then establish lazy inactive
section handling, canonical allocation and a practical search budget. The proposed
diagnostics need stable categories and request/resource context, not a particular
compiler's wording. No compile, AVR-target or hardware validation was performed
for these documentation examples; existing validation holds remain in force.
