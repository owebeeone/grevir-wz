# GrevirEsp32DeferredDispatchCheckpoint — Code Review 3

**Review object:** ESP32 deferred dispatch, root `e242a92..17f0f05`, including shared Core and the classic ESP32 adapter.  
**Baseline:** The controlling draft is [GrevirEsp32DeferredDispatchCheckpoint.md](/Users/owebeeone/limbo/grevir-wz/dev-docs/GrevirEsp32DeferredDispatchCheckpoint.md).  
**Date:** 2026-09-28. **Axis:** Code architecture, interfaces, call graphs, and generated-policy agreement.  
**Verdict: STOP.** One **P2 new architectural root cause** remains. The second remediation closed the prior identity collision, but introduced a valid-policy serialization failure. The two-round review cap calls for a redesign decision.

| Prior finding | Closure assessment |
| --- | --- |
| Code P2: omitting `MainLoopContext` allowed foreign-task dispatch | **Closed.** [DeferredContextPlan](/Users/owebeeone/limbo/grevir-wz/grevir-core/src/grevir/interrupt/binding.hpp:166) requires the policy for a selected deferred route, and [the queue](/Users/owebeeone/limbo/grevir-wz/grevir-core/src/grevir/event/queue.hpp:31) uses it unconditionally. The supplied negative ESP32 probe and host ownership tests support closure. |
| Code-2 P2: `lock + "_" + context` gave distinct policies the same identity | **Closed for the reported counterexample.** [The new encoding](/Users/owebeeone/limbo/grevir-wz/grevir-core/src/grevir/event/context_policy.hpp:34) uses component lengths. The checked generated fixtures contain `l10_alpha_beta_c5_gamma` and `l5_alpha_c10_beta_gamma` with different fingerprints; [strict output](/Users/owebeeone/limbo/grevir-wz/grevir-core/tools/grevir_irqgen/emit.py:111) compares the live policy. The checkpoint reports a stale-binding rejection. |

**Changed-range analysis:** Core changed the queue, context policy, deferred plan, handler, startup, and host test. The ESP32 adapter added task/ISR guards and a per-application loop-task owner. The root changed the example and documentation. The **NEW ARCHITECTURAL root cause** is a mismatch between the composite identity’s accepted length and the probe record’s one-byte text field, introduced by the final length-prefix remediation.

## 0 Evidence base

The repository tuple matched at the start and end: root `17f0f05f7e99bb831d5f052e85a9fbbcbfb2c70a`; grevir-core `a711cb78a4276222e135cfd806959aaf2e391bd1`; grevir-arduino-esp32 `6b5a1776d2c2eba1fa0e00729c5f9e3ac5a79e5f`; grevir-avr `322e8b9fc520d7ba60b46104b14f86e2eaf79c9c`; grevir-test-support `b13fea29e82d813f2ae2f6719b0adb2fb6a8cb45`. The checked repositories were clean.

This was read-only source and artifact inspection under [AGENTS.md](/Users/owebeeone/limbo/grevir-wz/AGENTS.md), [AGENTS_GWZ.md](/Users/owebeeone/limbo/grevir-wz/AGENTS_GWZ.md), the [cross-MCU](/Users/owebeeone/limbo/grevir-wz/dev-docs/review-policies/CrossMcu.md) and [ESP32](/Users/owebeeone/limbo/grevir-wz/dev-docs/review-policies/Esp32.md) policies, and the declarative integration Dos/DontDos. I ran no builds. The draft reports 192 native CTests, a mock generator round trip, and staged Uno and classic ESP32 compile/link; these are supplied results, not independently rerun evidence. Silicon behavior remains unverified.

## 1 Findings

**P2 — A valid deferred policy can no longer pass probe serialization.** [Component validation](/Users/owebeeone/limbo/grevir-wz/grevir-core/src/grevir/interrupt/binding.hpp:192) applies `valid_component`, which accepts arbitrarily long alphanumeric or underscore identities. [ContextPolicyIdentity](/Users/owebeeone/limbo/grevir-wz/grevir-core/src/grevir/event/context_policy.hpp:36) permits a composite up to 65,535 characters. The [probe writer](/Users/owebeeone/limbo/grevir-wz/grevir-core/src/grevir/interrupt/probe_record.hpp:75) instead rejects any text longer than 255 by entering an endless loop during constant evaluation.

**Trigger:** On a board with one selected `MainLoop` event, give `EventLock::identity` and `MainLoopContext::identity` 123 valid characters each. The prior joined identity was `123 + 1 + 123 = 247` characters and fit the probe field. The new length-prefixed identity is `257` characters. Both components and the composite pass the current C++ validity checks, but [probe emission](/Users/owebeeone/limbo/grevir-wz/grevir-core/src/grevir/interrupt/probe_record.hpp:89) cannot complete.

**Scope and provenance:** Shared deferred-plan/probe contract, reachable by an ESP32 board-defined policy and by other deferred targets. This is a correctness regression introduced by Core `a711cb7`, not an inherited timer or AVR cost issue. **Impact:** A previously representable application cannot reach canonical planning, generation, or strict compilation, despite satisfying the advertised component checks. **Correction direction:** Make the policy representation and record width agree across C++, schema, protocol, and strict output. Separate component fields or a widened record field would retain both unambiguous identity and the formerly representable case. **Closure test:** Exercise composite lengths 255, 256, and 257; require successful probe, plan, and strict compilation for valid accepted identities, plus the original collision pair and stale-binding rejection.

## 2 Invariant analysis

The selected example retains one handler specialization as interrupt demand. Probe serialization carries deferred capacity and policy; the emitter’s ESP32 callback reaches `dispatch_bound_interrupt`, which calls `post_from_isr`. Startup prepares the queue before installing and enabling the source, and reports `event_context_failed` if preparation fails. Task and ISR publication use the corresponding guards on one ESP32 port mux. Dispatch checks the bound task and invokes callbacks after releasing the lock. These inspected paths support the checkpoint’s narrow classic ESP32 `MainLoop`/`Elide` route.

The length-prefix format is injective for validated components. The failure occurs at the next boundary: the plan accepts a policy string that its probe format cannot represent. No evidence indicates that the reported 257-character trigger could silently accept stale generated output; it fails earlier.

## 3 Risks and next action

The current example’s short identities fit the record, so the supplied target compile/link evidence does not cover this boundary. Cross-core timing, physical interrupt delivery, S2/S3, named FreeRTOS contexts, software events, and general timer ownership remain outside this review.

Record **STOP** under the stated two-remediation-round cap. Resolve the identity/serialization contract as a design decision before another patch or acceptance review.
