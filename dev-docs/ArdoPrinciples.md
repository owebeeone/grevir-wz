# Grevir — guiding principles carried forward from Ardoinus

Recorded: 18 September 2026, consolidating the discussion through whole-application
compilation and boot-selected applications.

This is the consolidated record of Gianni's principles and direction. Existing
capabilities, future goals, and unresolved design ideas are distinguished below.
The move-forward ideas and target sequencing remain in [ArdoPlanIdeas.md](ArdoPlanIdeas.md).
This record does not select an implementation API, allocation algorithm, compiler
baseline, or repository layout.

Naming update, 18 September 2026: **Grevir is the selected project name**, and
`grevir-wz` is its initialized GWZ workspace. See the [naming decision](README.md).

## Purpose and experience

Ardoinus began as an experiment showing how C++ can provide embedded programming
interfaces and compile-time guarantees beyond what standard C directly expresses.
Gianni considers the original hypothesis proven and wants to explore how much
further this approach can go. The combination is distinctive; uniqueness is not
an assumption or a requirement for its value.

Gianni's experience was that virtually none of the mistakes he would normally
have made in C appeared in his Ardoinus applications because the recurring
opportunities for those mistakes had been programmed out of the interfaces.
Building the library took substantial work, while application code became
straightforward. This is his reported experience, not a measured comparative
error rate or a claim that every possible application defect is prevented.

## Principles to carry forward

### 1. Put routine hardware work into the library

Application code should declare intended behavior. The library should own the
mundane register arithmetic, masks, shifts, configuration relationships, resource
bookkeeping, and integration boilerplate. Encode that knowledge once and reuse
it through a robust interface. The investment in the framework is justified by
simpler applications and fewer opportunities to repeat low-level mistakes.

### 2. Make safety a property of the interfaces

C++ facilities should make correct usage easy and reject modeled invalid usage.
The value includes removing classes of mistakes through API design. Predictable,
static allocation of application state reduces exposure to dynamic-allocation
failures and related lifetime errors. Preserve this benefit without converting
it into a claim that static allocation prevents every memory error, or imposing
an unagreed ban on allocation inside all third-party/platform code.

### 3. Express hardware through C++ symbols and types

Prefer namespaces, constexpr constants, semantic types, register descriptions,
and bit-group accessors over application-facing preprocessor definitions.
Gianni's example, `constexpr unsigned ccCHR9 = 2;`, illustrates constants usable
directly in template composition. Templates encode field positions and the
required shifts and masks, including fields spread across registers.

[avr_api_gen.py](avr_api_gen.py) is the identified AVR constants generator.
The [ATmega328P development header](../../ardoinus/ardOinus/src/sys/mcu/avr/ardo_supplemental_atmega328p_dev.h)
is the first fleshing out of the declarative register model. Preserve the
boundary between regenerable hardware facts and manually maintained semantics.
Minimal preprocessor use is the intent; platform binding glue may still need it.

### 4. Compute known configuration at compile time; retain useful runtime flexibility

The timer work demonstrates deriving waveform selection, prescaler, TOP count,
and register-field values from a declaration. Fixed configuration should reach
runtime as precomputed values applied to hardware. Frequency and duty-cycle
changes remain available where supported, using the same underlying hardware
model. Static resource allocation and dynamic operating parameters can coexist.
The permitted changes and their effects on sharing still need an explicit contract.

### 5. Compose reusable module instances through their resources and dependencies

A module's template parameters bind the resources allocated to it. This allows
multiple specializations of reusable module code in one application. Declared
dependencies should bring supporting modules into the application automatically,
including dependencies carried by resource parameters. Shared dependencies should
be included once where they represent the same module identity.

The current implementation deduplicates module types and provides static state
per specialization through its singleton mechanism. Distinct resource bindings
can produce distinct module types; repeating an identical specialization does
not create another independent instance. Explicit identity for otherwise identical
instances, and state ownership across applications, remain design questions.
An instance identifier must not bypass exclusive-resource collision checks.

### 6. Reason about resources across the complete application

Application composition includes the transitive dependency closure. Existing
compile-time collision detection covers declared exclusive resources, overlapping
ranges, and incompatible shared configurations. Preserve that capability and
extend it toward allocating hardware from module requirements across the whole
application. Timers, I2C, UARTs, and other peripherals should participate in one
model, including their related pins, clocks, and interrupts.

Allocation must consider requirements together, compatible sharing, and declared
Arduino/core/third-party reservations. A reusable module should be able to request
a capability and receive the concrete resource types selected for its target.
The automatic allocation mechanism remains future work.

### 7. Expose the whole application for compilation and optimization

Gianni's principle is whole-application compilation: host compilers can process
program units far larger than the firmware capacity of small MCUs. Make the
complete module graph, resource bindings, configurations, and implementation
available for compile-time validation, specialization, and maximal optimization
opportunity. The host performs the template and configuration work; the target
receives the implementation required by the concrete application.

