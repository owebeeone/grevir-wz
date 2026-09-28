# Declarative MCU integration: don't-dos

## Project purpose

Grevir exists to make MCU applications declarative. An application states its
modules, requirements, constraints and any target-specific preferences;
device and board descriptions supply authoritative capabilities. From these
inputs, C++ templates and constant evaluation should derive every integration
decision knowable at build time: dependency closure, resource ownership and
allocation, peripheral choices and configuration, startup order and event
bindings. The result should be deterministic, inspectable and checked against
the modeled constraints before firmware runs. Moving hand-written wiring into
template syntax is not enough; the system must derive the wiring from intent.

Build-time generators may translate the validated plan into syntax required
by a particular toolchain. They must not independently choose hardware or
maintain another source of configuration truth. At runtime, selected resource
owners apply the plan and handle interrupts, inputs and failures that cannot
be resolved from build-time facts. Compile-time proof is limited to the facts
and behavior the model actually covers.

The rules below protect that purpose across peripherals, modules, boards, MCU
backends and mocks. Each rule is general; the named AVR, PWM and interrupt
incidents are examples, not limits on its scope or prescriptions for another
architecture. The examples do not imply that the current interrupt ownership
design has been completed.

## 1. Do not create a parallel device or resource model

Derive capabilities, resource identities, encodings, routes and constraints
from authoritative device facts or explicitly documented board facts. An
identity must distinguish the intended resources within its device and board
scope and preserve aliases and conflicts. Do not maintain a second inventory
that must be updated whenever the authoritative model changes.

Example: the AVR PWM integration invented `PB1 = 101`-style resource IDs even
though the extracted definitions and typed GPIO model supplied port-register
and bit information. A bit position alone would not distinguish pins on
different ports; an arbitrary number duplicated their identity.

Review check: trace each target-specific fact and each resource identity to its
source. Identify independently maintained values and mappings.

## 2. Do not work around an inadequate abstraction without reviewing its contract

When a generic API or provider interface cannot express a required operation,
identify the missing information and decide which boundary owns it. Do not
invent target-specific glue or bypass the provider solely to preserve an
existing parameter type or method shape. Extend the appropriate contract when
that is what the operation requires.

Example: numeric `Pin<P>` and `GPIOResource<P>` parameters were accepted
without reconciling them with Ardoinus's typed `GpioPortDefinition`. The
existing timer configuration interface covered PWM but not the needed
interrupt startup operation; that gap did not justify board-level register
programming.

Review check: state what cannot be represented or derived, and why the chosen
API or provider is the right place to add it.

## 3. Do not transfer provider implementation into coordination layers

A board can declare capabilities, reservations and startup policy. An
application lifecycle can order providers and dependencies. Neither should
independently implement a selected provider's configuration, enable, disable
or cleanup effects. The provider responsible for a resource must interpret
its selected configuration and own those effects, possibly through a
target-specific backend or dependent module.

Example: the AVR board's `configure<Spec>()` programs Timer1 directly, while
board hooks also mask and enable it. Timer1 is the example; the same ownership
question applies to any selected timer, GPIO, serial controller or other
peripheral on any target.

Review check: name the selected owner of every hardware effect and verify that
startup and failure cleanup touch only resources owned by that provider.

## 4. Do not encode an example's allocation shape into lifecycle behavior

Lifecycle dispatch must reach every selected owner through declared identities
and dependencies. Do not assume a fixed count, incidental array position,
declaration order, request name, candidate kind or physical peripheral merely
because an initial example has one of each. Passing the whole application
specification to a hook does not make a hard-coded implementation generic.

Example: `solution.configurations[0]` and a board-wide `configure<Spec>()`
work for the current single Timer1 request but do not define how another
selected owner is configured. Reordering independent module declarations
must not change their assignments or startup behavior.

Review check: reason through the supported zero, one and many-owner cases,
alternative candidates, dependency order and partial failure.

## 5. Do not let applied behavior diverge from the selected plan

