# Grevir — standalone packaging plan

Recorded: 18 September 2026.

Status: proposed implementation plan. No package extraction, build-system change,
registry publication, or Git operation has been performed by writing this plan.
Grevir is the [selected project name](README.md). Individual package names,
exact repository boundaries, the C++ baseline, and the first ESP32
architecture remain decisions to resolve in the relevant steps.

## Objective

Make small, useful subsets of the Ardoinus-derived Grevir project independently
installable and usable with only their real dependencies. This is Gianni's
primary reason for the GWZ-managed multi-repository workspace, `grevir-wz`.

A developer should be able to consume utilities, register abstractions, module
composition, a device library, or a host generator without adopting unrelated
parts of Ardoinus. A complete application can compose those packages while
retaining compile-time resource checking and whole-application optimization.

The audience is willing to use modern C++ and tooling. Packaging should make the
supported environment reproducible. Arduino integration is required; universal
compatibility with old compilers has not been selected as a requirement.

## Starting point and scope

The original Ardoinus monorepo contains five Arduino library directories: `ardOinus`,
`ardOQuadEncoder`, `ardOStepper`, `ardOFastLED`, and `ardOnet`. Each has
`library.properties`; the inspected manifests lack declared library dependencies.
The README and core metadata disagree about C++11 versus C++14. No CMake package,
PlatformIO manifest, Conan recipe, vcpkg manifest, or Python package definition
was found in the packaging survey.

The core mixes utilities, compatibility headers, composition, register machinery,
MCU definitions, and platform integration. For example, `setl_bit_fields.h`
includes `setl_system.h`; its portability boundary needs examination before it
can be advertised as an independent generic package.

Preserve both [avr_api_gen.py](avr_api_gen.py), now in this workspace's `dev-docs/`,
and [ardoExtract/extract.py](../../ardoinus/ardoExtract/extract.py), in the original
Ardoinus checkout, while their relationship is examined. Ship existing generated definitions where
appropriate
without requiring consumers to regenerate them. The Ardoinus root has both MIT and LGPL
license files; resolve ownership and notices per component before publication.

This plan covers dependency boundaries, artifacts, build integration, delivery,
version compatibility, and consumer validation. New peripheral allocation and
interrupt APIs remain separate work. Their completion is not a prerequisite for
shipping the first useful independent component.

## Contract for every package

- One coherent public purpose, documented headers or commands, and a package
  identity mapped to its names in each supported registry.
- Explicit target-library, host-tool, and development-only dependencies. Optional
  integrations must not become unconditional dependencies of lower layers.
- Declared language and library requirements, plus tested MCU/board, core/SDK,
  compiler, and build settings where relevant.
- Source and template definitions needed by consumer compilation. Pure header
  components use interface targets; compiled components offer source builds.
  Prebuilt objects must not be the sole route to target code.
- An artifact containing public files, necessary generated data, notices, and a
  minimal consumer example, with no absolute paths into a contributor checkout.
- An isolated consumer build using only the artifact and declared dependencies,
  with the GWZ workspace and sibling checkouts unavailable.

Shared dependencies must resolve to one compatible version within an application.
Keep the `setl`/`setlx` configuration coherent across translation units; packages
must not independently select incompatible fallback and standard-library types.

Whole-application optimization remains a consumer build property. Propagate
necessary include paths and language requirements through package targets without
forcing global compiler flags. Test optimization/LTO settings where supported,
including interactions with SDK objects; compare packaged and direct-source builds.

## Candidate boundaries

The later [repository and C++ API extraction plan](GrevirRepositoryPlan.md) now
provides concrete proposed package names, dependencies, per-file ownership and an
Arduino-compatible checkout layout. Its [queryable file map](GrevirFileMap.sqlite)
refines the logical candidates below.

These are logical responsibilities to audit, not reserved names or a requirement
for one repository per row.

| Component | Independent use | Dependencies to retain or investigate |
| --- | --- | --- |
| Utilities and compatibility support | Use a useful utility subset in an ordinary embedded C++ program | Necessary utility headers and a coherent standard-library policy |
| Register and bit-field machinery | Express typed fields without the application framework | Utilities and explicit target access/barrier policy where needed |
| Composition and resource claims | Compose modules and check declared resources | Utilities and backend contracts, without requiring every MCU implementation |
| MCU facts and peripheral backends | Use the selected target's declarations and configuration | Register machinery, required contracts, and target toolchain/SDK |
| Arduino integration | Connect to Arduino lifecycle and board support | Selected backend and relevant core/composition facilities |
| Encoder, stepper, FastLED integration, networking | Install a particular reusable capability | Actual common facilities and external libraries; separable platform adapters where useful |
| Host generators | Produce hardware facts and future binding glue if selected | Python dependencies and explicitly located tools/input data |