This should enable constant evaluation, inlining, branch elimination, and removal
of unused code. Template/header visibility and link-time optimization are possible
mechanisms; the exact build arrangement is undecided. A whole-application
declaration alone does not guarantee visibility into opaque separately compiled
libraries. Check the resulting image size, RAM use, and generated instructions;
no exact optimization or zero-overhead result has yet been measured here.

### 8. Make the application API seamless between AVR and ESP32

Modules using capabilities available on both families should retain their
requirements and operations when changing target. Backends supply hardware
allocation, configuration, and binding details. Model capabilities explicitly
so unsupported requirements can be explained.

Classic ATmega328P-based Arduino Uno, Nano, and related AVR boards are the first
complete peripheral milestone. Initial ESP32 MCU work may be needed during AVR
development to validate the shared API before it is settled. Broader ESP32
coverage follows. Xtensa versus RISC-V remains undecided. ARM and possibly PIC
were mentioned as later possibilities, without commitments to support them.

### 9. Integrate interrupts into the declarative model

Interrupts are the major missing capability. Modules should declare their needs
and handlers while the backend supplies the platform binding. Generated C++ is
an acceptable implementation tool where needed to make the public interface
generic. Pure C++ binding, per-device generation, and per-application generation
remain alternatives to investigate. Generated hardware data and interrupt glue
must fit the intended Arduino workflow.

### 10. Preserve Arduino usability and treat compatibility deliberately

Published libraries should work in the Arduino ecosystem without making GWZ a
prerequisite for sketch users. Exact stock-toolchain guarantees are still open.
Historical compiler limits and missing standard headers were real constraints:
`setl`/`setlx` supplied the subset Ardoinus needed using available language support.
Evaluate language support, library availability, and platform integration
separately when choosing what to retain or modernize.

### 11. Let repository modularity support whole-application composition

The initialized `grevir-wz` workspace is intended to coordinate independently
maintained repositories extracted from the Ardoinus monorepo. Its primary purpose
is to make small,
useful subsets independently consumable with only their actual dependencies.
Repository boundaries should support that independence while preserving dependency
composition and compiler visibility across the final application. Exact extraction
boundaries and release practices remain open. A repository component is distinct
from a runtime/application module. Delivery milestones and isolated consumer checks
are set out in the [standalone packaging plan](ArdoPackagingPlan.md).

## Open idea: multiple applications in one firmware image

Gianni asks whether two or more applications can be compiled into one image and
the active application selected at boot. The discussion identifies this as
feasible in principle; Ardoinus does not yet implement a complete mechanism.
Each application can have its own compiled module graph and resource allocation.
Runtime boot selection chooses which graph is activated.

The proposed resource semantics are:

- Check all modules and dependencies that coexist within an application.
- Allow mutually exclusive applications to use the same hardware differently.
- Check persistent firmware services against every selectable application.
- If applications run concurrently, check their combined resource claims.

Initialization, interrupt ownership/dispatch, instance identity, and memory
layout need design. Inactive applications must not configure hardware as a side
effect of startup. All selectable code remains in flash; the current static
state model can reserve RAM for inactive modules as well. Reusing that RAM would
require explicit lifetime/storage management, not merely selecting another graph.
Whole-application optimization still applies to each compiled alternative, while
boot selection and any interrupt dispatch retain the necessary runtime work.

Selecting once per boot and requiring reset to change application was an assistant
recommendation, not a settled requirement. Live switching, teardown, and resource
handover have not been designed. There is no approved boot-selector API yet.

## Discussion assessment: relevance to AI-assisted embedded development

The assessment discussed was that AI makes code production cheaper while an
explicit hardware/resource model can make the resulting composition checkable.
Ardoinus could let people and AI assemble modules, obtain deterministic allocation
or conflict feedback, and inspect the chosen configuration. Clear diagnostics
and an allocation report were suggested as useful directions, not implemented
features or a separately agreed AI integration project.

This assessment is supported by neighboring approaches: [Nordic supplies AI tools
with verified SDK and device context](https://www.nordicsemi.com/Products/Technologies/AI-assisted-development),
[Zephyr derives configuration from hardware descriptions](https://docs.zephyrproject.org/latest/build/dts/intro-input-output.html),
[Embassy checks interrupt bindings through types](https://docs.embassy.dev/embassy-rp/git/rp2040/macro.bind_interrupts.html),
and [modm generates tailored C++ peripheral libraries](https://github.com/modm-io/modm).
Their existence supports the value of explicit configuration and checked interfaces;
it does not establish uniqueness, comparative performance, or future adoption
of Ardoinus. Validation still includes hardware behavior beyond modeled constraints.

## Related records

- [Move-forward goals and open decisions](ArdoPlanIdeas.md)
- [Initial survey, generator notes, and compiler research](ArdoStateOfPlayAug26.md)
- [Ecosystem review and comparisons](ArdoEcosystemReviewSep26.md)
