# Host-only timer allocation prototype

This standalone C++23 experiment exercises the
[proposed design](../../dev-docs/GrevirTimerAllocationDesign.md). It is not a
Grevir library, installed API, MCU backend or migration of working legacy code.
No member repository or production header is changed by this experiment.

## Run

From the workspace root, with the existing host compiler and CMake:

```sh
cmake -S experiments/timer-allocation -B build/timer-prototype \
  -DCMAKE_BUILD_TYPE=Debug -DTIMER_PROTOTYPE_SANITIZERS=ON
cmake --build build/timer-prototype
ctest --test-dir build/timer-prototype --output-on-failure
```

There are no downloaded dependencies, SDKs or Catch2 requirements. This project is
separate from the production workspace's 123-case host suite. AVR compiler and
hardware validation remain on hold.

## Concrete API example

The option types are inert metadata. Only common and resident-target options
are interpreted. This uses explicitly synthetic resource IDs from `fixtures.hpp`:

```cpp
#include "fixtures.hpp"
using namespace timer_prototype;

using Motor = PwmRequest<"pwm",
  Frequency<Hertz<1000>, WithinPpm<10'000>>, // Common: 1 kHz +/- 1%.
  DutyStepAtMost<1, 256>,
  For<Target::avr, Pin<101>, Frequency<Hertz<1000>, Exact>, avr::FastPwm, avr::TopFromIcr>,
  For<Target::esp32, Pin<102>, Frequency<Hertz<1005>, Exact>, esp32::ApbClock>>;

constexpr auto plan = compile(Problem{
  requests<Target::atmega328p, Instance<"motor", Motor>>(),
  std::array{fixtures::choice(1, {"motor", "pwm"}, 1, 101)},
  fixtures::resources, std::array<unsigned, 0>{}
});
static_assert(plan.ok());
static_assert(plan.candidates[0] == 1);
```

The common interval is inclusive 990..1010 Hz. AVR narrows it to exactly 1000 Hz;
ESP32 narrows it to exactly 1005 Hz. An active requirement for 1020 Hz would make
the intersection empty and produce a conflict, regardless of declaration order.
Overlapping tolerances are intersected too; they need not have the same center.
A valid interval without a realizable candidate reports `no_candidate`, distinct
from contradictory clauses in one request.

`compile` is consteval: successful plans and failures are computed at compilation.
The inspectable result contains canonical request keys and candidate IDs, or a
diagnostic category/key/detail. `require_success<plan.diagnostic.status>()` makes a
failed plan a compiler rejection when called in a constant expression. The internal
`solve` also runs on the host for exhaustive comparison; this is not a runtime
firmware allocator. Failed plans never expose a partial assignment.

## What is demonstrated

- One portable request compiles to different synthetic AVR and ESP32 plans.
  Family/device matching, active wrong-domain errors, and active conflicts work.
- Inactive sections can contain incomplete types, invalid numeric requirements
  and a validator that deliberately fails if instantiated. They remain unexamined.
- Identity validation precedes payload validation, including an intentionally
  failing validator in a duplicate request. Keys are `(instance, local)` pairs
  with restricted ASCII components. The primary diagnostic is order-independent.
- Candidate search uses canonical ordering and backtracking. Budget exhaustion is
  distinct from conflict; no partial assignment escapes either result.
- A sharing unit has one timer owner with distinct internal channels/pins. A
  reservation of either a whole timer or any child excludes external use of the
  timer. Physical-pin collisions, duplicate internal channels and inconsistent
  candidate identities are rejected.
- Separate timers may require one shared domain setting. Search finds agreeing
  settings or rejects the combination. An exclusive domain reservation blocks it.
- A candidate declaring coupled duties cannot serve an independent-output group.
  This checks a backend declaration, not actual electrical independence.
- Common and active-target exact/tolerant frequency requirements intersect as
  closed rational intervals; no clause wins by position. Equivalent ratios,
  repeated requirements and touching endpoints normalize exactly. Maximum duty
  step is also checked without floating-point arithmetic.

