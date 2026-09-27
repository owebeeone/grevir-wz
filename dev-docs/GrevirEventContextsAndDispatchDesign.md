# Event contexts and deferred dispatch

Status: design proposal. `MainLoop`/`Elide` and `MainLoop`/`Stream` dispatch are
implemented for mock and ATmega328P AVR; named contexts, software-only events,
the deadline service, and ESP32 deferred dispatch remain proposed. This
document extends the [interrupt binding architecture](GrevirInterruptBindingArchitecture.md)
and its [implemented first slice](GrevirInterruptBindingImplementationProgress.md).
It does not change the existing `grevir::on_interrupt<Event>()` contract.
The two-parameter handler spelling below has been superseded by the
[one-parameter activation candidate](GrevirEventActivationDesign.md); the
remaining context and deadline semantics here remain under discussion.
Public API documentation and complete examples belong under `/docs` after this
design is implemented.

## Purpose and boundary

An interrupt should acknowledge its source and perform only bounded work.
Application logic usually runs later: in the main loop on AVR, in the Arduino
loop task or a named task on ESP32, and in a deterministic dispatcher in the
mock MCU. The application declares *which logical event is handled*, *where
its handler executes*, and *whether repeated firings are retained*. Target
backends implement those semantics using their own synchronization and wake
mechanisms. A portable context is an application identity, not a promise that
every MCU creates an operating-system thread.

Two distinct scheduling structures exist:

- The **dispatch queue** retains events awaiting application handling. Its
  order is successful enqueue order. Concurrent producers are serialized by
  the backend; their relative order is not inferred from wall-clock time.
- The **deadline queue** retains timer requests ordered by due time. One
  hardware alarm can be armed for its earliest deadline. When a deadline
  becomes due, its event enters a dispatch context. The deadline queue is not
  a FIFO circular buffer.

No deferred callback is promised to run at the precise interrupt instant.
It runs no earlier than its event or deadline and may run later if its context
is busy. An action that truly must happen at the hardware instant belongs in
a small ISR-level handler.

## Proposed C++ surface

The names and signatures below are proposed API, not existing Grevir code.
An event remains a type in a finite module-provided catalog with its stable
`EventKey`. A route states the context and delivery mode; the handler has the
two template parameters requested for `on_event`:

```cpp
namespace grevir::event {
struct MainLoop;                  // Portable application-loop context.
struct IsrLevel;                  // Direct interrupt context.
template <class Id> struct Context; // Named, portable serial context.
struct Elide;                     // At least one firing while pending.
struct Stream;                    // One queue record per firing, until full.
struct Direct;                    // ISR call; no deferred record.
template <class Context, class Delivery> struct Route;
}

namespace grevir {
template <class Event, class Context>
void on_event() noexcept = delete;
}

struct ButtonWentLow {
  using Key = grevir::interrupt::EventKey<"controls", "button", "low">;
  using Route = grevir::event::Route<grevir::event::MainLoop,
                                    grevir::event::Elide>;
};

template <>
inline void grevir::on_event<ButtonWentLow,
                             grevir::event::MainLoop>() noexcept {
  Controls::handle_button_press();
}
```

A second event can use a named context and retain each accepted firing:

```cpp
struct ControlContextId {};
using ControlContext = grevir::event::Context<ControlContextId>;

struct PulseSeen {
  using Key = grevir::interrupt::EventKey<"controls", "pulse", "edge">;
  using Route = grevir::event::Route<ControlContext,
                                    grevir::event::Stream>;
};

template <>
inline void grevir::on_event<PulseSeen, ControlContext>() noexcept {
  Controls::record_pulse();
}
```

The `Route` is part of the event type offered by a module request, so the
probe can enumerate a finite set of event types and read each route. It does
not need to search arbitrary task types or parse C++ symbols. The handler's
context parameter must equal the route's context. A visible specialization
constitutes a demand; a route with no visible handler does not enable a
hardware interrupt. As with `on_interrupt`, the specialization is visible in
the application header during both probe and strict compilation. The
generated key gate rejects a noncatalog, mismatched or unbound handler.

`MainLoop` and `IsrLevel` are predefined contexts. A named `Context<Id>` is
a stable *logical* identity: the AVR backend may dispatch it cooperatively,
while the ESP32 backend may map it to an Arduino/FreeRTOS task. This keeps
the event type and handler specialization unchanged across targets. Target
configuration can choose a task's priority, core affinity and stack size;
nonresident target sections are ignored under the existing cross-MCU
configuration rule. A named context must have exactly one active consumer in
one application. Contexts and event routes cannot depend on declaration order
for identity or tie-breaking.

