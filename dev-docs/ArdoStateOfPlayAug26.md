# Ardoinus — state of play and discussion notes

Recorded: 18 September 2026. The filename `ArdoStateOfPlayAug26.md` is retained
as requested; the compiler research below is dated September, not August.

This document captures Gianni's direction, the initial repository survey, and
the discussion so far. It is a working record, not an approved migration design
or implementation plan. At the time of the initial survey, no workspace
creation, repository extraction, API change, compiler baseline, or release
operation had been agreed or performed.

Later update, 18 September 2026: **Grevir is the selected project name**,
`grevir-wz` has been initialized, and these development notes have moved into
that workspace. See the [naming decision](README.md).

The [consolidated principles](ArdoPrinciples.md) and [move-forward ideas](ArdoPlanIdeas.md)
record the later discussion, including the refined AVR/ESP32 sequencing, module
instances, whole-application compilation, and boot-selected application question.

## Gianni's direction and historical context

- Ardoinus is an older embedded C++ project: **“declarative to the maxx.”**
- Its original purpose was experimental: demonstrate embedded capabilities
  enabled directly by C++ that standard C does not offer in the same way.
  Gianni considers that hypothesis proven and now wants to **push harder and
  explore how far the approach can go**. This research purpose should guide the
  revamp, alongside restoration and repository extraction; see
  [the purpose and candidate research question](ArdoPlanIdeas.md#purpose-push-the-embedded-c-experiment-further).
- Survey the existing project and discuss its future before restructuring it.
- The development environment is the **GWZ workspace `grevir-wz`**, initially
  proposed as `ardoinus-wz`, with Ardoinus to be broken into subprojects.
- **Arduino compatibility is a requirement.** Its exact installation and
  toolchain guarantees still need to be defined.
- Historical C++ limitations were practical toolchain constraints: Ardoinus
  pushed templates hard while needing to work with compilers that lagged badly
  behind the leading edge.
- Many standard headers were unavailable. Ardoinus therefore implemented the
  subset it needed using the C++ language support that those compilers did
  provide. The compatibility code was intentional, not simply an accidental
  substitute for an available standard library.
- Hardware definitions should be C++ declarations: Gianni's principle is that
  **“#define is extra super bad.”** Namespaces and constant symbols make hardware
  descriptions composable in templates. Declarative register and bit-group
  access is a central part of the concept, not just a convenience wrapper.
- Interrupt support is the major missing capability. Gianni has struggled with
  a pure C++ compiler approach and believes generated C++ may be needed for a
  fully generic interface. This possibility and newer compiler/SDK binding
  options are captured in [the interrupt ideas](ArdoPlanIdeas.md#major-missing-capability-declarative-interrupts).
- The initial hardware focus was AVR and ESP32 Tensilica/Xtensa. The subsequent
  direction is a seamless AVR/ESP32 API, with classic AVR as the first complete
  target and early ESP32 work as needed to validate portability. **Xtensa versus
  RISC-V for the first ESP32 target remains open.**
- The broader target discussion also mentioned ARM and possibly PIC as later
  considerations. This does not imply that ARM is an ESP32 architecture or that
  those later targets are currently supported.

## Interpretation to preserve

The defining idea is declarative application composition and compile-time
reasoning about hardware resources. Templates are the mechanism supporting that
idea, rather than an end in themselves.

The `setl`/`setlx` facilities should be assessed in their embedded context.
Availability of a recent C++ compiler does not by itself establish availability
of the corresponding standard headers, library implementation, or runtime
support. Removing these facilities merely because a desktop implementation
exists would miss their original purpose.

Three compatibility dimensions need independent treatment:

1. **Language support:** template, constant-expression, and other language
   features implemented reliably by the selected compiler.
2. **Library support:** headers and facilities supplied by that toolchain, and
   the subset Ardoinus must supply itself.
3. **Hardware support:** GPIO, timers, interrupts, board mappings, and the
   resources reserved or used by the surrounding framework.

The working interpretation of Arduino compatibility is to retain the Arduino
toolchain, existing libraries, and sketch lifecycle while allowing richer
declarations above them. Direct hardware access remains useful when the
Arduino API cannot express the required configuration. Whether compatibility
must include completely unmodified stock board packages is still open.

## What is in the repository

Surveyed checkout: `/Users/owebeeone/limbo/ardoinus`.

| Component | Observed responsibility |
| --- | --- |
| `ardOinus` | Module composition, resource checking, GPIO, time, embedded utilities, and platform support |
| `ardOQuadEncoder` | Quadrature encoder module and scaling |
| `ardOStepper` | Stepper motor control |
| `ardOFastLED` | FastLED integration |
| `ardOnet` | Packet fragmentation and reassembly |
| `ardoExtract` | Python generation of AVR and Arduino board definitions |
| `dev-docs/avr_api_gen.py` | AVR constants generator specifically identified by Gianni; retained here as a reference tool |

The five library directories already contain `library.properties` files. Their
existing boundaries provide a useful starting point for a workspace split.
`ardoExtract` is a development tool rather than an Arduino runtime library.

### Declarative composition and resource checking

- `Application` expands the closure of module dependencies, checks resource
  claims, and dispatches setup and loop calls.
- Modules can supply static behavior or use the singleton instance mechanism.
- The resource model includes exclusive claims, overlapping range claims, and
  shared claims whose configurations must match.
- Resource claims can represent GPIO, serial ports, timers, and other finite
  resources. Range claims also suit address ranges such as EEPROM storage.
- The timer API expresses requirements such as frequency, resolution, and pins,
  then provides machinery to select suitable hardware.
- Examples retain Arduino's `setup()` and `loop()` entry points and forward
  them to the declared application.

Compile-time conflict checking covers resources that have been declared.
Third-party libraries and Arduino core services need accurate resource models
or adapters if their usage is to participate in those checks.

Relevant source references:

- [Application and GPIO API](../../ardoinus/ardOinus/src/ardOinus.h)
- [Module parameters, runners, and dependency closure](../../ardoinus/ardOinus/src/ardo_params_modules.h)
- [Resource claims](../../ardoinus/ardOinus/src/ardo_resources.h)
- [Timer abstraction and selection](../../ardoinus/ardOinus/src/ardo_timers.h)
- [Declarative blink example](../../ardoinus/ardOinus/examples/Basics/ArdoBetterBlink/ArdoBetterBlink.ino)

### AVR constants and declarative register access

Gianni identified [avr_api_gen.py](avr_api_gen.py) as the tool used to generate
the AVR constants. It emits declarations in the form
`constexpr unsigned cc{macro_name} = {value};` within the MCU namespace.
Gianni's illustrative example is:

```cpp
constexpr unsigned ccCHR9 = 2;
```

The principle is to expose hardware facts as namespaced C++ symbols that can
participate directly in template declarations. Although a numeric macro can
also expand inside a template argument, it has neither C++ namespace scope nor
its own typed declaration. The intended API therefore represents constants,
registers, and fields through C++ declarations rather than propagating the
vendor headers' macro-based interface into application code.

The first fleshing out of this concept is
[ardo_supplemental_atmega328p_dev.h](../../ardoinus/ardOinus/src/sys/mcu/avr/ardo_supplemental_atmega328p_dev.h).
The file combines generated constants with enums, semantic field types,
register descriptions, accessors, and timer configuration metadata. For example,
it declares an output-compare field as:

```cpp
using BitsCOM0A = setl::BitsRW<
  setl::SemanticType<setl::hash("COM0A"), EnumCOMn>, ccCOM0A1, ccCOM0A0>;
```

This describes the field's meaning, value type, and bit positions. The bit-field
templates handle the masks and left/right shifts needed to encode and decode
values. Callers work with register fields instead of repeating bit manipulation
expressions. The same approach describes fields split across registers, such
as waveform-generation mode bits. Register reads and writes still happen at
runtime; the layout and transformation machinery are expressed in templates.

There are very few `#define` directives in this prototype: the inspected file
has five, comprising its include guard and four declaration helpers. The
architectural intent is a C++ symbol/type API with minimal preprocessor use,
not a claim that every macro has already been removed.

The generated and manually developed layers have distinct ownership:

- [ardo_supplemental_atmega328p.h](../../ardoinus/ardOinus/src/sys/mcu/avr/ardo_supplemental_atmega328p.h)
  contains the regenerable MCU definitions, including namespaced constants.
- The `_dev.h` file explicitly states that it is not regenerated, preserving
  manually developed MCU-specific semantics and register abstractions.
- [setl_bit_fields.h](../../ardoinus/ardOinus/src/setl_bit_fields.h) provides the template
  machinery for field transformations, including mask/shift composition.

Any future extraction should preserve this declarative hardware model and the
boundary between regenerated definitions and manually maintained semantics.
The relationship between the supplied `avr_api_gen.py` and the separately
surveyed `ardoExtract/extract.py` has not yet been audited; neither generator
was run or modified for these notes.

### Size and internal boundaries

The initial line count found approximately 404,000 code lines under `ardOinus`,
of which approximately 385,000 were in the AVR support tree. That tree includes
604 generated supplemental headers. These are source-line counts, including
comments and blank lines, not a measure of handwritten implementation size or
verified device coverage.

Within `ardOinus`, three potential boundaries stand out:

- Embedded utilities and standard-library compatibility facilities.
- Declarative module composition and resource checking.
- Arduino/platform adapters and direct hardware support.

Those are plausible future subprojects, but their current include and type
dependencies need examination before extraction. Generated hardware data also
needs a clear ownership and regeneration story.

## Workspace and packaging ideas discussed

Gianni's potential monorepo-to-GWZ-multi-repo goal is now captured separately in
[ArdoPlanIdeas.md](ArdoPlanIdeas.md). It records possible extraction boundaries
and behavior to preserve, without authorizing repository restructuring.

The initial suggestion was six workspace members corresponding to the five
existing libraries and `ardoExtract`. The subsequently identified
`dev-docs/avr_api_gen.py` is recorded as a reference tool, not an additional
agreed workspace member. This is a proposal, not a settled repository layout.

Potential responsibilities for `grevir-wz` include coordinated member revisions,
cross-library examples, integration checks, and shared development documentation.
Each published Arduino library should remain independently installable with
its dependencies, without requiring its users to install GWZ.

Arduino's packaging rules fit independent library repositories:

- A Library Manager library needs `library.properties` at the repository root.
- Arduino libraries use `src/` and `examples/` layouts.
- Dependencies can be declared by library name through `depends` metadata.
- Library Manager archives do not include Git submodule contents.

Preserving familiar public headers and the existing sketch lifecycle during an
initial split was suggested. Establishing reproducible builds before extracting
the internal foundation, composition, and platform layers would make it easier
to distinguish pre-existing failures from changes introduced by the split.

References:

- [Arduino library specification](https://docs.arduino.cc/arduino-cli/library-specification/)
- [Arduino Library Manager requirements and limitations](https://github.com/arduino/library-registry/blob/main/FAQ.md)

## Restoration and verification observations

The survey found several existing issues or uncertainties. These have not been
fixed, and this list is not a complete correctness audit.

- The README states a C++11 minimum, while the main header and library metadata
  refer to C++14. Some facilities use newer standard-library names with local
  compatibility implementations. The actual supported feature set needs
  compilation evidence rather than inference from these labels.
- Arduino detection partly relies on whether `INPUT_PULLUP` has already been
  defined. That creates an include-order dependency and can select desktop
  mocks when Arduino definitions have not been included yet.
- Board support and generated register definitions are more extensive than the
  available build evidence. `architectures=*` is not proof of universal support.
- Library metadata lacks dependency declarations. Stepper's advertised header
  capitalization differs from its actual filename, and ArdOnet advertises the
  main Ardoinus header rather than its own packet header.
- Build scaffolding is largely older Visual Studio projects and a stale
  Makefile. The main test source also references `tests/modules_test.h`, which
  was not present in the surveyed checkout.
- The root contains an LGPL `LICENSE` and an MIT `LICENSE.txt`, while the README
  identifies MIT. The intended scope of these files needs clarification before
  preparing independent library releases.

Small, syntax-only probes with Apple Clang 21 found:

| Probe | Observed failure |
| --- | --- |
| `setl_system.cpp`, C++14 | `setl::System` is not defined for this host configuration |
| ArdOnet packet test, C++14 | `IPAddress` is used without a declaration available to that translation unit |
| BetterBlink through the host compatibility path, C++14 with `HAS_STD_LIB=1` | Missing `std::move` availability and `std::is_same_v` compatibility, among the first reported errors |

These probes establish specific host-build failures only. They do not establish
that the sketches fail under their original Arduino configurations, and they
are not a substitute for compiling and running on the intended targets.
`arduino-cli` was not on PATH during the survey; no actual AVR or ESP32 Arduino
build, hardware test, size measurement, or performance measurement was completed.

## Compiler research — checked 18 September 2026

The latest available compiler and the compiler installed by a stock Arduino
board package are distinct choices.

| Toolchain | Version verified | Language position |
| --- | --- | --- |
| Upstream GCC, including the AVR target | GCC 16.2 | C++20, extensive C++23 support, and experimental C++26 features; individual features still need checking |
| Ready-made community AVR package inspected | AVR-GCC 16.1.0, Zak Kemble release `v16.1.0-1` | Modern GCC language support; separate from Arduino's packaged compiler |
| Espressif stable Xtensa toolchain | GCC 16.1.0, `esp-16.1.0_20260609` | C++20, extensive C++23 support, and experimental C++26 features |
| Espressif Xtensa prerelease | GCC 16.2.0, `esp-16.2.0_20260914` | Prerelease published 15 September 2026; not the stable Arduino toolchain |
| Arduino AVR Boards 1.8.8 | GCC 7.3.0, `7.3.0-atmel3.6.1-arduino7` | Arduino selects `gnu++11`; C++14 and experimental C++17 support are available through compiler mode selection |
| Espressif Arduino ESP32 3.3.11 | GCC 14.2.0, build `20260121`, package `esp-x32` version `2601` | Includes modern template facilities such as C++20 concepts and substantial C++23 support |

The ESP32 row describes compiler capability, not a verified default language
flag for every Arduino configuration. A supported compiler mode also does not
promise complete standard conformance, all standard-library facilities, or
compatibility with every precompiled framework component.

The important implication is that **AVR hardware does not itself require an old
C++ baseline; retaining Arduino's stock AVR compiler constrains that choice**.
With an updated AVR toolchain, C++20 is a candidate common language baseline for
AVR and Xtensa, subject to testing the features Ardoinus uses. This is an option
raised in discussion, not a baseline decision. The embedded standard-library
compatibility layer may remain necessary after a compiler upgrade.

Research sources:

- [GCC release history](https://gcc.gnu.org/releases.html)
- [GCC C++ feature support](https://gcc.gnu.org/projects/cxx-status.html)
- [GCC 7 release notes, including experimental C++17 support](https://gcc.gnu.org/gcc-7/changes.html)
- [Community AVR-GCC 16.1.0 package](https://github.com/ZakKemble/avr-gcc-build/releases/tag/v16.1.0-1)
- [Espressif stable GCC 16.1.0 release](https://github.com/espressif/crosstool-NG/releases/tag/esp-16.1.0_20260609)
- [Espressif GCC 16.2.0 prerelease](https://github.com/espressif/crosstool-NG/releases/tag/esp-16.2.0_20260914)
- [Arduino package index](https://downloads.arduino.cc/packages/package_index.json)
- [Espressif Arduino package index](https://espressif.github.io/arduino-esp32/package_esp32_index.json)
- [Arduino AVR build configuration](https://github.com/arduino/ArduinoCore-avr/blob/master/platform.txt)

## Questions still open

Before choosing the revival direction, Gianni requested a review of what has
changed since the earlier work. The dated findings, comparisons, and implications
are recorded in [ArdoEcosystemReviewSep26.md](ArdoEcosystemReviewSep26.md).
The local history spans initial development in September 2018, major AVR timer
work in March 2023, and smaller updates through May 2025.

- Must the revived project build using stock Arduino AVR packages without
  changing compiler flags or installing a newer compiler?
- What language baseline and specific template/constant-expression facilities
  do we want to require?
- Which AVR devices and ESP32 architecture (Xtensa or RISC-V), boards, and core
  versions form the initial supported and tested set?
- Should the first workspace split follow the six existing component
  directories exactly, or include an immediate foundation extraction?
- Which compatibility facilities should use native standard headers when
  available, and which need Ardoinus-owned implementations?
- How should resource claims represent Arduino core reservations and
  third-party peripheral use?
- How should generated hardware definitions be reproduced, versioned, and
  validated independently of claims about supported hardware?
- What is the intended licensing scope of the two existing license files?

The useful next evidence discussed was compilation of representative sketches
on specific AVR and selected ESP32 Arduino configurations, covering basic
composition and the more demanding timer/resource machinery. That would inform the
compatibility contract and extraction boundaries before implementation choices
are fixed.
