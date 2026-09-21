# Timer allocation prototype and ATmega328P PWM MVP

The implementation has been promoted into installed Grevir packages. The local
headers now forward to Core/Peripherals/AVR; fixtures and oracles remain here.
See [installed application integration](../../dev-docs/GrevirPwmIntegration.md) for
module declarations, automatic setup and Core resource claims. `timer_prototype`
is only a local alias for `grevir::pwm` used by the retained checks.

## Run

From the workspace root, with the existing host compiler and CMake:

```sh
cmake -S experiments/timer-allocation -B build/timer-prototype \
  -DCMAKE_BUILD_TYPE=Debug -DTIMER_PROTOTYPE_SANITIZERS=ON
cmake --build build/timer-prototype
ctest --test-dir build/timer-prototype --output-on-failure
```

There are no downloaded dependencies, SDKs or Catch2 requirements. This project is
separate from the production workspace's 125-case host suite. AVR compiler and
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
- All four CTest checks pass with address/undefined-behavior sanitizers enabled.
  Sanitizers exercise the host allocation/frequency checks; static assertions
  establish their own constant-evaluation validity. All nine headers compile
  independently; the README example compiles. All fourteen C++ files pass the
  raw-token control-body brace check.

This is a small verified model, not a complexity guarantee for all applications.
The eight-request example has an immediate solution; it does not establish a
worst-case compilation bound. Default search budget is 100,000 attempted eligible
candidate extensions, counted before compatibility checks; parsing, validation,
candidate generation and compiler-internal limits are outside that budget.

## Deliberate limits

- Explicit candidate inventories, physical pins and numeric sharing-group IDs
  remain in the synthetic model. The ATmega328P adapter generates candidates and
  initializes each selected timer once; logical board maps, `SameTimer` composition,
  nested module paths and dependency references remain deferred.
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
  numerators are at most 2*10^12 and denominators at most 10^6. Candidate frequencies
  can use the full positive uint32 numerator/denominator range. An exact Euclidean
  fraction comparison avoids overflowing cross products against frequency bounds.
  This is host/compile-time metadata, not firmware duty arithmetic.
- The prototype reports one canonical primary diagnostic, not the complete
  ordered error collection proposed in the design. Shared-domain setup owners and
  waveform simulation remain deferred. AVR typed bindings and register/duty updates
  are described below.

## ATmega328P MVP boundary

`atmega328p_candidates.hpp` derives synchronous **fast PWM** candidates from the
existing Timer0/1/2 mode/divider/width/output declarations. It includes built-in
TOP, OCRA TOP (B output only), and Timer1 ICR TOP. Timer1's existing 8/9/10-bit
built-in modes follow directly from its table. Physical pad names are explicit;
these are not Arduino digital-pin numbers. Supported programmable TOP is 3..255
or 3..65535. Phase-correct allocation, other devices, asynchronous Timer2, capture,
interrupts, runtime frequency changes, board mappings and a real ESP32 backend
remain TBD. No extra AVR timer-feature project is required for this MVP.

The generator intersects grouped requests first. For each mode/prescaler it picks
the longest period meeting the frequency interval, then checks duty resolution
and pin routes. Shorter periods within that mode consume the same resources and
have worse duty granularity, so retaining only the longest loses no solution in
this fixed-frequency scope. Candidates prefer the smallest prescaler, then numeric
WGM code; ordinary canonical timer/configuration/endpoint ordering breaks ties.
Generated keys are stable under declaration reordering of the same request set.

`atmega328p_program.hpp` forwards to the installed AVR implementation, which emits typed endpoints. `setup()` initializes each chosen
timer once: stop/normal mode, disconnect compares, reset count, write programmable
TOP, initialize requested outputs LOW, apply PWM mode, start clock. The caller
must have enabled the peripheral clocks, left Timer2 synchronous (`AS2=0`), disabled
timer interrupts, and supplied exclusive ownership plus byte access/barrier policies.
Setup is sequential, with no promise of glitch-free live reconfiguration. Duty
updates guarantee the eventual steady setting, not cycle-synchronous transitions.

```cpp
#include "atmega328p_program.hpp"
using namespace timer_prototype;
namespace m = timer_prototype::atmega328p;

using Motor = PwmRequest<"pwm",
  Frequency<Hertz<1000>, WithinPpm<10'000>>, DutyStepAtMost<1,256>,
  For<Target::avr, Pin<m::PB1>, Frequency<Hertz<1000>,Exact>,
    avr::FastPwm, avr::TopFromIcr>,
  For<Target::esp32, Pin<18>, esp32::ApbClock>>;
// Device = existing TimerBindings<your byte-access policy, your barrier policy>.
using App = m::Program<Device,16'000'000,Instance<"motor",Motor>>;
static_assert(App::plan.ok());
void initialize() { App::setup(); }
bool quarter_duty() {
  return App::Pwm<"motor">::write(1,4); // 25%; TOP=15999, OCR=3999.
}
```

`write(n,d)` rejects d=0 or n>d without IO; otherwise it rounds the number of high
ticks downward. `writeTicks(high_ticks)` exposes every realizable duty step,
including 65535/65536 and full duty on a 16-bit timer. It rejects values above the
period. Zero/full duty use GPIO endpoints; interior fast-PWM duty writes OCR=ticks−1,
including OCR=0 for a one-tick pulse. Group members have independent compare values.
`actual_frequency` and `duty_step` expose the selected exact rational metadata.
Only 32-bit bounded multiplication/division is needed in the dynamic fraction path;
allocation and its temporary vectors are evaluated at compilation.

The sanitizer-backed checks cover all three timers, all six output routes,
independent shared outputs, endpoints/one-tick duty, invalid writes without IO,
maximum TOP and built-in 9/10-bit selection, reservations, OCRA conflicts and
reordering. 168 period searches agree with a separate exhaustive period oracle.
Native optimized setup/duty probes contain no floating or 64-bit arithmetic or
runtime allocation; these are host observations, not AVR code-size/cycle evidence.
The production 125-case suite, isolated AVR installation and installed consumer
pass with the corrected TOP expectations. AVR compiler/hardware validation stays
on hold. The byte mocks verify register effects, not electrical waveforms.

The MVP is now installed and connected to Core application assembly. A named
ESP32 backend and board integration remain separate work. The existing low-level raw-OCR duty API and its inherited runtime rescaling
convention are unchanged; the portable fixed-frequency endpoint uses the explicit
hardware duty conversion above.

## File boundaries

`requirements.hpp` owns target selection and request normalization; `numeric.hpp`
owns bounded rational comparisons; `frequency_window.hpp` owns exact frequency
intervals; `model.hpp` owns inventory/result records;
`validation.hpp` owns topology/ownership checks; `allocator.hpp` owns canonical
search. `atmega328p_candidates.hpp` owns declaration-driven generation and
`atmega328p_program.hpp` owns selected-driver setup/duty binding. Fixtures, static checks, oracle comparison and compiler rejection probes
are separate from that prototype implementation. These boundaries follow the
split-files guidance; no large legacy source was moved or reformatted.
