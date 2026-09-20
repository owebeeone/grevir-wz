# Grevir extraction progress

Latest checkpoint: 21 September 2026. Five local members now exist: Base, Time,
Core, the first Peripherals increment and development-only Test Support. The rest
of the repository plan remains unimplemented. The initial three-library baseline
was committed on 21 September, followed by the portable GPIO/timing checkpoint
(`569b1b7`). The next increment fixes period division and adds
debounce/button handling. Hardware validation stays on hold.

## Period division, debounce and button events — 21 September 2026

Gianni authorized fixing period division and extracting debounce/button handling.
`Period::operator/` now divides rather than multiplies, retaining its storage type,
units and ordinary numeric truncation. The regression first reproduced `12 / 3`
returning 36; signed/unsigned, fractional and truncation checks now pass.

Two further source mappings are extracted: `DebounceInput` from `ardOinus.h` and
`ButtonEventModule` from `ardo_button_events.h`. Their new signatures are
`DebounceInput<InputPin, Clock, debounceTime = 300>` (interval in clock ticks) and
`ButtonEventModule<InputPin, Clock>`. Pins retain their inherited claims/interfaces.
Button thresholds retain their physical 300 ms / 400 ms durations across clock
units. The active-low classifier and its decision order are preserved, including
second-press priority over a simultaneously observed timeout. Event retrieval still
consumes one pending slot rather than a queue.

Mock sequences exposed inherited debounce errors: returning to the accepted level
did not cancel a pending transition, initial high input was reported low, and setup
retained previous state. Debounce now cancels the pending transition, samples the
initial level during setup and resets its state. Strict `>` expiry remains.
Button setup also clears classification state and pending events. Its previously
undefined static instance now has a template definition in the public header.

Validation on Apple Clang 21 / arm64 macOS / C++23:

- The ten new focused runtime cases initially had eight failures; all now pass.
  The full suite passes 37/37: Base 4, Time 4, Core 10, Peripherals 19.
- Button/debounce cases exercise startup, both bouncing edges, strict debounce
  expiry, repeat setup, delayed single click, double click, long press, event
  consumption, clock wraparound and native microsecond clock thresholds.
- All 19 peripheral cases also pass together in seeded random order: 106 assertions.
- Eight peripheral public headers compile independently. Raw/debounced pin probes
  pass two valid cases and reject two conflicts with the intended diagnostic.
  Existing Core application/index probes remain passing.
- Isolated Time and Peripherals builds pass their host suites offline against
  installed dependencies. The installed consumer executes output timing, a
  debounced long click and period division with Catch2/Test Support discovery
  disabled. Logs are under ignored `build/button-packages/`.
- The raw-token scope check passes across 80 extracted C++ files, including disabled
  branches. All 736 original source hashes remain unchanged, and all 52 extracted
  assignment hashes match the current files in the ledger.

Hardware validation stays on hold. The separate
sequencer, PWM, timer selection, storage regions and MCU bindings remain later work.

## Portable GPIO/timing and foundation tests — 21 September 2026

The first `grevir-peripherals` increment extracts pin interfaces, input pins,
output/open-drain/external pins, and `TimePoller` with cyclic/finite time sequences.
Pins now take an explicit GPIO backend and logical mode enums. Pollers take an
explicit clock exposing `TimeType` and `now()`; `Sequence` takes an explicit tick
type. Class names/namespaces and original polling behavior remain: strict `>`
expiry, one-step catch-up, `init()` preserving state and `reset()` clearing it.
Callers must supply the new bindings; no MCU/Arduino adapter has been introduced.
Pin resource identity remains the logical pin number, independent of backend type.

All seven retained Base/Time test files now build under Catch2. The buffer harness
now honors requested operation counts and actually selects both buffer variants;
integer-width checks explicitly instantiate the original boundaries. Five Base
files contribute four runtime cases and static type-algorithm assertions. Two Time
files contribute three runtime cases, including unsigned wraparound. The earlier
PICOS/NANOS enum correction is included in this build; no dedicated regression was
added for it. The inherited `Period::operator/` multiplication defect was observed
while reading the time implementation and recorded in Time's README; it remains
outside this extraction and outside the retained test coverage.

`grevir-test-support` now owns shared Catch2 setup, supporting installed 3.8.1,
local source and explicit SHA-256-pinned fetching. Test fixtures stay with their
owning libraries; the legacy test-support source mappings remain planned. Normal
production builds/exports do not depend on Catch2 or Test Support. Both new members
were registered through GWZ, without manually editing workspace configuration.

