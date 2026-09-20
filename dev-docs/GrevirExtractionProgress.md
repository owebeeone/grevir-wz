# Grevir extraction progress

Latest checkpoint: 20 September 2026. The multi-repository migration now includes
Base, Time and the first Core extraction. The remaining repository plan is not yet
implemented. All three members remain local, without published remotes. The
workspace and extracted libraries are being committed as the initial development
baseline on 21 September 2026; the validation checkpoints below retain their dates.

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
