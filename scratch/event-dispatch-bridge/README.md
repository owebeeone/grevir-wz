# Event dispatch bridge proof of concept

This is a dispatch-only experiment. The proposed catalog-driven activation
path is in
[the event activation design](../../dev-docs/GrevirEventActivationDesign.md).

This is a standard C++23 host proof with no event-code generator. A
`MockInterruptSource<Event>` calls `dispatch_bound_interrupt<Event, MockBackend>()`.
That template detects whether the application specialized the existing
`on_interrupt<Event>()` or the proposed `on_event<Event>()`, rejects
zero or two handlers, and chooses one route. The event bridge invokes an
`IsrLevel` handler directly, or asks the backend to queue a
`void (*)(void*) noexcept` callback and `void*` argument for a `MainLoop`
handler. The backend owns the per-event pending mark and clears it as the
record is dequeued. All of this is ordinary C++ instantiation. The proof
uses Grevir's real deleted `on_interrupt` primary template.

From the workspace root, run:

```sh
c++ -std=c++23 -DGREVIR_IRQ_PROBE \
  -Igrevir-base/src -Igrevir-core/src \
  scratch/event-dispatch-bridge/main.cpp \
  -o /tmp/grevir-event-bridge-poc
/tmp/grevir-event-bridge-poc
```

Three deliberate compile failures check the boundary. `missing_handler.cpp`
has no handler, while `conflicting_handlers.cpp` provides both kinds. Each
fails the dispatcher's exactly-one-handler assertion. `address_fail.cpp`
fails because Grevir's real
`on_interrupt<DirectTick>` remains deleted; taking its address does not
create a specialization.

```sh
c++ -std=c++23 -DGREVIR_IRQ_PROBE -fsyntax-only \
  -Igrevir-base/src -Igrevir-core/src \
  scratch/event-dispatch-bridge/missing_handler.cpp
c++ -std=c++23 -DGREVIR_IRQ_PROBE -fsyntax-only \
  -Igrevir-base/src -Igrevir-core/src \
  scratch/event-dispatch-bridge/conflicting_handlers.cpp
c++ -std=c++23 -DGREVIR_IRQ_PROBE -fsyntax-only \
  -Igrevir-base/src -Igrevir-core/src \
  scratch/event-dispatch-bridge/address_fail.cpp
```

`MockBackend` is single-threaded and deliberately has no ISR/task
synchronization. The common bridge delegates posting, coalescing and dequeue
state to the backend. This proof establishes C++ instantiation and mock
dispatch, not AVR or ESP32 concurrency. The existing Grevir IRQ generator
still binds real hardware vectors. Integrating this mechanism would make its
generated entry call the C++ dispatcher and make its C++ demand probe accept
either handler kind; event dispatch needs no second generator. Today, an
`on_event` specialization alone does not cause Grevir to emit an ISR: the
production demand probe checks only `on_interrupt`.
`GREVIR_IRQ_PROBE` allows this host proof to include the current Grevir
interrupt header without a generated strict binding gate.
