# GrevirTimerIntegrationRemediationPlan — SAFETY-AXIS RE-REVIEW

**Review object:** `dev-docs/GrevirTimerIntegrationRemediationPlan.md` at `a15fac26d8a83ab4b19e5db3bc90ca544f9ad93b`; proposed work plan, draft-stage re-review  
**Baseline:** Root HEAD `a15fac26d8a83ab4b19e5db3bc90ca544f9ad93b`, compared with `2c7d5dd821308225a8bd16d05697b4aa161268d3`. HEAD matched and the working tree was clean at the start and end of review.  
**Date:** 27 September 2026  
**Axis:** Safety: re-trace the four original configurations that could pass an unsafe gate. Independent, adversarial, read-only.

**Verdict: GO** — all four prior P2 findings are closed in the revised *plan*. This verdict does not accept an implementation or claim that its gates have run.

---

## Prior-finding closure table

| ID | Disposition claimed | Verified against the original counterexample | Status |
| --- | --- | --- | --- |
| P2-1 | Make canonical identities and alias conflict checks a Phase 2 prerequisite for the supported slice. | A PWM route to PB1 and an explicit claim through a board alias must normalize before selection; the Phase 2 gate requires the conflict in either declaration order (plan lines 163–165, 176–178). Phase 4 now addresses the broader inventory (lines 207–221). | Closed |
| P2-2 | Give an admitted shared configurable domain one selected setup owner, or reject it while unsupported. | Two timers requiring D=8 cannot each write D: the plan requires one owner across setup, update and cleanup, and its gate rejects independent writes for agreeing or conflicting divider requirements (lines 132–145, 166–169, 178–180). | Closed |
| P2-3 | Check applied effects against independent timing and capability expectations. | Two different but equally wrong TOP programs no longer satisfy the gate. It requires independently calculated realized timing and capability, TOP-boundary checks, and rejection of an incompatible interrupt mode/source (lines 171–176). | Closed |
| P2-4 | Test resident declarations without the nonresident SDK and test inactive-section invariance. | An AVR declaration that pulls in an ESP32 SDK cannot pass the new Phase 2 gate. Each resident target must compile without the other SDK, and changing only an inactive section must leave the active plan and diagnostics unchanged (lines 185–189). | Closed |

## Changed-range analysis

The reviewed revision changes the remediation plan and its two timer-design companions. It moves identity and shared-domain safeguards into the Phase 1 contract and Phase 2 implementation gate, strengthens Phase 2’s independent behavioral and cross-target evidence, and clarifies that separate timer-owning modules cannot use the older `SameTimer` grouping. The allocation design now describes each provider module instance as one allocation unit (`GrevirTimerAllocationDesign.md`, lines 254–289); the examples require consumers to bind to one provider’s offers without second physical claims (`GrevirTimerApiExamples.md`, lines 221–249).

I found no **NEW ARCHITECTURAL** safety root cause in the changed range. The first capability slice and plan representation remain Phase 1 decisions; the revision now requires the gate to reject unsupported shared-domain configurations rather than silently accept them.

## 0. Evidence base

I inspected the committed diff from `2c7d5dd821308225a8bd16d05697b4aa161268d3` to `a15fac26d8a83ab4b19e5db3bc90ca544f9ad93b` for `GrevirTimerIntegrationRemediationPlan.md`, `GrevirTimerAllocationDesign.md` and `GrevirTimerApiExamples.md`; read the merged `GrevirTimerIntegrationRemediationPlan-RemPlan.md` and prior safety report; and inspected numbered lines of the revised plan and relevant companion sections. `git rev-parse HEAD` and `git status --porcelain` verified the exact clean tuple at both ends. No files were changed and no tests or builds were run.

## 2. Invariant analysis

The PB1 alias collision now fails before selection for the supported slice. Agreeing uses of a shared divider have one writer or are rejected; conflicting settings cannot pass as independent timer setup. Applied timer settings must be checked against independently derived expectations, and inactive target declarations must be usable without the other target’s SDK. These requirements close the four paths that previously could satisfy Phase 2’s written gate while violating the selected plan.

The revised gate remains evidence to be produced during implementation. This review verifies that the plan demands it; it does not verify register behavior, toolchains or physical hardware.

## 3. Risks and next action

Physical timing and silicon behavior remain under the stated validation hold. Implement Phase 1’s reviewed contract, then require the revised Phase 2 checks before treating the first supported timer slice as complete.