The selected candidate must determine every behavior it promises. No example,
board or backend may independently reselect or override relevant parameters.
Recognizing a configuration name is not proof that the actual setup implements
its mode, routing, timing, interrupt semantics or other selected properties.

Example: an earlier AVR interrupt example could select a PWM candidate while
programming the timer in normal mode. Later manual mappings from a selected
name to mode bits, TOP and preload still required proof that they implemented
the complete candidate semantics. Other peripherals have analogous routing,
sampling, transfer or driver parameters.

Review check: trace request -> selected candidate -> provider configuration ->
applied effects. Compare distinct legal selections and verify that all
relevant effects follow each selection.

## 6. Do not bypass owner facilities or leak backend primitives

Use or extend the selected owner's established configuration and programming
facilities. Derive encoded values from applicable metadata and enforce the
operation's semantic preconditions. Keep target or toolchain primitives within
explicit backend boundaries; public declarations for one target must remain
parseable when another target is resident. This rule does not require every
architecture to use the same low-level programming mechanism.

Example: AVR timer setup already had typed fields and `setl::ApplierValues`.
Direct `TCCR1A`/`_BV` writes, a thin `Timer1Overflow` wrapper, and hard-coded
preload literals bypassed that machinery. The generated AVR entry translation
unit is a distinct toolchain boundary where `ISR(...)` is needed to declare a
vector; that exception does not license avr-libc register macros in device
declarations or application configuration.

Review check: identify the provider operation behind each effect, the source
of each encoded value, and every crossing into target-specific code. Inspect
inactive target branches too.

## 7. Do not mistake a local repair for an architectural repair

After fixing a symptom, recheck provenance, resource identity, selected-plan
interpretation, ownership and lifecycle order. A mechanical change can leave
the original design defect intact even when compilation and behavior tests
pass. Record any remaining architectural work explicitly.

Example: replacing an AVR register macro with a typed write still leaves an
ownership problem if the write remains in the board. Renaming `PB1` to avoid
macro substitution removes the collision but leaves the invented numeric
identity model in place.

Review check: state which architectural cause the repair removed and which
duplicated facts, mappings or misplaced effects remain.

## 8. Do not use one form of validation as proof of another

Architectural review, source-boundary inspection, host parseability, target
compilation and runtime or hardware behavior establish different claims.
Apply the shared review policy and the relevant MCU supplement to each target.
Inspect disabled platform sections as well as active ones, and preserve any
validation hold. A successful build cannot prove declarative provenance or
modular ownership by itself.

Example: a native AVR-library compile must run without AVR target and hardware
macros, while a source check catches forbidden tokens hidden in inactive
`#if` branches. A generated `ISR(...)` entry instead needs AVR target
compilation. Neither check proves that a selected peripheral is configured by
its owner. Other backends need equivalent boundary checks for their own SDK
symbols and target conditions.

Review check: report separately what each check establishes and what remains
unverified. Do not use target compilation as a substitute for architectural
reasoning, or host compilation as a substitute for target evidence.

## 9. Do not confuse constant evaluation with target-library availability

A facility used only to compute a build-time plan must still be declared by
headers available to the target compiler when that compiler parses the source.
Keep target-reachable compile-time algorithms within the selected toolchain's
language and library surface. Where a library facility is absent, use a bounded
equivalent or a tested compatibility implementation; keep its capacity derived
from the model rather than an arbitrary magic limit.

Example: AVR PWM candidate generation used `std::vector` only during constant
evaluation, but the selected AVR C++23 compiler has no libstdc++ `<vector>`.
Its fallback `array`, `tuple` and algorithm operations also need the specific
constexpr behavior that candidate generation invokes. A host build cannot
establish either property.

Review check: compile a representative public target header and each selected
feature with the named target compiler. Check its headers and required constant
evaluation, then distinguish successful compilation from runtime and silicon
validation.

Related policies: [Cross-MCU](review-policies/CrossMcu.md),
[AVR](review-policies/Avr.md), and [ESP32](review-policies/Esp32.md).