Evidence on Apple Clang 21 / arm64 macOS / C++23:

- 27 CTest cases pass: Base 4, Time 3, Core 10 and Peripherals 10. The new peripheral
  cases execute actual wrappers and pollers with recorded GPIO operations and a
  controlled clock, including both open-drain variants, wraparound, cyclic/finite
  sequences, reset/catch-up and a composed blinking application.
- Six peripheral public headers compile independently. Production input/output
  wrappers accept separate pins and reject the same pin with Core's expected
  resource-conflict diagnostic. Core's 6 valid / 19 invalid application probes and
  2 valid / 3 invalid parameter-index probes still pass.
- Separate source copies of all four production libraries build and install using
  only installed dependencies, before provision of test dependencies. An installed
  peripheral consumer links and runs without Catch2.
- All four standalone host suites pass offline with installed Test Support/Catch2.
  The workspace also passes with installed Catch2, in addition to local-source
  setup. Package results/logs are under ignored `build/portable-packages/`.
- A Clang raw-token scope check inspects the extracted C++ files, including disabled
  branches. Original source inventory hashes remain unchanged; extraction hashes
  and per-file validation notes are updated in the SQLite ledger.

This validates software GPIO calls and timer logic, not electrical behavior,
register side effects, interrupts or target toolchains. Debounce/button logic,
the separate sequencer, PWM, timer selection and MCU bindings remain later work.

## Internal resource-claim validation — 21 September 2026

Gianni selected this policy: repeated exclusive resources and conflicting ranges
are errors, including within one module's own claim list. Compatible explicit
shared-use claims remain allowed. `SelfModuleParamsConflictTest<Param>` now runs
`SelfParamsConflictTest` on the parameter's resources instead of returning false
unconditionally. This closes the lone-parameter gap without changing conflict
predicates or dependency-module deduplication.

The new duplicate-resource regression compiled before the fix and now fails with
the intended diagnostic. Six positive application probes pass and nineteen
negative probes fail as intended. New cases cover lone-parameter duplicate pins,
overlapping/contained/identical ranges, whole-resource versus range in both orders,
incompatible shared settings, nonadjacent duplicate entries and a conflict in a
dependency module. Positive controls retain compatible sharing, adjacent ranges,
different resource types/IDs and empty claim lists. The prior unrelated-parameter
case still fails with the same internal-conflict diagnostic.

The workspace build, parameter indexing probes and all ten Core mock runtime tests
pass on Apple Clang 21. Original Ardoinus sources remain unchanged. Next: adapt the
retained Base/Time tests, then extend portable peripheral mock coverage. Hardware
validation remains on hold.

## Parameter indexing fix — 21 September 2026

`Parameters<...>::Param<N>` now returns the Nth type for positive indices as well
as zero. `ParamByIndex` inherits the next lookup's result and specializes index
zero to return the selected type. The public alias is unchanged. Empty and
out-of-range lookups produce `GREVIR_CORE_PARAMETER_INDEX_OUT_OF_RANGE`.

Ten new static assertions reproduced the old bug and now pass, covering first,
middle and last indices, repeated types, a singleton list, exact type preservation
and the helper itself. Two valid compiler probes pass; three invalid probes
(one past the end, a larger index, and an empty list) fail with the intended
diagnostic. The workspace build, existing application probes and all ten Core
runtime mock tests still pass on Apple Clang 21. Original Ardoinus sources remain
unchanged; the Core module destination hash is updated in the ledger.

At this checkpoint the lone-parameter duplicate-claim gap was next; the subsequent
policy decision and fix are recorded above. Hardware validation stays on hold.

## Core mock validation — 20 September 2026

Gianni authorized host-side behavioral validation and explicitly put hardware
validation on hold. Core now has ten passing Catch2/CTest cases exercising its
production `Application`, `ModuleBase`, `ModuleInstanceBase` and `Singleton`
code through deterministic mock callbacks and state. This is Core coverage, not
completed GPIO, virtual-clock, timer/register, interrupt or AVR/ESP32 mock coverage.

