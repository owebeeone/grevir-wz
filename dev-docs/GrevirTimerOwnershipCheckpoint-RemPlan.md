# Timer ownership checkpoint — first review disposition

Status: **NO-GO; remediation round 1 is open**. The settled review tuple and
three verbatim reports are recorded in
[the checkpoint](GrevirTimerOwnershipCheckpoint.md),
[Code](GrevirTimerOwnershipCheckpoint-ReviewCode.md),
[State](GrevirTimerOwnershipCheckpoint-ReviewState.md), and
[Surface](GrevirTimerOwnershipCheckpoint-ReviewSurface.md). This is a review
artifact, not a new timer implementation plan or a change to the agreed
one-module/one-timer decision.

| Finding | Disposition and correction | Closure evidence |
| --- | --- | --- |
| Code P0-1 board pin alias | Accept the collision. The report's `41` calculation omitted the extracted I/O offset (`rrPORTB::addr` is `0x25`); the canonical PB1 ID is `297`. That arithmetic detail does not affect the conflict. Normalize Arduino board pin claims and device pin claims at the Core conflict boundary and during reservation. | PB1 PWM plus Uno D9 rejects before effects in both orders; an unrelated pin composes; same checks on Nano mapping. |
| Code P1-1 foreign writable binding | Accept. Bind each requested module to an owner-scoped allocation view. A module with no owner demand must not see another owner's `Pwm` update method. | Positive owner compile and negative foreign-binding compile probe. |
| Code P1-2 PWM-shaped owner boundary | Accept as the principal structural blocker. Replace the `Own` wrapper over `pwm::Instance` with a use-neutral owner demand and candidate/plan envelope. PWM-specific validation belongs only in its use adapter; event-only and joint PWM/event mock candidates must be representable even before their AVR driver exists. Do not add an `EventUse` token that is silently ignored. | Event-only mock without a pin selects; compatible PWM plus period event selects one candidate; conflicting register roles reject. Existing AVR PWM remains one-owner/one-payload. |
| Code P2-1 order-derived candidate IDs | Accept. Identity must derive from normalized timer/mode/divider/TOP and owner/use key, not traversal count. | Permuted capability inventory yields identical selected plan IDs. |
| Code P2-2 contradiction diagnostic | Accept. Differentiate impossible exact-timer/width intersection from ordinary unsupported PWM. Preserve module/target/timer/width as structured diagnostic data exposed at compile gate. | Distinct static diagnostic assertions and negative compiler probe. |
| State P1-1 dependency setup order | Accept. Selected timer setup must run after prerequisites' setup and before owner/consumer setup through dependency-aware dispatch. | Mock dependency sets readiness in `runSetup()`; every timer write observes ready in either descriptor order. |
| State P2-1 custom parameter hooks | Accept. Preserve `Impl::paramsSetup()` and `Impl::paramsLoop()` once while adding selected-owner operations; do not replay original params. | Counter fixture sees one call of each hook, one owner setup. |
| Surface P2 two-use walkthrough | Accept. Publish a two-output one-owner example with both bindings and compare/TOP checks. | Compile and run published example; surface re-review can complete first-day walkthrough. |
| Surface P3 evidence label | Accept as a small documentation correction. | Support page says precisely whether the two-use owner path has host, target compile, simulation or silicon evidence. |

The principal design risk is Code P1-2: patching the PWM model with optional
event fields would preserve the wrong common abstraction. The first round must
establish a genuinely use-neutral owner envelope before accepting local fixes.
The review loop allows at most two remediation rounds; any new architectural
root cause found in a third round stops this lane for redesign-or-accept.
