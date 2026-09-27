# Grevir event dispatch checkpoint — CODE-AXIS REVIEW

**Review object:** First `MainLoop`/`Elide` dispatch slice, each checkpoint commit against its immediate parent.  
**Baseline:** Exact five-repository tuple below, verified at review start and end; all five working trees were clean at both checks.  
**Date:** 28 September 2026, Australia/Sydney.  
**Axis:** CODE — architecture, interfaces, call graphs and compatibility.  
**Verdict:** **NO-GO: one P2 plan-fidelity defect.** The handler-to-queue causal chain is present, but the new queue-capacity decision is outside the canonical plan and its strict agreement checks.

## 0 Evidence base

| Repository | Reviewed commit | Parent |
| --- | --- | --- |
| Workspace root | `f1acfc67d87efd632acc281e779c80404b580f09` | `4b1028efb95d07f650d480c5065da088ec2effbb` |
| grevir-core | `95b9c438bef5f07946bb51a066a3f77c7388758b` | `852a38aedf9bc7bc7569e971b91d4a8d08821b11` |
| grevir-test-support | `c6a618d549ac61e6c9fc51f8e15e5a718914a07c` | `468ce73de33bc641d0e8bc60ecdaa43655ce9de4` |
| grevir-peripherals | `4612cecdc99c22f326a0c1498846fea34c0c596c` | `0cf6a7d7d3358dcb3df9474c8e190d5217029e69` |
| grevir-avr | `032f907fe430408c2d214431b68162cff1c85977` | `9f6bc02cc232d8a60a7b16a1ecdf16bfc8c42e0a` |

This was an independent, read-only source review. No files or Git state were changed; no builds or runtime tests were run. Findings below are source deductions with concrete reproduction recipes, not claims of newly executed compiler tests. No other reviewer's report was consulted.

Controlling material read:

- `AGENTS.md` and `AGENTS_GWZ.md`.
- `dev-docs/GrevirEventActivationDesign.md`, with its replacement of the old handler syntax respected.
- `dev-docs/GrevirEventContextsAndDispatchDesign.md`, for queue semantics and plan agreement.
- `dev-docs/GrevirDeclarativeIntegrationDos.md` and `GrevirDeclarativeIntegrationDontDos.md`.
- `dev-docs/review-policies/CrossMcu.md`, plus `Avr.md` and `Esp32.md`.

Inspected implementation includes the new route and queue headers, interrupt handler/startup integration, demand and binding representation, probe serialization, Python protocol and emitter, both lock implementations, runtime queue fixture, affected examples and public guide, and the compatibility substitutions in Core and Peripherals.

The shared API was assessed under CrossMcu. AVR-specific reasoning concerns ATmega328P with the documented 16-bit `int` ABI; it does not establish instruction counts, stack cost or silicon behavior. Classic ESP32 was assessed only at the supported direct-route and unsupported deferred-route boundary. Existing reported compiler/simavr results were not rerun or independently re-established. Physical silicon validation remains held.

## 1 Findings

### CODE-01 — P2: Queue capacity bypasses the canonical plan and strict-build agreement

**Location:** `grevir-core/src/grevir/event/queue.hpp:28`, where effective capacity is read directly from the current `Board`; `grevir-core/tools/grevir_irqgen/emit.py:88–97`, where the generated binding captures handler, context and delivery but no queue-capacity decision.

**Scope/classification:** Shared contract; demonstrated plan-fidelity defect affecting the mock and AVR deferred implementation. No AVR-specific cost assumption is involved.

**Provenance:** Introduced by this checkpoint. The parent rejected deferred dispatch. This change makes capacity determine accepted-versus-dropped events without extending the previously direct-oriented plan representation.

**Violated invariant:** A queue's capacity and storage are fixed by the application plan, and strict compilation rejects capacity disagreement. These requirements appear in `GrevirEventContextsAndDispatchDesign.md:186–187` and `:342–354`. They also implement Dos §7, “expose the plan,” and §8, “keep lowering faithful to one validated plan,” and DontDos §5, “do not let applied behavior diverge from the selected plan.”

**Evidence:**

- `queue.hpp:28–30` reads and validates only the capacity visible in the consuming translation unit.
- `interrupt/catalog.hpp:42–49` stores event identities, handlers, contexts and deliveries in `DemandSummary`; capacity is absent.
- `interrupt/probe_record.hpp:82–95` serializes those identities and route fields, without capacity.
- `tools/grevir_irqgen/protocol.py:65–68` and `:85–87` require exact plan/demand field sets that likewise contain no capacity.
- Generated bindings in `emit.py:88–97` and dispatcher checks in `interrupt/handler.hpp:82–88` cannot compare a queue-capacity value that was never recorded.

**Concrete reproduction:**

1. Use a valid deferred mock application with two distinct catalogued event handlers and two independently bindable sources.
2. In the same application header, select `Board::event_queue_capacity = 2` under `GREVIR_IRQ_PROBE` and `1` otherwise. Keep source bindings, event routes and all board identity strings unchanged.
3. Run the normal probe/emitter/strict sequence.
4. The probe and strict compilation agree on every currently serialized field. Both capacities are individually in range, so no queue or generated-binding assertion detects the disagreement.
5. Start the application and fire A then B before dispatch. Strict firmware accepts A and drops B with `full`/overrun because its effective capacity is one.

This is also an inspection failure without conditional compilation: otherwise identical capacity-one and capacity-two applications produce no semantic queue-capacity difference in their canonical interrupt plans.