For direct handling, the new spelling is:

```cpp
struct EncoderEdge {
  using Key = grevir::interrupt::EventKey<"encoder", "pin", "edge">;
  using Route = grevir::event::Route<grevir::event::IsrLevel,
                                    grevir::event::Direct>;
};

template <>
inline void grevir::on_event<EncoderEdge,
                             grevir::event::IsrLevel>() noexcept {
  Encoder::record_edge();
}
```

`on_event<Event, IsrLevel>()` executes in the hardware interrupt context.
It does not gain a queue, task stack or permission to block. `Elide` and
`Stream` are invalid with `IsrLevel`; `Direct` is invalid for deferred
contexts. Existing `on_interrupt<Event>()` remains a supported direct ISR
specialization. For one event, **exactly one** of `on_interrupt<Event>()` and
`on_event<Event, Context>()` may be present. The probe rejects both rather
than silently choosing an order or delivering the event twice. An existing
`on_interrupt` may explicitly post a *different* event when an owner must
translate hardware status into a logical event. The direct event route calls
`on_event` itself; it does not secretly call `on_interrupt`, which would make
ownership and ordering depend on a second user specialization.

The existing generated hardware entry calls a C++
`dispatch_bound_interrupt<Event, Backend>()` template. That template detects
whether the event has a handwritten `on_interrupt` or an `on_event`, requires
exactly one, and calls the selected route. The target entry still snapshots
and acknowledges the hardware source according to the binding plan. A
deferred route then posts a small event record and wakes the destination
context; it must not call the deferred application handler.
Software-originated deferred events can use the same route and context
without claiming a hardware interrupt. A software event does not select
`IsrLevel` unless an actual interrupt-context producer is specified.

An `OnEventBridge<Event, Backend>` class can hold common dispatch code, but
instantiating that class does not define or force an explicit specialization
of the free `on_interrupt<Event>()` function. Taking the address of
`on_interrupt<Event>` while its deleted primary template is selected is an
error. The dispatcher and bridge are ordinary C++ templates: for `IsrLevel`,
the bridge invokes `on_event<Event, IsrLevel>()`; for a deferred route, it
posts the event through the backend. The existing generated entry's call
instantiates these templates. No synthetic `on_interrupt` specialization,
address-taking trick or separate event-code generator is needed. The
[host proof](../scratch/event-dispatch-bridge/README.md) checks direct,
deferred, missing and conflicting handler cases.

## Posting and delivery contract

The proposed posting interface distinguishes caller context without exposing
target-specific queue functions:

```cpp
grevir::event::post<Event>() noexcept;          // Application/task context.
grevir::event::post_from_isr<Event>() noexcept; // ISR context; never blocks.
grevir::event::dispatch<grevir::event::MainLoop>(budget);
```

An Arduino-style application can call `dispatch<MainLoop>()` once per `loop()`
iteration, alongside its ordinary module work. The AVR implementation checks
its ready state without waiting. The ESP32 implementation may wake a
dedicated context task with a task notification; it does not require the
application handler to run inside the interrupt or a timer-service callback.
An optional blocking wait is a backend/context execution policy, not a
different event type.

`post` returns a result such as `queued`, `coalesced`, `full`, or `not_ready`.
The final type and budget units need a small compile probe before freezing
the spelling. Generated interrupt bridges call the ISR form internally; users
need not write a forwarding `on_interrupt` to obtain deferred delivery.
Posting performs no dynamic allocation. A context's capacity, event set and
storage are fixed by the application plan. Application callbacks are
`void`/`noexcept`, execute serially within their context, and may post another
event or rearm a timer. The dispatcher never holds its queue lock while it
calls application code. It processes at most the requested budget per call;
the caller decides how often to dispatch and how to handle pending work.
Within one producer, accepted `Stream` records preserve posting order.
Across concurrent sources, enqueue linearization determines order; Grevir
does not invent a deterministic order for physically concurrent interrupts.
If a dispatch budget expires with records remaining, the context stays ready;
it cannot go to sleep until those records are handled. Backend wake/sleep
sequencing must prevent a post between the empty check and sleeping from
being lost.

| Delivery | Post while no record is pending | Repeat while pending | On full queue |
| --- | --- | --- | --- |
| `Elide` | Enqueue one record and mark this event pending | Return `coalesced`; do not append another record | Return `full`, record an overrun, and **do not** set its pending mark |
| `Stream` | Enqueue one record | Enqueue another record for every firing | Return `full` and record a dropped firing; never claim it was delivered |
| `Direct` | Call the handler in ISR context | Call it again for each interrupt | No queue exists |

