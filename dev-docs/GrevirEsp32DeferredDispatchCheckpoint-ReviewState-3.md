# Grevir ESP32 deferred dispatch checkpoint — State review 3

**Review object:** Implementation from root base `e242a92` through `17f0f05`, assessed against the controlling draft at root HEAD.  
**Baseline:** Root `17f0f05f7e99bb831d5f052e85a9fbbcbfb2c70a`; Core `a711cb78a4276222e135cfd806959aaf2e391bd1`; Arduino ESP32 `6b5a1776d2c2eba1fa0e00729c5f9e3ac5a79e5f`; AVR `322e8b9fc520d7ba60b46104b14f86e2eaf79c9c`; Test Support `b13fea29e82d813f2ae2f6719b0adb2fb6a8cb45`. The tuple matched at the start and end of this read-only review; `git status --short` was empty both times.  
**Date and axis:** 2026-09-28; State — races, lock scope, task ownership, queue lifecycle, and failure behavior.  
**Verdict:** **GO** for the documented classic ESP32 Dev Module, Arduino-ESP32 3.3.11, `setup()` → `loop()` path. No P0–P2 state defect or **NEW ARCHITECTURAL root cause** was found. This is a source-level verdict; physical and cross-core behavior remain unverified.

| Prior finding | Closure assessment |
| --- | --- |
| State P3: backend-wide owner could be rebound while a queue was active | **Closed.** [MainLoopContext](/Users/owebeeone/limbo/grevir-wz/grevir-arduino-esp32/src/grevir/arduino_esp32/event_context.hpp:38) stores ownership per application `Spec`. [prepare()](/Users/owebeeone/limbo/grevir-wz/grevir-core/src/grevir/event/queue.hpp:37) checks `ready_` and `dispatching_` under the same lock used for binding. An active queue retains its owner and records; a stopped queue can bind a new owner after any admitted dispatch finishes. The supplied host test exercises active reprepare rejection and transfer after stop. |
| State effect of the prior optional-context defect | **Closed.** [DeferredContextPlan](/Users/owebeeone/limbo/grevir-wz/grevir-core/src/grevir/interrupt/binding.hpp:166) requires a context policy for a selected deferred route. The queue uses that policy unconditionally for binding and dispatch admission. The supplied missing-policy probe and foreign-task host test support this source conclusion. |

**Changed-range analysis — NEW ARCHITECTURAL root cause: none found.** The final Core change length-prefixes lock and context identities in [ContextPolicyIdentity](/Users/owebeeone/limbo/grevir-wz/grevir-core/src/grevir/event/context_policy.hpp:21). It changes compile-time policy identity and generated-artifact freshness; it does not alter queue state transitions, ESP32 locking, or task ownership. The supplied collision and stale-header checks address the prior identity finding. The earlier Core and ESP32 changes establish the guarded queue and per-`Spec` owner assessed here. The example starts in `setup()` and drains in `loop()`.

## 0. Evidence base

I read `AGENTS.md`, `AGENTS_GWZ.md`, the CrossMcu and Esp32 policies, the declarative Dos/DontDos, both prior State reports, both remediation plans, the checkpoint draft, event design, relevant root and member diffs, and the queue, startup, ESP32 adapter, timer, tests, example, and public guide. I made no edits and ran no builds. The documented 192 host tests, generator round trip, and staged Uno and classic ESP32 compile/link results are supplied validation, not independently rerun evidence.

## 1. Findings

**P0–P3: none.**

## 2. Invariant analysis

[ESP32 TaskGuard and IsrGuard](/Users/owebeeone/limbo/grevir-wz/grevir-arduino-esp32/src/grevir/arduino_esp32/event_context.hpp:16) enter the same static port mux with context-appropriate critical-section APIs. Queue publication writes a record and its Elide mark before incrementing count under that guard. [Dispatch](/Users/owebeeone/limbo/grevir-wz/grevir-core/src/grevir/event/queue.hpp:68) admits only the bound task, prevents a competing dispatcher, removes a record and clears its Elide mark under the lock, then invokes the callback after releasing it. A firing during the callback can therefore enqueue again. A full Stream queue sets overrun without claiming delivery.

[stop()](/Users/owebeeone/limbo/grevir-wz/grevir-core/src/grevir/event/queue.hpp:50) makes publication return `not_ready` and clears pending records under the lock. An already admitted callback may finish after stop, while `dispatching_` prevents preparation from transferring ownership until it finishes. [Startup](/Users/owebeeone/limbo/grevir-wz/grevir-core/src/grevir/interrupt/start.hpp:26) prepares the queue before enabling the source; on later setup failure it masks the source and stops the queue. A preparation failure reports `event_context_failed` without stopping the queue it did not acquire. The selected [TimerStartPolicy](/Users/owebeeone/limbo/grevir-wz/grevir-arduino-esp32/src/grevir/arduino_esp32/timer_group0_timer0.hpp:110) runs that startup body once, so replay cannot rebind the owner.

## 3. Risks and next action

The host mutex tests and target compile/link do not establish ESP32 cross-core timing, physical interrupt delivery, or task-handle lifetime behavior. The documented path keeps Arduino’s loop task alive; named FreeRTOS contexts, S2/S3, software events, notification-based dispatch, and silicon validation remain outside this checkpoint. The two-round review cap requires STOP if this review finds a third new architectural root cause; this State review found none. Proceed with the checkpoint’s remaining independent review and acceptance decision.
