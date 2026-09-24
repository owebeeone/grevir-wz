# Declarative interrupt binding — investigation and scratch evidence

The [interrupt binding architecture](GrevirInterruptBindingArchitecture.md)
supersedes the proposals in this document. This file retains the reasoning,
scratch observations and alternatives that led to that specification.

## Scope and present state

Interrupts are the largest missing part of the Grevir peripheral story. The
current timer integration implements fixed-frequency PWM; it does not bind an
application handler to an AVR vector or ESP32 interrupt source. The public
timer API must nevertheless leave room for counting, compare events, capture,
timed alarms and their handlers now. These functions are constrained by the
selected timer mode and often share registers, channels or vectors.
[Timer allocation design](GrevirTimerAllocationDesign.md),
[original interrupt notes](ArdoPlanIdeas.md#major-missing-capability-declarative-interrupts)

The user's proposed application syntax is that **defining a specialized handler
function is the declaration of intent to handle an event**. Grevir should use
that fact to add an interrupt requirement to the complete application, select a
legal peripheral configuration and arrange the target binding. No separate
manual vector name, registration call or duplicate handler list should be
required from the application. This is a design direction, not an implemented
or yet accepted C++ spelling.

An illustrative spelling, with no promise that these exact names are final:

```cpp
template <class Event>
void on_interrupt() noexcept = delete;

template <>
inline void on_interrupt<MotorInstance::Timer::PeriodElapsed>() noexcept {
  MotorInstance::tick();
}
```

`MotorInstance::Timer::PeriodElapsed` is one **logical event type** that already
identifies the concrete module instance, timer request and event kind. It is not
`TIMER1_OVF_vect` or an ESP32 CPU interrupt number. Two instances of the same
module implementation have distinct event types and may define different
handlers. The selected backend maps each event to its physical source and
platform entry point. A handler signature and context policy still need to be
chosen; the example only shows the proposed detection mechanism. A second
template argument repeating the instance would add ceremony without adding
information.

## What the specialization can and cannot trigger

For a finite set of event types known from a module's requested peripheral
capabilities, Grevir can probe whether a specialization is callable. One
possible C++23 mechanism is a deleted primary function template plus a
`requires` expression. A native Clang syntax probe confirmed that a visible
single-event specialization satisfies such a probe and an unspecialized event
does not.
This is evidence for the language mechanism only, not AVR or ESP32 ABI evidence.

The probe must run where each relevant specialization **declaration is visible
before instantiation**. It can test a known event; standard C++ does not provide
a way to enumerate every specialization defined somewhere in an arbitrary
program. A declaration alone can satisfy the probe even if its definition is
missing, so ordinary compile/link rules must still verify that the handler body
is present. A handler hidden in another translation unit cannot silently become
an application-wide discovery mechanism. The intended contract should put the
specialization in the application configuration/header visible to its binding
pass, or otherwise make its declaration visible there.

Discovery should proceed through the existing module dependency closure:

1. Identify each concrete module instance and its requested peripheral
   capabilities, with stable request identity independent of declaration order.
2. Probe only the event vocabulary of those capabilities for visible handler
   specializations. A discovered handler adds an interrupt requirement; a
   missing handler must not create a vector binding.
3. Select a whole legal peripheral configuration that supplies every requested
   function and interrupt event. An ISR cannot rescue an incompatible timer
   mode, consumed compare channel or unavailable vector.
4. Produce one binding record for each handled logical event from the selected
   allocation. The record identifies the event, peripheral owner, physical
   source, event selector (when a source carries multiple events), and target
   entry mechanism. The complete binding set is the authority for both
   conflict checking and entry generation; a separately declared resource
   claim cannot establish that a handler is bound.
5. Configure the peripheral and handler before enabling the source. The target
   backend owns acknowledgement/masking rules and records any runtime
   acquisition failure.

This puts interrupt capability into the timer candidate footprint **now**, even
though PWM is the only timer function implemented. The first interrupt slice can
be narrow, but the candidate representation must express offered events, source
identity, vector/CPU routing and conflicts with other functions. It must not
pretend that `PWM`, `capture` and `compare interrupt` are independent flags.

One physical interrupt source has **one owner and one target registration**.
That owner controls source masking, status inspection, acknowledgement and
hardware reads. In particular, two UART receive handlers must never race to
drain the same FIFO; a UART owner drains it once and provides received data to
dependent modules through its own resource API. The existence of multiple
`on_interrupt` specializations must not cause independent `esp_intr_alloc()`
calls for the same source. If a peripheral exposes distinguishable event bits,
its owner may dispatch separate logical events after taking a status snapshot
and applying a defined acknowledgement policy. Dispatch order is stable by
event identity where order matters; no application may depend on incidental
toolchain registration order. Distinct peripheral sources may share an ESP32
CPU interrupt only when the backend can keep their status and ownership
independent; that is separate from sharing ownership of one source.

The universal enforcement mechanism should be the **binding set**, not an
independently authored interrupt resource claim. Let `D` be the detected
handler-event set and `B` the selected binding records. The build must require
`events(B) == D`, with each event appearing exactly once. For every physical
source in `B`, all records must identify one owner and one target entry;
multiple logical events on that source require a backend-defined status
snapshot, distinguishable selectors, acknowledgement policy and dispatch order.
Otherwise the plan fails. The allocator also checks that each binding is legal
for the selected peripheral mode and that the bound owner belongs to the
application's dependency closure. A source key means the peripheral interrupt
source, not merely an ESP32 CPU interrupt line; separate sources may share a
line. The present scratch generator permits one event per source, a narrower
first slice.

The same validated `B` must drive **all** interrupt outputs: the detector
specializations, AVR vector entries or ESP32 callback registration table, and
source enable/configuration. There must be no second, handwritten owner list or
registration path inside Grevir. `AllocatedApplication` currently calls
`Allocation::setup()`, but that call site alone does not prove one runtime
invocation. A production path needs an idempotence/duplicate-registration guard
and a visible failure result. Ordinary Grevir resource claims can still model
timer and peripheral allocation constraints; an interrupt claim derived from
`B` may be useful to integrate with those checks, but it is never independent
evidence of an ISR demand or binding.

Link-time checks are supplementary. On AVR, two strong definitions of the
same vector symbol normally produce a duplicate-definition link error. On
ESP32, two calls to `esp_intr_alloc(source, ...)` have no inherently conflicting
symbol, so the linker cannot detect them without a deliberately emitted strong
source-specific guard symbol. Ordinary template instantiations or inline
variables may be coalesced and are not such a guard. External libraries that
do not use Grevir claims or guards remain outside compile/link proof; the
ESP32 setup path must check runtime acquisition and ownership failures.

A [host-only ownership probe](../scratch/interrupt-ownership/README.md) exercises
the existing Grevir claim checker and both link strategies. Two dependent
consumers of one source owner compile; two source owners in one application or
a repeated source claim inside one allocation fail compilation. Two ordinary
registration calls in separate translation units link and run, whereas
emitting the same strong source-specific symbol in both units fails linking;
two strong AVR-like vector symbols likewise fail linking. The probe also
establishes the limits: two separately instantiated application closures can
each claim the same source and still link, and calling one application's
`runSetup()` twice invokes backend setup twice. Most importantly, the claim
probe does **not** correlate a claim with a discovered handler. It demonstrates
why a claim cannot be the interrupt binding authority. A production build
needs one canonical application composition and must derive its registrations
from the checked binding set. Compile/link checks do not cover code that
bypasses Grevir.

## Target binding differs

| Target | Binding obligation | Consequence |
| --- | --- | --- |
| AVR | The vector table reaches a predetermined symbol with the compiler's ISR calling convention. | A thin target entry function must dispatch to the specialized application handler. Stock AVR toolchains need an explicit vector-symbol strategy; recent GCC/AVR-LibC also document numbered ISR attributes, but that route has not been target-proved here. |
| ESP32 Xtensa | Peripheral interrupt sources route through an interrupt matrix/allocator to a CPU interrupt and callback. | The backend registers one callback bridge per owned source at setup, even though ESP-IDF permits multiple handlers for a source. It must account for distinct sources sharing a CPU interrupt, affinity, priority, source status, IRAM safety and resource lifetime. The same user-facing specialization need not imply generated vector symbols. |

[AVR-LibC ISR interface](https://avrdudes.github.io/avr-libc/avr-libc-user-manual/group__avr__interrupts.html),
[GCC AVR ISR attributes](https://gcc.gnu.org/onlinedocs/gcc/AVR-Attributes.html),
[ESP-IDF interrupt allocation](https://docs.espressif.com/projects/esp-idf/en/v5.5/esp32/api-reference/system/intr_alloc.html)

The handler is part of the application, while the entry ABI and dispatch are
target code. Generated AVR glue, if required, belongs in the consuming
application's build area rather than an installed shared library. The backend
must respect Arduino/core and external-library claims; a strong AVR vector
definition must not silently displace a core ISR.

## Generation question

A concrete [scratch probe](../scratch/interrupt-detector/README.md) implements
the user's proposed default-template-argument gate:

```cpp
template <class Event> struct DetectorImpl;
template <class Event, class Detector = DetectorImpl<Event>,
          unsigned BindingId = Detector::binding_id>
void on_interrupt() noexcept = delete;
```

When a handler explicitly specializes `on_interrupt<Event>` but no
`DetectorImpl<Event>` specialization is visible, Apple Clang 21 rejects that
declaration and names the event in the substitution diagnostic. When a
generated detector is included first, the same handler compiles, links and
runs. A mapped event with no handler remains uncallable. This is a valid
compile-time mapping gate on that compiler; `BindingId` is a dependent default
that requires a generated binding. C++ permits omitted trailing function-template
arguments to come from defaults; the scratch result shows this rule applied
to an explicit specialization.
[C++ draft function-template specialization rule](https://eel.is/c++draft/temp.fct.spec.general)

A deliberately failed compile pass could thus expose handler/event types in
diagnostics and let a tool generate detector specializations and entry points.
It is a plausible prototype, but diagnostic prose and template-name formatting
are not a stable machine-readable protocol across compilers or versions.
Failure also obscures real application errors and complicates incremental
builds. Treat the diagnostic-parsing step as an experiment, not yet the default
build contract.
The successful build also needs a specific declaration order: application event
types first, generated detector specializations second, user handler
specializations third, and application-plan instantiation last. A forced include
at the top of a sketch cannot simply specialize a nested event type that has
not been declared yet. This order is straightforward in the scratch's split
headers but needs an ergonomic Arduino integration design.
The detector gate and handler discovery are different jobs. The gate rejects a
handler with no mapping; it does not enumerate handlers for a generator. Grevir
can probe a finite catalog of known logical events in C++ after their
specializations are visible. That result could drive a structured binding
manifest without parsing expected compiler failures. The failure-driven route
remains useful if a build tool needs to discover unknown event types directly
from handler declarations, but its diagnostic and ordering costs must be
measured against this catalog-based route.

The [scratch generator](../scratch/interrupt-detector/README.md) now exercises
the **host-side** discovery, allocation handoff and strict compile cycle:

1. An event catalog supplies logical event IDs and C++ event names, without
   allocating vectors. Python generates a discovery translation unit with a
   permissive detector primary and one probe call for **every** catalog event.
2. Compile that translation unit to an object. On Apple Clang 21 at `-O2`,
   undefined symbols encode `(event ID, handler present)`. Python reads them
   with `nm` and `c++filt`, requires the exact catalog event set and a SHA-256
   catalog fingerprint, then writes the discovered-handler manifest. Catalog
   records are sorted by stable ID before hashing, so declaration order does
   not affect the result. The discovery object is never linked into firmware.
3. The discovered handlers become **requirements for the allocator**. A
   selected plan must assign each handled event a legal physical binding and
   vector, with no binding for an unhandled event. The scratch uses a
   hand-written plan fixture; Python checks exact set equality, event type
   identity and unique binding/source/vector ownership before emitting code.
4. Python emits a detector header only for handled events and a C++ source
   containing `ISR(vector)` entries under `__AVR__`, plus mock entries when
   explicitly enabled for the host probe. Other targets fail preprocessing.
   The strict host build includes generated detectors before the
   user handler, compiles the generated source and invokes the mock entry.
   A handler without a generated detector still fails at its specialization.
5. Repeat discovery on every relevant build. A failed discovery, allocation
   or generation step must invalidate prior output rather than permit a stale
   but compilable vector mapping.

The prototype deliberately encodes integer IDs rather than C++ type names in
the symbols. `nm` is defined to list undefined object symbols, but mangling and
demangling styles still vary by compiler; production needs an adapter per
host/object format or a different structured manifest. The marker must remain
reachable through compilation and object inspection must precede LTO or
dead-code elimination that could discard it. The catalog and selected plan are
hand-written fixtures; a production catalog must come from the module closure
and the selected plan from Grevir's allocator **after** handler discovery, with
stable IDs independent of declaration order. The event declaration / generated
detector / handler ordering problem remains for ordinary sketches. This proves
host compilation and dispatch through a mock entry, **not** AVR vector ABI,
ESP32 routing or Arduino build integration. The emitted AVR ISR branch has not
been target-compiled under the existing hold.
The current AVR allocation implementation
([PWM program](../grevir-avr/src/grevir/avr/devices/atmega328p/pwm_program.hpp),
[PWM model](../grevir-peripherals/src/grevir/peripherals/pwm/model.hpp))
selects fixed-frequency PWM candidates; it does not yet consume a discovered
interrupt requirement or serialize an event-to-vector binding. That bridge is
the main remaining design/implementation gap exposed by the generator.
[GNU nm documentation](https://sourceware.org/binutils/docs/binutils/nm.html)

Use one build pipeline for every target: discover visible handler
specializations from the canonical application closure, pass those events as
requirements to the target allocator, validate the resulting binding set, then
generate and compile one application binding unit from that set. The generator
uses a target-neutral binding schema and a target emitter: AVR emits vector
entries; ESP32 emits the owner callback/registration code. This is one
**authority and validation mechanism**, although the target entry ABI differs.
No application-authored interrupt claim or second registration list is an
input. A pre-link step over compiled metadata is one possible discovery
transport; a target-independent configuration probe is another. Neither is yet
proved against Arduino's build pipeline. The output must be deterministic,
keyed by the application/target plan, and rebuilt when the plan or toolchain
changes. Arduino documents pre-build and pre-link recipe hooks, but their
availability alone does not prove generated sources are automatically picked
up by library discovery.
[Arduino platform hooks](https://docs.arduino.cc/arduino-cli/platform-specification/#pre-and-post-build-hooks-since-arduino-ide-165)

Before implementation, test a small AVR proof with one selected event, one
specialization and a correct vector binding under the actual supported
toolchain; inspect symbol ownership and the final vector table. Then test an
ESP32 callback bridge generated from the same binding schema. Target proofs
and Arduino build integration are design gates, not completed work; respect
the current validation holds.

## Next bounded design slice

Make the interrupt API the next timer-integration gate. Settle the following in
order, using host-only examples and synthetic capability inventories while the
AVR compiler and hardware holds remain in place:

1. Choose the handler spelling, signature and visibility rule. The candidate
   above uses one event type containing its module-instance identity and
   `noexcept`. Define whether an event carries data, how the handler reaches its
   module state, and exactly where the specialization must be visible.
2. Specify one timer event precisely (for example, counter wrap), then probe
   its specialization from the module closure. Presence adds an interrupt
   requirement; absence leaves the source disabled. Prove declaration-order
   independence and that an impossible event/mode combination is rejected.
3. Describe the candidate footprint as one selected timer mode with its
   outputs, event sources, registers, vector/CPU-line constraints and
   compatible consumers. Exercise two synthetic candidates where PWM and an
   event can coexist, and where the same event would consume PWM's TOP or
   compare resource and must fail.
4. Define a target-neutral dispatch contract for multiple logical events on
   one physical source/vector. One backend owner reads status and acknowledges
   once, then dispatches distinguishable logical events in stable identity
   order; conflicting consumers of one FIFO or indistinguishable event fail.
   Keep ISR execution separate from deferred task delivery.
5. Only after the API and host model are coherent, prove one actual binding
   per target. AVR vector entry proof waits for the explicit compiler hold to
   lift. ESP32 can first use a named target compile/link check, with silicon
   validation still deferred. Decide whether application-specific generation
   is necessary from those binding proofs.

This slice establishes the extensible API and allocator obligations. It does
not require implementing every timer mode, pin interrupt, UART interrupt or
CAN interrupt before further PWM integration.