For `Elide`, the consumer clears the pending mark atomically with dequeuing
the record, **before** invoking the handler. A firing during that handler can
therefore enqueue a new record. The earlier record keeps its original queue
position; coalescing does not move it to the back. An empty queue implies no
pending marks. If enqueue fails, the mark remains clear so a later firing can
retry. This first contract records loss rather than silently replacing queue
order with a fallback bit scan. The application may inspect sticky overrun
state and choose a recovery policy; clearing that diagnostic must not clear
still-queued events. `Stream` means every *accepted* firing has a record, not
an impossible guarantee that an undersized finite queue never overflows.

This first event API carries no general payload in the queue. A device such
as UART, SPI or CAN retains bytes or frames in its own owned buffer and may
post an elided `DataReady` event. A future payload route needs separate
bounded storage and an explicit copy/lifetime/overflow contract. A counted
event is likewise a separate delivery mode, not an accidental interpretation
of `Stream` or `Elide`.

## Context ownership and backends

One context-owning dependent module initializes and manages each dispatch
context. Client modules depend on it and request delivery resources. The
application plan validates one context identity, one consumer, a finite queue
capacity, legal routes, and the target's ability to instantiate the requested
execution policy. It cannot create two task owners for the same context or
allow two bindings to read the same hardware FIFO independently. A context
can receive events from several hardware sources, timers and software posts.

The portable contract is serial callback execution *within* one context.
Callbacks in different ESP32 contexts may run concurrently; the API must not
imply global serialization. The application must provide synchronization for
state shared between contexts. An ISR may never wait for queue space or take
a blocking task lock. Wakeup is a consequence of a successful deferred post,
not an additional application-authored interrupt claim. A backend may wake
again on a coalesced post, but may not rely on repeated posts to recover a
lost wakeup for an already queued record.

| Backend | Deferred context mapping | Required proof |
| --- | --- | --- |
| Mock MCU | Deterministic in-memory queue and explicit `dispatch` calls | Ordering, elision, overflow, reentrant posting and startup faults |
| ATmega328P AVR | `MainLoop` and named contexts dispatched cooperatively; short interrupt-masked critical sections where multi-byte state or loop-origin posting requires them | ISR/main-loop publication, capacity, vector/stack cost, no floating-point path or unneeded wide runtime arithmetic |
| Classic ESP32 | `MainLoop` maps to Arduino's loop task; named contexts may map to statically provisioned FreeRTOS tasks or share a dispatcher | Cross-core/ISR producers, atomic publication, ISR-safe wakeup, task lifetime, priority, affinity and stack limits |

For the first implementation, each deferred queue record contains a plain
callback and one opaque argument, with no `std::function` or owned callable:

```cpp
struct DispatchRecord {
  void (*invoke)(void*) noexcept;
  void* argument;
};
```

The generator supplies a typed, noncapturing thunk for each event route. The
argument is either null or points to storage with a lifetime covering the
pending record; the record does not own a payload. A source that needs bytes
or frames keeps them in its own bounded buffer. The same record contract
applies to AVR, mock and ESP32, while each backend provides its own queue
synchronization and wakeup. Pointer widths, and therefore record size, follow
the target ABI and are included in capacity budgeting. The public API does
not expose the record layout. The generic event bridge must not own a plain
pending flag or mutate queue indexes: the backend's `post_from_isr<Event>`
operation owns the coalescing check, enqueue, pending mark and publication as
one serialized transaction. Its dequeue operation removes the record and
clears that mark under the same synchronization, then calls the callback
outside the critical section. This is the proposed common contract; AVR and
ESP32 implementations must prove they can uphold it. The existing
`setl::CircularBuffer` is a
candidate, **not** an established ISR-safe backend: its default
`setl::System` barrier is a no-op, its comment
assumes one writer and one reader, and its sticky overrun behavior needs to
match the contract above. Multiple AVR vectors are normally serialized but
loop-origin posts and explicit interrupt nesting change that assumption.
The AVR adapter must prove index width, publication order, queue-full
behavior and the exact critical section used; `volatile` alone is not a
synchronization proof. On ESP32, producers may run on different cores, so
the backend may use an appropriate static RTOS queue or its own bounded
multi-producer structure plus task notification. It must not reuse an AVR
single-producer proof by analogy.

Context startup precedes source enablement: allocate/provision static queue
storage, create or attach destination tasks, initialize dependent modules,
install interrupt entries, apply each pending-event policy, and only then
enable sources. Failed context creation or attachment is a terminal
`Application::start()` failure that leaves owned sources masked. If a source
is pending during startup, its existing preserve-or-declared-discard policy
still applies; the event cannot be delivered into an unready context.

