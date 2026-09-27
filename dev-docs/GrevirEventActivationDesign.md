# Event handler activation through the interrupt catalog

Status: direct-route activation implemented; deferred backend dispatch remains.
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
Until the deferred queue backend is implemented, a working application selects
the direct route explicitly. The specialization is in a header included by both
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
Once deferred delivery is available, the default route will let the common
case use only the function specialization. A handler
specialization declaration without a body still forms a
demand, but omitting its definition fails the final link. A local C++23 probe
confirmed both behaviors. The specialization must be reachable before the
probe and strict build instantiate the handler check.

## Existing pipeline changes

The probe currently tests only `requires { grevir::on_interrupt<Event>(); }`
in `grevir-core/src/grevir/interrupt/demand.hpp`. For every catalog event it
should also test `requires { grevir::on_event<Event>(); }`, validate the
selected route and callable,
and reject simultaneous handwritten `on_interrupt` and `on_event` handlers.
Only then does it put the event key in `DemandSet<Application>`. The allocator
already consumes that demand set.

The canonical plan includes the handler kind, context identity and
delivery mode, not only the event key and physical binding. Strict compilation
must compare those values against the visible route policy. Changing
an `Elide` main-loop handler to an `IsrLevel` handler after the probe must fail
as stale output rather than silently retaining the old binding.

The emitter calls `dispatch_bound_interrupt<Event>()` in each target entry.
The dispatcher calls `on_event<Event>()` directly for `IsrLevel`/`Direct`.
Deferred `MainLoop` delivery is retained as the default route in the API and
canonical plan but currently fails the strict build with
`GREVIR_IRQ_DEFERRED_BACKEND_NOT_IMPLEMENTED`. Its queue and target publication
mechanisms are the next implementation stage.
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
route fails strict compilation. The AVR and ESP32 staged Arduino builds prove
target compile/link for explicit `IsrLevel`/`Direct` routes; they do not prove
silicon behavior. Declaration-only, malformed and uncatalogued cases remain
additional negative checks. The earlier
`scratch/event-dispatch-bridge` experiment proves only C++ handler selection
after a source already exists.
