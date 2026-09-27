# Event handler activation through the interrupt catalog

Status: direct and `MainLoop`/`Elide` activation implemented for mock and AVR;
`Stream`, software-only event discovery, and ESP32 deferred dispatch remain.
This replaces the handler-activation
mechanism in [Event contexts and deferred dispatch](GrevirEventContextsAndDispatchDesign.md).
That document's queue, context and deadline semantics remain separate questions.

## Required causal chain

For a hardware event, a user declaration must cause all of these steps:

1. The module closure offers an event type in `EventCatalog<Application>`.
2. The application defines one handler specialization for that type.
3. The existing compile probe sees the specialization and records an interrupt
   demand. No explicit handler list or second generator is supplied by the
   application.
4. The allocator selects a physical source, or compilation fails if no valid
   source exists.
5. The existing IRQ emitter creates the AVR vector or ESP32/mock registration
   entry for that source. Its entry calls one C++ dispatcher.
6. The dispatcher invokes a direct handler or posts to the selected deferred
   context. The strict build checks that the handler and route still match the
   generated plan.

The handler specialization cannot create an AVR `ISR(...)` definition by itself.
Standard C++ templates can select and instantiate the handler, but a target
entry still needs a concrete vector name or registration call. Grevir already
has a probe/plan/emitter for that purpose; this design extends it rather than
adding an event generator.

## Application syntax

The module provides the event type and stable key. `RouteFor<Event>` defaults
to `MainLoop`/`Elide`; an application may specialize that policy when it needs
another context or delivery mode, subject to the module's event capabilities.
For direct ISR handling, an application selects the direct route explicitly.
The specialization is in a header included by both
probe and strict builds:

```cpp
template <>
struct grevir::event::RouteFor<Motor::PeriodElapsed> {
  using Context = grevir::event::IsrLevel;
  using Delivery = grevir::event::Direct;
};

template <>
inline void grevir::on_event<Motor::PeriodElapsed>() noexcept {
  Motor::tick();
}
```

The deleted primary template makes an unspecialized call invalid. A visible
specialization makes `requires { on_event<Event>(); }` true and therefore
forms an interrupt demand for a catalogued hardware event. The application
need not write `on_interrupt<Event>()`, forward it, or list the handler again.
The default deferred route lets the common case use only the function
specialization on the mock and AVR backends. A handler
specialization declaration without a body still forms a
demand, but omitting its definition fails the final link. A local C++23 probe
confirmed both behaviors. The specialization must be reachable before the
probe and strict build instantiate the handler check.

## Existing pipeline changes

The probe tests both `requires { grevir::on_interrupt<Event>(); }` and
`requires { grevir::on_event<Event>(); }` for every catalog event. It rejects
simultaneous handlers and invalid routes, then places the event key in
`DemandSet<Application>`. The allocator consumes that demand set.

The canonical plan includes the handler kind, context identity and
delivery mode, not only the event key and physical binding. Strict compilation
must compare those values against the visible route policy. Changing
an `Elide` main-loop handler to an `IsrLevel` handler after the probe must fail
as stale output rather than silently retaining the old binding.

The emitter calls `dispatch_bound_interrupt<Event>()` in each target entry.
The dispatcher calls `on_event<Event>()` directly for `IsrLevel`/`Direct`.
Deferred `MainLoop`/`Elide` delivery uses a fixed-capacity application queue
on mock and AVR. The target entry posts one record; main-loop dispatch clears
the pending mark before invoking the handler. The queue is prepared during
startup before source enablement and stopped on startup failure. Its critical
section is supplied by the board's target policy. `Stream` and ESP32 deferred
dispatch still require target implementations.
The handwritten raw-interrupt route still calls `on_interrupt<Event>()`.
The backend owns queue publication, elision and wakeup; the handler declaration
does not pretend to solve target concurrency.

## Boundary that C++ cannot erase

The probe must *name* candidate event types to inspect their specializations.
Grevir's finite module-provided `EventCatalog<Application>` is that anchor.
An `on_event<UncataloguedEvent>` specialization elsewhere is not discoverable
through standard C++23 alone and cannot cause hardware allocation. If the
requirement is that any arbitrary specialization activates an ISR without a
catalog or application registration, that requirement needs a nonstandard
symbol scan, a separate registration list, or a different API. This design
deliberately requires the event to be offered by a module in the application
closure.

## Proof and remaining work

A focused proof runs the *existing* probe/plan/emitter on mock, AVR and ESP32.
The mock round trip shows that adding the function specialization creates one
source entry, removing it leaves no demand, dual handlers fail and a stale
route fails strict compilation. The generated mock `MainLoop`/`Elide` example
passes on macOS, Raspberry Pi and Windows/MSVC. The AVR staged Uno build
compiles and links the deferred route, and simavr observes its main-loop
callback. The ESP32 staged Arduino build has evidence only for an explicit
`IsrLevel`/`Direct` route; no silicon behavior is established.
Declaration-only, malformed and uncatalogued cases remain additional negative
checks. The earlier
`scratch/event-dispatch-bridge` experiment proves only C++ handler selection
after a source already exists.