## Evidence

On AppleClang 21 / arm64 macOS / C++23:

- All static assertions compile, including eight independent requests/eight
  candidates, and one typed configuration applied to both synthetic target tags.
- An independent exhaustive oracle enumerates all 27 assignments for each of 512
  three-request/three-timer eligibility graphs. All six request permutations and
  forward/reversed candidate/resource inventories give 6,144 matching results.
  Failure diagnostics, visit counts and absence of partial assignments are checked.
- 84,672 frequency-membership comparisons agree with separately evaluating each
  clause's relative-error inequality. Checks also cover interval commutativity,
  idempotence, three-clause permutations, fractional touching boundaries, zero
  lower bounds at 100% tolerance and the prototype's maximum arithmetic inputs.
- A positive compiler control and seven expected rejections verify duplicate
  identity, joint conflict, no candidate, invalid model, budget exhaustion and
  reservation failure, plus conflicting common/target frequency requirements.
  Logs are in `build/timer-prototype/probe-*.log`.
- All three CTest checks pass with address/undefined-behavior sanitizers enabled.
  Sanitizers exercise the host allocation/frequency checks; static assertions
  establish their own constant-evaluation validity. All seven headers compile
  independently; the README example compiles. All eleven C++ files pass the
  raw-token control-body brace check.

This is a small verified model, not a complexity guarantee for all applications.
The eight-request example has an immediate solution; it does not establish a
worst-case compilation bound. Default search budget is 100,000 attempted eligible
candidate extensions, counted before compatibility checks; parsing, validation,
candidate generation and compiler-internal limits are outside that budget.

## Deliberate limits

- Explicit candidate inventories, physical pins and numeric sharing-group IDs
  stand in for backend generation, logical endpoint/board maps and `SameTimer`
  composition. Only two-component request identities are implemented; nested
  module paths, dependency references and setup ownership are not implemented.
- Frequency clauses accumulate into one interval; there is no fixed clause-count
  cap in that representation. Duty-step constraints retain the tighter bound.
  Other active constraints still require agreement; there is no implicit override.
- Hardware topology is a two-level timer/channel tree with distinct pin/domain
  roots. At most four endpoints and one optional shared domain occur per candidate.
  Exclusive reservations are physical IDs; range claims, arbitrary resource graphs,
  existing Core claim adapters and aggregate compatibility predicates are deferred.
- Frequency/duty numerators and denominators must be positive and at most
  1,000,000; duty ratio is at most one; tolerance is at most 1,000,000 ppm. Frequency
  bounds are exact rational microhertz, not rounded fixed-point values: bound
  numerators are at most 2*10^12 and denominators at most 10^6. Comparisons therefore
  fit uint64_t (cross products at most 2*10^18). That arithmetic belongs to this
  host/compile-time model and does not prescribe firmware arithmetic.
- The prototype reports one canonical primary diagnostic, not the complete
  ordered error collection proposed in the design. It does not emit typed driver
  bindings, domain setup owners, register writes, duty updates or waveform models.

Before promoting anything into Grevir, settle the public spelling and initial
sharing scope, implement backend candidate generation and the missing binding pieces,
and define duty rounding/lifecycle behavior. Correct AVR TOP conversion before
using the AVR implementation to generate real candidates. The standalone evidence
does not validate an ESP32 device, an AVR driver or a complete portable PWM path.

## File boundaries

`requirements.hpp` owns target selection and request normalization; `numeric.hpp`
owns bounded rational comparisons; `frequency_window.hpp` owns exact frequency
intervals; `model.hpp` owns inventory/result records;
`validation.hpp` owns topology/ownership checks; `allocator.hpp` owns canonical
search. Fixtures, static checks, oracle comparison and compiler rejection probes
are separate from that prototype implementation. These boundaries follow the
split-files guidance; no large legacy source was moved or reformatted.