The first run passed seven of nine cases and failed two: a shared dependency could
run after a consumer, including when introduced through a parameter and explicit
root. The legacy closure traversal was not a valid dependency-first callback
order. The new `lifecycle_order.hpp` computes that order for `ModuleRunner`, with
no change to `AllModules` membership or the parameter-before-module phase split.
Independent applications keep their existing reverse module order. Cyclic module
dependencies now fail with `GREVIR_CORE_DEPENDENCY_CYCLE`; two compile-fail cases
cover mutual and self-cycles. This is a focused correctness change after the
initial behavior-preserving extraction.

Evidence, Apple Clang 21 / arm64 macOS / C++23:

- All ten CTest cases pass: empty application, callback phases, dependency chains,
  shared and uneven graphs, parameter dependencies/duplicate roots, persistent
  state, independent identities, cross-translation-unit identity and fixture reset.
- A single-process run in seeded random order passes 35 assertions, checking that
  test isolation does not rely on CTest giving each case a separate process.
- Workspace compilation still passes: eleven Core headers (38 across all three
  libraries), existing compile cases, two valid applications and ten expected
  failures. Two of those failures are the new cycle checks.
- Standalone Core tests pass against installed Base, with a local Catch2 source
  and fetching disabled. Catch2 3.8.1's default download is SHA-256 pinned and
  requires an explicit setup option. Running tests never downloads dependencies.
- Installation contains no Catch2 or test headers. An installed consumer builds
  and executes with the source copies hidden. Ordinary library-only configuration
  works without finding/fetching Catch2.
- Clang raw-token brace checks pass for all 61 extracted/new C++ files, including
  disabled branches. All 736 original source hashes remain unchanged; extraction
  destination hashes and the SQLite integrity checks pass.