**Impact:** A behavior-changing bounded-resource decision is invisible in the reviewable plan and unchecked across the probe/strict seam. The normal full-queue diagnostic does not explain that the queue differs from the probed configuration. File freshness alone cannot close the conditional-compilation reproduction because the header itself need not change between phases.

**Remedy:** Add the effective deferred-context configuration, at least capacity and an explicit implementation/policy identity, to the C++-derived plan. Serialize and fingerprint it, then emit strict assertions against the current configuration. Validate the capacity during discovery as well as when instantiating runtime storage. Keep the generator a lowering stage.

**Regression/closure test:** Add a generated mock round trip with at least two events and capacity two; verify the capacity is visible in the canonical plan. Add an unchanged-header probe/strict capacity mismatch and require a dedicated stale-context/capacity diagnostic. Verify a deliberate capacity change changes the semantic plan/fingerprint. Preserve valid zero-demand and direct-only cases without requiring deferred storage.

## 2 Invariant analysis

### Handler specialization derives the binding and queue path

The intended chain is real:

1. The module closure supplies event types through `EventCatalog`.
2. `DemandData` probes `HandlerChoice<Event>` and records the visible specialization.
3. Existing allocation and binding validation require a matching physical binding.
4. The emitter creates one entry per validated source.
5. `dispatch_bound_interrupt<Event>()` checks handler kind, context and delivery.
6. The new `Binding::Application` alias selects the application-specific queue.
7. Deferred dispatch posts through `post_from_isr<Application, Event>()`; dequeue invokes the specialized handler.

The application does not supply another event list, vector table or forwarding raw handler for this path. The new emitter alias is a derived application association, not an independently authored binding.

### Board capability/policy boundary

An `EventLock` supplied by the board is a reasonable way to expose the target's critical-section policy for this narrow slice. The new Board declarations do not themselves program timers or copy peripheral configuration into another coordination hook. Queue mechanics remain in `MainLoopQueue`, and the existing startup coordinator calls its lifecycle operations.

`event_queue_capacity` is also a legitimate explicit bounded-storage choice rather than a duplicate event registration. Its defect is the missing plan representation and agreement check in CODE-01.

The implementation does not yet establish the broader context-owning dependent-module architecture described in the older design. Startup detects capability members rather than deriving a selected context owner. That architectural limit should remain explicit; it is not evidence that the inherited timer-provider ownership problems have been repaired.

### Zero, one and many events

- **Zero:** An absent specialization creates no interrupt demand. A queue-capable Board nevertheless causes `start()` to instantiate and prepare its queue even when no deferred demand exists (`interrupt/start.hpp:28–30`). This is capability-driven initialization, not demand-derived context selection. No retained-byte or target-cost finding is claimed without measurement.
- **One:** The examples exercise the intended single-event route and explicit loop dispatch.
- **Many:** The queue is indexed by application and pending state by application/event. It does not hard-code the example's event type or source count. Different events can occupy distinct records and are drained in enqueue order. Source sharing remains deliberately rejected by the existing validator/emitter.
- **Capacity limit:** The generic implementation explicitly rejects capacities above 255 and uses byte indices. This is a current implementation limit, not a cross-MCU semantic necessity. No claim is made that a wider generic API must inherit that AVR-friendly representation.

The existing single-source AVR inventory and board configuration indexing are inherited restrictions. This checkpoint does not justify attributing them to event dispatch or treating their repair as a prerequisite added by this review.

### Strict rejection and unsupported routes

Handler kind, context and delivery are checked in `dispatch_bound_interrupt`. Duplicate raw/event handlers remain rejected by `HandlerChoice`. Posting checks exact catalog type identity and only accepts `MainLoop`/`Elide`.

`Stream` remains recognizable metadata but fails when the unimplemented queue path is instantiated. A default deferred event on the existing ESP32 board fails the missing-context check; this checkpoint does not implement ESP32 queue synchronization merely by exposing generic queue templates. Explicit `IsrLevel`/`Direct` remains separate from queue dispatch.

Malformed or declaration-only handler cases were not newly compiled in this review. The controlling activation document appropriately distinguishes the specialization's visibility from its eventual definition/link obligation.

### Queue and backend boundaries that held under source inspection

Queue publication, coalescing, capacity checking and pending marking occur under the same Board lock. Dequeue clears the pending mark before calling application code and releases the lock before invocation. A failed enqueue does not set the pending flag. No dynamic allocation or payload ownership is introduced by the queue.

The AVR lock keeps status-register assembly within an AVR-specific header and restores the saved interrupt state. The host mock lock supplies mutex serialization. These observations establish source structure only; they do not prove silicon timing, ISR cost or multi-consumer correctness.

The Core and Peripherals compatibility substitutions preserve their Boolean trait meaning while avoiding unavailable convenience aliases. They do not introduce a new timer ownership mechanism.

## 3 Risks and next action

Close CODE-01 with a bounded context-plan extension and an adversarial probe/strict mismatch test. This review can support **GO on the CODE axis after that fix and its targeted verification**, subject to any independent findings from the other axes.

Keep the remaining boundaries explicit: context-owner derivation is incomplete; generic queue capacity is currently limited to 255; the current target inventories remain narrow; and inherited timer/board ownership issues remain deferred rather than solved.

`Stream`, software-only discovery, ESP32 deferred dispatch, named contexts, deadline service and physical silicon validation were not treated as required completed outcomes. None was used to justify a speculative defect or an unauthorized validation run.

The exact reviewed tuple remained unchanged through the final verification.