## Timer deadline service

The timer service is another dependent module and event producer, not a
special callback path. It owns one selected timer or alarm capability and a
fixed-capacity deadline set. Clients identify scheduling slots with stable
types. The first contract allows one outstanding deadline per slot; a client
that needs two independent deadlines declares two slot identities. The
proposed operations are `schedule_after<Slot>(integer_duration)`,
`cancel<Slot>()`, and `rearm<Slot>(integer_duration)`, each reporting whether
the request was accepted. A slot identifies its resulting catalog event and
normal `Route`, so its handler is an ordinary `on_event<Event, Context>()`.

The earliest due deadline controls the hardware compare/alarm. The ISR
acknowledges the alarm and marks the scheduler ready; the owning context
drains all due slots, posts their events according to `Elide` or `Stream`,
then arms the next deadline. Equal deadlines use stable slot identity as a
tie-breaker, independent of declaration order. Scheduling a new earlier
deadline updates the alarm safely. Cancellation removes a deadline that is
still in the deadline set, even if the timer has signalled a wake but the
context has not drained it. Once its due event is posted to the dispatch
queue, cancellation reports `already_fired` if no newer deadline is pending
and does not retract that event. Rearming after an event was posted creates
a new deadline; cancelling that newer deadline leaves the already-posted
event intact.
If several deadlines pass before the context runs, they remain due and are
handled later; callback lateness is observable and is not presented as
hardware jitter. A too-short-to-arm request is treated as already due and
wakes the context. Delays beyond one counter cycle require an explicit
integer-clock extension or a stated maximum horizon. Wraparound, clock
changes and low-power sleep need backend-specific rules before those
capabilities are offered. Periodic fixed-rate and fixed-delay requests,
missed-period counting, and catch-up policy are subsequent extensions.

The timer's mode, compare channel, IRQ source and PWM coexistence remain in
the deterministic allocator's capability problem. A task context cannot
make an impossible hardware combination valid. No AVR timer feature is
generalized to classic ESP32 merely because both can schedule a deadline.

## Generator and validation changes

The current IRQ generator discovers only `on_interrupt<Event>()` and emits a
direct ISR call. Event dispatch uses standard C++ templates and needs no
second generator. Integration extends the existing probe/plan/emit chain:

1. Derive the finite event and context catalogs from the module closure.
   Detect visible `on_interrupt` and `on_event` specializations; reject
   duplicate routes, invalid mode/context combinations and two handlers for
   the same event before allocation.
2. Treat each selected handler as one interrupt demand when its event comes
   from a hardware source. Include route, delivery mode, context identity,
   capacity requirement and source binding in the canonical object record and
   JSON plan. Software-only posts do not invent a hardware binding.
3. Validate the target's context inventory and the same deterministic
   peripheral allocation before emission. The existing target emitter calls
   `dispatch_bound_interrupt<Event, Backend>()` for each selected binding;
   the C++ template selects direct or deferred handling. Continue to treat
   generated C++ as write-and-forget output; verify current-attempt hashes
   and identities at build consumption.
4. Make strict compilation reject a stale route, handler, capacity, target
   context mapping or source binding. A missing generated bridge cannot
   silently turn an event into a no-op. Preserve the current one-source
   guard and startup failure behavior.

The current emitter handles one logical event per physical source. This
proposal does not silently lift that limit: a later shared-source adapter
must still own one status snapshot, acknowledgement and entry registration,
then distinguish and post each logical event without letting two consumers
read the same device FIFO.

Mock tests should cover queue order, equal-time deadlines, declaration-order
independence, elision while queued and while a handler runs, stream overflow,
queue recovery, cancel/rearm races, nested posting, concurrent producers,
startup-pending delivery and failed context creation. Named AVR compiler and
simavr checks should bound ISR work and prove publication to the loop; named
ESP32 compile/link checks should prove task/ISR API selection and storage.
Physical silicon validation remains on hold. A passing host test or target
link does not establish physical interrupt latency or cross-core behavior.

## Design limits before implementation

The interface above fixes the semantic split and discovery model. The exact
context declaration spelling, `PostResult` type, dispatch budget units,
queue record representation and timer duration/horizon types still require
small mock and named-target probes. Those are implementation choices if they
preserve this contract; changing whether an event may have multiple
consumers, whether a firing is elided or streamed, or which context invokes
the handler is an API decision requiring review. The first implementation
should use `MainLoop` on mock and AVR, then classic ESP32; a named ESP32 task
is an explicit extension after its lifecycle and concurrency proof.
