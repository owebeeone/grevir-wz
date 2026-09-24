# Interrupt binding architecture review

Scope: [interrupt binding architecture](GrevirInterruptBindingArchitecture.md),
the shared Grevir contract, the proposed mock MCU backend, ATmega328P AVR
binding, and classic ESP32 Arduino binding. Apply the
[cross-MCU policy](review-policies/CrossMcu.md) plus the
[AVR](review-policies/Avr.md) or [ESP32](review-policies/Esp32.md) supplement
for target-specific conclusions. This is a design review of proposed code, not
target compilation or hardware validation. No independent reviewer took part.

## Corrected design defects

### A generated gate could not be emitted without C++ type names

Trigger: a handler for a nested event type such as
`MotorInstance::Timer::PeriodElapsed`. The previous architecture promised that
the generator would emit `BindingGate<Event>` specializations, while also
promising that the generator would never read or emit C++ type spellings. Those
promises were incompatible. This was a shared design defect introduced by the
architecture draft, not inherited code.

Correction: the generator emits `BoundEventKey<Key>` specializations using
stable event keys. Each specialization resolves the exact event type through
`EventCatalog<GrevirApplication>::ByKey<Key>` in C++; generic
`BindingGate<Event>` requires that exact type. A distinct type with the same
key cannot borrow its binding. The
[host syntax probe](../scratch/interrupt-detector/check_key_gate.py) accepts the
mapped event and rejects both an unbound catalog event and a foreign type with
the mapped key. This checks C++ syntax on the local host compiler, not AVR or
ESP32 target compilation.

### Repeated setup could race on ESP32

Trigger: two FreeRTOS tasks call application start concurrently. An ordinary
Boolean idempotence guard can let both enter registration. This is an ESP32
correctness risk introduced by the draft's unspecified guard implementation.

Correction: the architecture now requires a task-safe start state machine with
one winner, visible results for both callers, and sources left masked on
failure. Its synchronization and callback-context behavior remain to be
implemented and target-tested under the ESP32 policy.

## Open implementation gates

### Handler body may depend on allocation before discovery finishes

Trigger: an inline `on_interrupt<Event>` specialization calls a module method
whose type or body instantiates the selected timer binding. The probe must
parse that specialization before it can detect the demand and run allocation.
This is a shared API/design concern with a concrete circular-instantiation
scenario; no current mock or firmware example proves it safe.

The architecture requires a preallocation module facade so the handler's
interface is available during discovery, with selected operations resolved
after allocation. The first mock integration must compile a representative
handler that uses an allocated timer **without** a second user-authored
handler declaration. If that fails, the handler declaration/definition
contract must change before implementation proceeds.

### Object-section transport is not yet target-proved

Trigger: the proposed probe serializes a plan into `.grevir_irq_plan`, then
the host tool extracts it from a mock, AVR or ESP32 object. The scratch probe
uses `nm` symbols and a handwritten plan; it does not prove section emission,
relocation-free encoding, extraction, LTO behavior, or ELF/Mach-O/COFF support.
This is a shared tooling concern, with target-specific validation requirements.

The mock path must first exercise the actual section protocol and generator
through CMake on supported host object formats. AVR and ESP32 then need named
target-compiler and object-extractor checks. A failed extraction must fail the
build; no scratch `nm` or handwritten-plan fallback may silently replace it.

### Probe and strict compilation must see equivalent application settings

Trigger: a CMake firmware target adds a board or peripheral definition absent
from its probe configuration, or Arduino CLI discovers different libraries
after generated code appears. The emitted binding could describe a different
program. This is a shared build-tooling correctness concern.

The strict build recomputes and compares the canonical plan; the CMake adapter
requires a common `CONFIG_TARGET`; the Arduino wrapper uses the same FQBN,
libraries and build properties in both passes. Integration tests must
deliberately change a plan-affecting definition between passes and verify
failure. The exact Arduino compilation-database and library-discovery behavior
still needs a named-core test.

## Review disposition

The binding-set authority and target-neutral pipeline remain coherent design
directions, but they are not yet an end-to-end implementation. The key-gate
inconsistency has been corrected and host-probed. The
preallocation handler-body cycle and probe transport are the leading gates for
the mock implementation. AVR vector ownership, ESP32 callback/affinity/IRAM
rules, and silicon behavior remain target-specific work. External code that
bypasses Grevir's bindings is outside compile-time ownership proof and must be
handled by reservations or runtime failure reporting.