Use the [workspace commands](../README.md#host-mock-validation) or
[standalone test instructions](../grevir-core/tests/README.md). Shared presets,
CI/other host OSes and a permanent source-scope lint remain outstanding.

Next mock work: resolve the already documented parameter-index/claim-check gaps
with regressions, adapt the retained Base/Time tests, and add deterministic clock
and GPIO fixtures as portable peripheral code is extracted. Hardware checks stay
on hold. The allocator remains a placeholder.

## Core extraction — 20 September 2026

This section records the earlier compilation-only checkpoint; the behavioral
validation and subsequent callback-order fix are recorded above.

`grevir-core` was created and registered using `gwz repo create grevir-core`.
It supplies `GrevirCore.h`, Arduino library metadata, an independent CMake target
`grevir::core`, install/export rules and the original MIT notice. Its only
production dependency is Base.

Nine owned headers separate module parameters/dependencies, singleton storage,
resource claims, conflict checks, application composition, board inventory traits,
device mappings, resource topology and the legacy allocation placeholder. GPIO,
Arduino services, target selection and register access remain outside Core.
The splits preserve complete declarations and namespaces; the resource graph
retains the historical `ardo::sys::avr::base` spelling without any MCU dependency.
Each new C++ file is below 500 lines.

Necessary boundary repairs were limited to direct includes and replacing the
graph's `setl::has_type_v` register-header dependency with Base's equivalent
`setl::tuple_contained_in<...>::value`. Existing algorithms and diagnostic strings
were retained. `SelectionResolver` remains a pass-through stub, not an allocator.

Validation with Apple Clang 21.0.0, arm64 macOS, C++23 and `HAS_STD_LIB=1`:

- Workspace build passes, including all ten Core public headers independently
  (37 across Base, Time and Core), concrete template users, historical dependency
  assertions and shared/range claim assertions.
- Both historical singleton module styles compile against test-only pin/poller
  bindings. This adapts the useful declarations, not the empty old runtime harness.
- Two valid applications compile. Eight invalid compositions fail with their
  expected static assertions: exclusive resources, same-module parameters,
  overlapping ranges, whole-resource versus range, incompatible shared settings,
  transitive dependency claims, internal claim duplicates and an empty range.
- Separate copies of Base and Core build/install outside the workspace. Core
  resolves only installed Base. A two-translation-unit installed consumer links
  with both copied source trees hidden; its compile commands contain no workspace
  or Ardoinus include paths. The executable was not run.
- A one-off Clang raw-token check passes for all 56 extracted/new C++ files,
  including disabled branches. A disabled-branch fixture verifies detection of
  an unbraced `if`; braced `do`/`while` and `else if` forms are accepted. Permanent
  lint integration and broader legacy-source migration remain deferred.
- All 736 original source hashes still match the inventory. Twelve Core ledger
  assignments now record destination hashes and compile-check evidence.

The inspection also confirmed inherited issues: nonzero `Parameters::Param<N>`
returns an index wrapper, and duplicate resources in a lone parameter of a lone
module escape checking. `RootDependencies` also retains its different treatment
of an empty dependency list at the top level versus nested traversal. These are
documented in [Core's README](../grevir-core/README.md), not silently fixed during
relocation. The negative probes do not establish complete claim-check coverage.
Runtime lifecycle/order, storage identity, fallback compatibility, Arduino and
AVR/ESP32 validation remain outstanding.

Next: make focused Core correctness changes with regression checks for those
confirmed gaps, then continue the planned portable register/peripheral extraction.
Catch2/CTest runtime fixtures and target validation remain separate increments.

## Foundation extraction — 18 September 2026

| Repository | Contents | Validation |
| --- | --- | --- |
| `grevir-base` | Portable types, compatibility wrappers, type/tuple algorithms, buffers, memory-policy contract and diagnostic hook | Native source/header compilation, concrete template instantiations, extracted tuple static assertions, standalone install/export |
| `grevir-time` | Typed time, periods, units and interactive scaling | Native source/header compilation, concrete template instantiations, standalone build against installed Base, install/export |

Both are registered GWZ member repositories with Arduino-style root layouts,
`library.properties`, public entry headers, standalone CMake targets and the
original MIT notice. They are local, uncommitted repositories without published
remotes. The original checkout remains intact; all 736 inventoried source hashes
still match. Source history import and final removal from Ardoinus remain undecided.

Existing namespaces and API spellings are retained. New entry headers are
`GrevirBase.h` and `GrevirTime.h`; owned headers use the planned `grevir/...` paths.
This increment does not modernize template algorithms or rename their symbols.

## Boundary repairs

- Optional and integer helpers now include their direct prerequisites.
- Tuple type operations, tuple iteration/search and embedded test assertions were
  separated using Clang declaration ranges. Production assertions within templates
  remain with their declarations; namespace-scope examples moved into the compile
  test. Every resulting source file is below 500 lines.
- Standard-library configuration no longer implies mock AVR registers. Native
  CMake sets `HAS_STD_LIB=1`; target capability selection remains pending.
- Portable diagnostics are separated from the memory policy. `System` is an
  explicitly sequential-use, no-op default. The AVR `_NOP()` binding is not part
  of this portable extraction; interrupt/platform policies need the later backend.
- The compact buffer specialization now uses its `S::MemoryBarrier` parameter,
  matching the full-capacity variant. Compile assertions check both policy types.

The original compatibility fallback bodies are retained, including disabled
branches; they have not passed a no-standard-library build or capability audit.
Legacy literal-operator warnings remain. The duplicate PICOS/NANOS enum values
are recorded in Grevir Time's README for a separate correctness change.

## Evidence and scope

Apple Clang 21.0.0 on arm64 macOS, C++23, using the installed CMake/Make executor:

- Workspace build: 27 public headers compiled independently, two template-usage
  translation units, the extracted tuple assertions, and the diagnostic source.
- Separate copies of Base and Time build outside the workspace. Time resolves
  only the installed Base package. An installed-package consumer compiles and
  links successfully through `grevir::time`, including the diagnostic symbol.
- No original Ardoinus include path, Arduino installation, target identity macro,
  downloaded test dependency or MCU compiler is used by these builds.
- A Clang raw-token scope check covered all 38 relocated/new C++ files, including
  disabled branches, with no unbraced control bodies found. This is a source check,
  not evidence that disabled branches compile. A permanent lint rule is deferred.

Seven historical test files have been retained with relocated production includes,
but are not compiled or run: their harness dependencies await adaptation. No runtime
test framework or target/Arduino validation was added. Build commands are in the
workspace and library READMEs; the earlier legacy compile baseline remains separate.

## Queryable status

`GrevirFileMap.sqlite` now has an `extractions` table and `migration_status` view.
The existing ownership mapping is unchanged. Statuses distinguish files checked
by native compilation, retained tests awaiting their harness, and planned work.
Destination hashes capture the extracted copies at this checkpoint; the original
source hashes remain untouched. New packaging/build files are not legacy mappings.

```sql
SELECT repository, status, count(*) AS assignments
FROM migration_status
GROUP BY repository, status
ORDER BY repository, status;
```

At the foundation checkpoint the next extraction was `grevir-core`; its first
increment is recorded above. MCU backends, Arduino adapters, runtime fixtures and
the other package extractions remain later increments.
