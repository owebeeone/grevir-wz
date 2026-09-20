# Ardoinus — ecosystem catch-up

Research date: 18 September 2026.

Purpose: answer Gianni's question about what has happened since the earlier
Ardoinus work, before choosing a revival direction. This is a research and
discussion record, not an implementation plan or a decision to adopt another
framework. It supplements [the state-of-play notes](ArdoStateOfPlayAug26.md).

## What period are we comparing?

The local Git history supplies several useful reference points:

| Date | Local evidence |
| --- | --- |
| 24 September 2018 | Initial commits; `0331e96` begins the recorded history |
| 2019 and 2021 | Further substantial development activity |
| March 2023 | Register and timer development, including PWM and board definitions |
| 28 March 2023 | `f891314`: parameterized OCR registers for timers with more OCR registers |
| May 2025 | Build, module, and example updates |
| 21 May 2025 | Latest recorded commit, `34f1530`, adding the ParkLights example |

This is roughly eight years since the initial implementation, three and a half
years since the major timer/register work, and sixteen months since the latest
commit. These dates describe the checkout, not Gianni's recollection of his last
sustained work. The relevant review therefore spans 2018–2026, emphasizing
developments since March 2023. Projects listed below were not all invented
during that period; some are older peers that remain useful comparisons.

## Main assessment

The research does not establish that an existing project replaces Ardoinus's
whole combination of declarative composition, resource claims, register-field
semantics, and Arduino integration. It does establish substantial overlap in
individual layers. A reasonable direction to discuss is preserving that
combination while evaluating existing utility libraries and hardware data
pipelines before maintaining equivalents ourselves.

This is an architectural assessment from documentation and source inspection,
not a benchmark, adoption survey, proof of uniqueness, or compatibility test.

## C++ makes the implementation choices different