Audit includes and required symbols before finalizing boundaries. Keep mutually
dependent facilities together until their interface is understood. Avoid one
package per header; choose MCU data granularity that balances useful subsets and
release maintenance. An optional convenience package can assemble a tested set,
while lower layers remain usable without it.

Prove the contract using staged distribution artifacts before moving repositories.
Repository extraction and history handling belong to the revamp work; the proven
package layout can then move into its selected repository without changing the
consumer contract.

## Proposed delivery strategy

| Channel | Role | Priority |
| --- | --- | --- |
| Source archives and installable CMake targets | Common source distribution; consumption through find_package, add_subdirectory, or FetchContent/CPM | Proposed foundation |
| Arduino library format | Independent libraries, dependencies, and IDE examples | Required ecosystem integration |
| PlatformIO packages | Embedded delivery with framework/platform/build metadata | Proposed first additional channel |
| PyPI | Host generators and CLI tools, optionally containing data or headers | Proposed tooling channel |
| ESP Component Registry | ESP-IDF-native components | Add with validated ESP32/ESP-IDF support |
| Conan and vcpkg | General C++ consumers using their established package managers | Optional after the source contract is proven and there is consumer demand |

Arduino Library Manager requires `library.properties` at the repository root,
can install declared library dependencies, and does not distribute submodule
contents. Published library repositories must satisfy those constraints.
[Arduino library specification](https://docs.arduino.cc/arduino-cli/library-specification/),
[registry requirements](https://github.com/arduino/library-registry/blob/main/FAQ.md).

PlatformIO supports library manifests and dependencies; validate the packed files
and their actual build integration. CMake targets should be relocatable and usable
without the source checkout. Leave source acquisition under consumer control
rather than unconditionally fetching dependencies inside every library.
[PlatformIO packaging](https://docs.platformio.org/en/latest/librarymanager/creating.html),
[CMake exports](https://cmake.org/cmake/help/latest/guide/importing-exporting/index.html).

## Toolchain and generation delivery

Record exact supported compiler/core configurations. Evaluate language support,
standard headers, and SDK compatibility separately. New language features are
acceptable where justified; diagnose unsupported configurations clearly.

If stock Arduino board packages cannot supply the chosen compiler or flags,
provide a reproducible installation route. Investigate a Boards Manager
platform/tool package or another documented integration before committing to it.
A normal library archive must not be assumed to replace the board compiler or
provide arbitrary build hooks; those facilities belong to platform configuration.
[Arduino platform specification](https://docs.arduino.cc/arduino-cli/platform-specification/).

Ship generated device definitions with target packages, recording generator and
input versions. Preserve manual `_dev.h` semantics. Application-specific generation,
if selected later, needs a demonstrated invocation in each affected build system.
Prototype that before advertising the feature; an Arduino library manifest cannot
be assumed to provide PlatformIO-style script integration.

Python wheels can include non-Python files. If a PyPI package exposes C++ assets,
provide stable asset discovery and build metadata; installing the Python package
alone does not configure a target C++ build or supply an AVR/ESP32 compiler.
[Python package data](https://setuptools.pypa.io/en/latest/userguide/datafiles.html).

## Phases and steps

Each phase is a usable milestone. Each step has one goal and an aspirational
budget below 500 changed lines, including focused checks and documentation. These
are estimates, not measured work. Split steps that grow beyond the target.
Mechanical moves or generated output may exceed it: report actual size separately
and keep semantic changes reviewable. Repeat package steps per component rather
than hiding a repository-wide extraction inside one step.

### Phase 1 — First independently consumable source package

Milestone: one useful utility subset can be unpacked, installed, and consumed
outside Ardoinus using only its declared dependencies.

| Step | Goal and completion evidence | Aspirational LOC | Depends on |
| --- | --- | --- | --- |
| 1.1 | Define the pilot contract: audited dependencies, public surface, name mapping, language/library baseline, license ownership, and supported consumer configuration. | 100–250 | Source survey |
| 1.2 | Isolate the pilot subset; a direct-source consumer builds without the umbrella application header or unrelated platform SDKs. | 200–450 | 1.1 |
| 1.3 | Produce an installable, relocatable CMake target and source archive; allow Arduino layout without maintaining a second implementation. | 100–250 | 1.2 |
| 1.4 | Prove artifact independence in an isolated consumer job, including relocating the install prefix and denying workspace access. | 100–250 | 1.3 |

Choose a genuinely useful leaf from the audit. The register header is not
automatically the simplest pilot. Check registry name availability before fixing
names; no name reservation is implied by this plan.

### Phase 2 — An AVR application assembled from separate packages

Milestone: a small AVR application uses independently packaged foundations,
hardware support, and one reusable module. Lower layers remain usable alone.

| Step | Goal and completion evidence | Aspirational LOC | Depends on |
| --- | --- | --- | --- |
| 2.1 | Establish the register package boundary, including target access/barrier policy; its consumer builds without application composition. | 200–450 | Phase 1 |
| 2.2 | Establish the composition/claims package boundary; a consumer demonstrates dependency sharing and an expected collision diagnostic. | 200–450 | Phase 1 |
| 2.3 | Package the selected ATmega328P slice with required generated and manual files; a supported AVR consumer builds a representative configuration. | 200–450 plus reported data moves | 2.1 and AVR toolchain selection |
| 2.4 | Package the Arduino integration for that slice; a sketch resolves its board/backend and lifecycle through declared dependencies. | 200–450 | 2.2, 2.3 |
| 2.5 | Package one existing device module independently; its example builds from artifacts and exercises its declared dependencies. | 150–400 | Relevant contracts above |

Extend to other useful subsets by repeating this bounded process. Fix only the
baseline defects needed to demonstrate each package and record broader restoration
work separately. Full peripheral coverage and automatic allocation are separate
milestones; packaging can demonstrate existing explicit resource binding first.

### Phase 3 — Arduino and PlatformIO installation-ready artifacts

Milestone: the selected packages install and build in both supported environments
without GWZ or sibling checkouts.

| Step | Goal and completion evidence | Aspirational LOC | Depends on |
| --- | --- | --- | --- |
| 3.1 | Produce valid Arduino artifacts for one selected package and its dependency closure, with correct metadata, public headers, and example discovery. | 100–300 per package | Phase 2 contracts |
| 3.2 | Produce equivalent PlatformIO artifacts and verify framework/platform/dependency selection from the packed files. | 100–300 per package | Phase 2 contracts |
| 3.3 | Verify clean installation and compilation in each environment with pinned board/core/compiler configurations and representative behavior. | 150–350 | 3.1, 3.2 |
| 3.4 | Deliver a first-use guide covering subset selection, supported toolchain installation, building, dependencies, and support limits. | 100–250 | 3.3 |

Validate local packages and registry-ready metadata here; public discovery follows
through release operations. Include a deliberate resource-conflict build for the
composition package. Compare flash/RAM and relevant optimized output with the same
source and settings built directly; investigate packaging-induced differences.

### Phase 4 — Independently installable host generator

Milestone: a Python wheel and source distribution install the selected AVR
generator entry point and required data in a clean environment.

| Step | Goal and completion evidence | Aspirational LOC | Depends on |
| --- | --- | --- | --- |
| 4.1 | Resolve the two generators' relationship and ownership, selecting an initial entry point with a defined input/output contract while preserving the originals. | 100–250 | 1.1 contract pattern |
| 4.2 | Package that entry point with dependencies, data, and explicit external-tool discovery, independent of the original checkout location. | 150–400 | 4.1 |
| 4.3 | Demonstrate repeatable generation on bounded fixture inputs from installed wheel and source distribution, with provenance and output compatibility recorded. | 150–350 | 4.2 |
| 4.4 | Integrate the installed tool into one MCU data package's maintainer workflow, preserving manual files and ordinary use of shipped headers without Python. | 150–350 | 4.3, 2.3 |

The host OS runs the generator; locate target tools and vendor inputs explicitly.
The build must not try to run an MCU executable as a host tool. Regenerating the
entire historical AVR database is not required for this milestone.

### Phase 5 — Reproducible releases and workspace integration

Milestone: selected components have validated release candidates and a GWZ mapping
of compatible repositories/revisions, while remaining independently consumable.

| Step | Goal and completion evidence | Aspirational LOC | Depends on |
| --- | --- | --- | --- |
| 5.1 | Establish per-component version authority and cross-channel metadata validation; supported ranges and exact integration pins come from a tested compatibility set. | 150–350 | Package contracts and channel metadata |
| 5.2 | Produce an artifact manifest tying each package/version to its source revision, contents, generated-data provenance, and supported configurations. | 150–350 | 5.1 and applicable Phase 3/4 artifacts |
| 5.3 | Map selected extracted repositories and compatible revisions into GWZ while confirming that consumers build without the workspace. | 100–300 | Phase 2 and repository layout from revamp work |
| 5.4 | Rehearse release preparation from a clean checkout/cache and document channel publication and installation checks for the exact candidate artifacts. | 150–350 | 5.2, 5.3 |

Use independent component versions and a tested compatibility record. Changing
one package need not force every package to release. Define breaking header/API,
compiler-baseline, and generated-schema changes. Examples and CI pin exact resolved
versions; registry ranges express only supported combinations. Check each resolver's
actual behavior instead of assuming identical range or multi-version semantics.

Select one authoritative location per package for version and dependency facts;
validate or generate other manifests from it. Channel archives may have different
layouts while representing the same component revision. This coordination does
not require inventing another dependency solver or package manager.

Published versions and released tags remain immutable. Release preparation does
not authorize tagging, pushes, repository migration, or publication; those remain
the explicitly requested operations. This plan requires no registry credentials.

### Phase 6 — ESP-IDF and optional general C++ delivery

Milestone: additional consumers obtain the same components through their existing
ecosystems. Each selected channel is independently deliverable and does not delay
the initial Arduino/PlatformIO release.

| Step | Goal and completion evidence | Aspirational LOC | Depends on |
| --- | --- | --- | --- |
| 6.1 | Validate one portable package/API consumer on the chosen ESP32 device and architecture with an explicit SDK/toolchain configuration. | 150–350 | Phase 1 and sufficient backend support |
| 6.2 | Package that integration as an ESP-IDF component with declared dependencies and a clean consumer build. | 150–350 per package | 6.1 and Phase 5 metadata contract |
| 6.3 | Add a Conan recipe for one useful portable component, separating host/build requirements appropriately and including a consumer test. | 150–350 | Phase 1 and demonstrated consumer need |
| 6.4 | Add a vcpkg port for one useful portable component and supported configuration; prove bare-metal integration separately before claiming it. | 150–350 | Phase 1 and demonstrated consumer need |

Step 6.1 is an early portability probe and may run during Phase 2. Decide Xtensa
versus RISC-V before that experiment. AVR remains the first complete peripheral
target while early ESP32 work tests the shared API. Full ESP32 backend development
is tracked separately; its cost is not hidden inside these packaging steps.

ESP-IDF has a component manager and registry. Conan supports cross-compilation
profiles, and vcpkg supports custom toolchain integration. Recipes should reuse
the component's source/build contract.
[ESP-IDF](https://docs.espressif.com/projects/idf-component-manager/en/latest/),
[Conan](https://docs.conan.io/2/tutorial/consuming_packages.html),
[vcpkg](https://learn.microsoft.com/en-us/vcpkg/users/triplets).

## Independent work and coordination

- After Phase 1, register isolation (2.1), composition isolation (2.2), host-tool
  packaging (Phase 4), and a suitable ESP32 probe (6.1) can proceed independently.
- Arduino and PlatformIO adapters (3.1 and 3.2) can proceed separately once package
  identities, source contents, and dependency contracts are fixed.
- Keep recipe-only changes separate from source-boundary changes. Coordinate
  shared metadata changes before dependent adapters consume them.
- Optional Conan/vcpkg work can proceed once its source contract is stable. This
  structure permits parallel work without requiring multiple agents.

## Completion evidence and decisions

The initial packaging effort is complete when the selected component set has:

1. Isolated artifact consumers, including a lower-layer-only consumer, using just
   declared dependencies and no workspace access.
2. Arduino and PlatformIO installation/build evidence for the selected AVR setup,
   with examples, dependency resolution, and relevant failure diagnostics checked.
3. An installed generator fixture working from both Python artifact forms, with
   external inputs/tools stated and manually maintained files preserved.
4. A version/support matrix and clean release rehearsal preserving source/template
   visibility, with size and optimization comparisons recorded.
5. A repository/workspace mapping that remains optional for consumers.

Record physical-board validation separately from compilation. Packaging success
does not establish functional support for an untested peripheral. Retest affected
packages and consumers after changes; expand checks for changed shared dependencies
or unresolved regressions rather than repeatedly running an unchanged full matrix.

Close decisions as their steps begin: pilot subset and names; license ownership;
language and standard-library requirements; supported host OS/toolchain delivery;
MCU data granularity; whether PyPI exposes C++ assets as well as tooling/data; the
first ESP32 architecture; and which optional registries have actual consumers.
The complete future peripheral API need not be settled to begin packaging.

## Related records

- [Guiding principles](ArdoPrinciples.md)
- [Revamp goals and repository ideas](ArdoPlanIdeas.md)
- [Initial survey and generator notes](ArdoStateOfPlayAug26.md)
- [Ecosystem review](ArdoEcosystemReviewSep26.md)