C++17 offers fold expressions, `if constexpr`, and inline variables. C++20 adds
concepts, `requires`, `consteval`, and structural class values as non-type
template arguments. GCC 16 also implements experimental C++26 facilities,
including reflection behind an explicit option. These capabilities differ by
compiler version. [GCC feature matrix](https://gcc.gnu.org/projects/cxx-status.html)

For Ardoinus, the possible payoffs are simpler module traversal, clearer
diagnostics for malformed module/field declarations, and configuration values
that can be checked during compilation. Some recursive template machinery might
be expressed as ordinary constant-evaluated computation. These are opportunities
to measure, not reasons to rewrite every template or assume lower compile costs.

Reflection is worth watching, but it cannot supply missing hardware facts such
as register side effects or clock topology. It should not determine the first
supported baseline while toolchain and library availability remain unsettled.

The earlier compiler research still matters: current Arduino AVR packaging
retains GCC 7.3 and a C++11 default, whereas recent independent AVR and Espressif
toolchains are much newer. See the version table and dated sources in the
[state-of-play document](ArdoStateOfPlayAug26.md#compiler-research--checked-18-september-2026).
Latest compiler capability and stock Arduino installation compatibility remain
different commitments.

## Comparable projects and ideas

| Project | Relevant overlap | Significance for Ardoinus |
| --- | --- | --- |
| Kvasir::Register | Typed register fields, static checks, deferred register actions, generated device descriptions | Compare field operations and register-access semantics |
| modm | Generated C++ HALs, device data, compile-time checking, embedded utilities | Closest broad C++ comparison in this survey |
| Embedded Template Library (ETL) | Fixed-capacity containers and utilities without an STL dependency | Evaluate overlap with `setl` before expanding it |
| svd2rust and esp-hal | Generated typed registers and Rust hardware abstractions | Learn from their hardware-access and ownership models |
| Zephyr devicetree | Declarative hardware description and configuration | Compare the boundary between hardware data and application composition |

### Kvasir is relevant prior art, not a new discovery in the ecosystem

Kvasir describes fields using types and enums and combines deferred operations
through `apply()`. Its documented model closely overlaps with Ardoinus's typed
bit groups. The inspected upstream default branch's last commit was dated
29 December 2017, so it is an older reference rather than evidence of recent
maintenance. Other forks were not audited.
[Project documentation](https://github.com/kvasir-io/Kvasir),
[default-branch commit query](https://api.github.com/repos/kvasir-io/Kvasir/commits?per_page=1).

The useful comparison is precise: what does each API guarantee about field
values, combining writes, reserved bits, testing access, and the resulting
instructions? Similar intent does not establish interchangeable behavior.

### modm is a substantive current comparison

The current modm tree describes C++23 library generation for AVR and ARM
Cortex-M, modular HAL generation, and a partial AVR libstdc++ implementation.
Its default branch showed activity in August 2026. It is broader than a
register-access library. Its documented target list does not establish an
ESP32 replacement for Ardoinus.
[modm](https://github.com/modm-io/modm).

This changes the discussion from whether modern C++ on AVR is plausible to
which support and integration costs Ardoinus should own. An Arduino-compatible
library layered into existing sketches is still a separate integration goal
from adopting a generated platform library and its development workflow.

### ETL addresses the utility layer

ETL provides fixed-capacity containers and embedded utilities without requiring
the STL or heap allocation. It predates Ardoinus and remains maintained; its
repository describes development since 2014.
[ETL documentation](https://github.com/ETLCPP/etl).

That makes it a candidate for a focused comparison with buffers, optional
values, traits, and related utilities. It does not automatically replace
Ardoinus's time semantics, resource claims, or module graph. Target compiler
compatibility, code size, and behavior must be checked before adopting anything.

### Embedded Rust is now directly relevant on ESP32

`svd2rust` generates typed register maps from SVD descriptions. Espressif's
`esp-hal` supplies a bare-metal Rust HAL with blocking and asynchronous driver
interfaces, including Xtensa ESP32, S2, and S3 targets. Espressif publishes a
Rust development guide and distinguishes stabilized APIs from unstable ones.
[svd2rust](https://docs.rs/svd2rust/latest/svd2rust/),
[esp-hal](https://github.com/esp-rs/esp-hal),
[Espressif Rust guide](https://docs.espressif.com/projects/rust/book/).

This is a useful comparison for modeling peripheral ownership and constraining
operations through types. It is not a proposal to change Ardoinus's language:
retaining Arduino C++ libraries and sketches remains part of the stated scope.

### Declarative hardware configuration has other established forms

Zephyr uses devicetree for hardware description and configuration, with generated
definitions accessed through C macros. Its mechanism differs from the desired
namespaced C++ API, but its separation of board description from driver logic
is instructive. [Zephyr documentation](https://docs.zephyrproject.org/latest/build/dts/intro-scope-purpose.html).

## Hardware data generation deserves another look

Microchip publishes device packs with ATDF data, including register and bit-field
descriptions and subsequent corrections. These sources predate this review;
their existence is not claimed as a recent invention.
[Microchip packs and change history](https://packs.download.microchip.com/).

`modm-data` now extracts hardware descriptions from vendor sources, including
device packs and technical documentation. Its associated research on extracting
hardware data from PDF documentation was published in October 2023, after the
March 2023 Ardoinus timer work. The older `modm-devices` repository directs users
toward `modm-data`.
[modm-data](https://github.com/modm-io/modm-data),
[modm-devices](https://github.com/modm-io/modm-devices).

For `avr_api_gen.py`, the question is whether compiler-header extraction should
remain the only input, or whether structured device descriptions can add field
grouping, enumerations, and access semantics. Input coverage and correctness must
be inspected for the chosen devices; structured data is not automatically
complete or correct. The existing distinction between generated facts and the
manually maintained `_dev.h` semantics remains valuable.

An important review criterion is whether the model expresses more than masks
and shifts: read-only fields, write-one-to-clear flags, side effects, reserved
bits, and legal access sequences can require different operations. This is a
criterion for the next audit, not a claim that Ardoinus currently mishandles
those cases or that any generator resolves all of them.

## ESP32 Arduino compatibility changed materially

Arduino-ESP32 3.0 moved from the 2.x generation's ESP-IDF 4.4 base to ESP-IDF 5.1.
The migration changed LEDC, timers, RMT, I2S, and other APIs. It replaced
`ledcSetup`/`ledcAttachPin` with `ledcAttach`; `timerBegin` now takes a frequency.
UART defaults also changed. [Official migration guide](https://docs.espressif.com/projects/arduino-esp32/en/latest/migration_guides/2.x_to_3.0.html).

There is a concrete mismatch to investigate in this checkout:

| ESP32 serial port | Ardoinus's declared GPIO claims | Documented 3.x default RX/TX pins |
| --- | --- | --- |
| UART1 | 9 and 10 | 26 and 27 |
| UART2 | 17 and 16 | 4 and 25 |

The local claims are in
[ardo_sys_esp32.h](../../ardoinus/ardOinus/src/sys/ardo_sys_esp32.h). Pin assignments can be
configured, so neither table column should become a universal hardware truth.
The implication is that claims must follow the actual selected routing. A
successful compile alone would not prove the old claims describe actual use.

Arduino-ESP32 also has a Peripheral Manager that records pin/bus associations
and invokes deinitialization callbacks when replacing an existing association.
This is runtime ownership management, distinct from Ardoinus's compile-time
claim checks. [Version 3.3.11 implementation](https://github.com/espressif/arduino-esp32/blob/3.3.11/cores/esp32/esp32-hal-periman.c).

Integration needs to make these two views of resource use agree. For example,
automatic channel selection cannot be assumed to honor a channel chosen only
inside Ardoinus's type graph. The actual LEDC API provides both automatic
attachment and explicit channel attachment.
[LEDC API](https://docs.espressif.com/projects/arduino-esp32/en/latest/api/ledc.html).

ESP-IDF also documents multicore critical sections using spinlocks: disabling
interrupts on one core alone does not protect a shared resource from the other
core. This was not invented during the hiatus, but it matters when adapting AVR
register-access assumptions to ESP32.
[ESP-IDF FreeRTOS documentation](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/system/freertos_idf.html).

## AVR has continued beyond the classic ATmega model

DxCore supports newer AVR families, including DA, DB, DD, EA, EB, and DU. These
devices broaden the relevant peripheral and routing models beyond classic
ATmega328P assumptions. [DxCore](https://github.com/SpenceKonde/DxCore).

The immediate AVR scope should therefore name concrete devices. Supporting the
existing ATmega328P implementation first does not establish modern AVR-family
support, even when generated constants exist for those parts.

## Reproducible Arduino builds have better support

Arduino CLI's sketch project profiles can identify boards, platform versions,
and library dependencies, including local library directories. That provides a
way to specify integration builds without making the GWZ workspace itself the
Arduino package format.
[Arduino sketch profiles](https://docs.arduino.cc/arduino-cli/sketch-project-file/).

For this project, useful evidence would include expected-success sketches,
expected compile failures for conflicting claims, and generated-instruction and
size comparisons for representative register operations. These are proposed
validation criteria, not tests run during this survey.

## What this means before choosing the next direction

My assessment is that the combination worth evaluating is:

> Declarative C++ application composition, typed hardware operations, and
> explicit resource checking, usable with Arduino on small AVR devices and
> Xtensa ESP32.

Comparable projects reduce the justification for rebuilding every supporting
layer independently. They do not, from the evidence reviewed, establish that
this exact combination is already available as a compatible replacement.

The decisions informed by this review are whether to retain stock AVR compiler
compatibility, which utility/data components to reuse, and where static claims
meet framework-managed resources. No dependencies were installed, no external
framework was built, and no target hardware was tested for this review.

## Focused follow-up: compile-time resource collisions

Gianni specifically asked whether other systems already turn resource collisions
into compile-time errors. **Yes, several do, with different scopes and models.**
This is more specific than register-field type checking and should be compared
separately from it.

| System | Verified static checking | Boundary of the comparison |
| --- | --- | --- |
| Ardoinus | Checks claims across the declared module closure: exclusive resources, overlapping ranges, and incompatible shared configurations | Covers declared claims, not arbitrary external code's hidden hardware use |
| RTIC | Assigning the same task-local resource to multiple tasks produces a compile-time error; task access is declared | Task/resource ownership and synchronization, not demonstrated timer-frequency or address-range constraint solving |
| Rust peripheral ownership, including Embassy-style HALs | Moving or exclusively borrowing a unique peripheral token prevents conflicting safe-code use | Initial singleton acquisition may be checked at runtime; ownership checks thereafter are compile-time; unsafe escape hatches can bypass the model |
| modm | Rejects invalid pin/peripheral connections and incompatible pin remap groups at compile time | The documented checks are configuration/group checks; they do not establish a whole-application scan of independent module claims |
| Kvasir::Register | Statically checks register/field operations | The reviewed documentation does not establish application-wide peripheral allocation checking |
| Arduino-ESP32 Peripheral Manager | Tracks and changes pin/bus associations | Runtime mechanism, so not an example of compile-time collision rejection |

RTIC's resource documentation explicitly identifies duplicate assignment of a
`local` resource as a compile-time error. It also supports declared shared
resources with controlled access. This is a concrete affirmative answer, even
though the model is different from Ardoinus's configuration claims.
[RTIC resource documentation](https://rtic.rs/2/book/en/by-example/resources.html).

Rust's embedded singleton pattern separates obtaining a peripheral once from
subsequent ownership checking. A driver that consumes a non-copyable peripheral
token prevents a second driver from consuming that same token while it remains
moved. Exclusive borrowing similarly restricts overlapping use. This is a
compile-time guarantee within the safe API, not proof that all singleton
initialization is checked during compilation.
[Embedded Rust singletons](https://doc.rust-lang.org/stable/embedded-book/peripherals/singletons.html),
[Embassy peripheral contract](https://docs.embassy.dev/embassy-stm32/0.2.0/stm32g041k8/trait.Peripheral.html).

modm's documentation gives a concrete conflicting-remap example: pairing
`GpioA9::Tx` with `GpioB7::Rx` for USART1 is rejected because the pair combines
incompatible remap groups. That is real C++ compile-time conflict checking.
It should not be generalized into a guarantee that unrelated configuration
calls cannot reuse a pin; that broader guarantee was not established here.
[modm explanation](https://modm.io/how-modm-works/#modm-asserts-at-compile-time),
[documentation source inspected](https://github.com/modm-io/modm/blob/develop/docs/src/how-modm-works.md).

Ardoinus's interesting comparison point is the combination of a declarative
module graph and extensible claim relationships. Equal shared configurations
can be allowed while unequal ones conflict; ranges can conflict without having
identical types. The focused search did not establish an equivalent complete
C++/Arduino claim system, but it also does not prove uniqueness or novelty.
No comparison project was compiled or tested for this follow-up.
