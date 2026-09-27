# GrevirTimerIntegrationRemediationPlan — CONSISTENCY-AXIS REVIEW

**Review object:** `dev-docs/GrevirTimerIntegrationRemediationPlan.md` at `2c7d5dd821308225a8bd16d05697b4aa161268d3`, proposed work plan, 27 September 2026  
**Baseline:** Root Git HEAD `2c7d5dd821308225a8bd16d05697b4aa161268d3`; clean working tree. The plan was read with `git show` and numbered working-tree lines; the controlling documents were read with `nl -ba`.  
**Date:** 27 September 2026  
**Axis:** Consistency against the controlling design graph. Independent, adversarial, read-only; this report does not rely on the parallel review.

**Verdict: NO-GO** — two P2 findings block. I pre-commit to GO on a revision that resolves P2-1 and P2-2 as specified.

---

## 0. Evidence base

I inspected `AGENTS.md`, `AGENTS_GWZ.md`, the review-loop skill and reviewer template; the complete plan; relevant sections of `GrevirTimerAllocationDesign.md` (lines 1–56, 73–98, 242–351, 358–425), `GrevirTimerApiExamples.md` (lines 1–147, 210–281), `GrevirInterruptBindingArchitecture.md` (lines 1–163, 165–355, 466–512), both declarative integration documents, and the CrossMcu, Avr and Esp32 policies. Repository-wide document searches used `rg -uu`. `git rev-parse HEAD` returned the specified SHA at both the start and end; `git status --porcelain` was empty at both checks. I made no changes or build attempts.

## 1. Findings

### [P2-1] Explicit cross-module sharing remains a competing design instruction

**Location and invariant.** The plan makes a module instance the sole physical timer owner, requires other modules to consume offers from that provider, and defers automatic cross-module sharing (lines 23–35, 137–140). The cited allocation design instead treats an *explicit sharing group* as one allocation unit and configuration owner whose members may be requests from different module instances (`GrevirTimerAllocationDesign.md` lines 242–264, 322–326). Its worked `SameTimer<RequestRef<"left", "pwm">, RequestRef<"right", "pwm">>` example groups requests declared by two separate `MotorModule` instances (`GrevirTimerApiExamples.md` lines 81–115, 210–240). Explicit `SameTimer` is not the automatic sharing that the plan says it defers.

**Counterexample and impact.** An implementer following the still-current worked example can accept `SameTimer` over `left/pwm` and `right/pwm` with a synthetic group owner. That passes the example’s expected result of one timer owner and two endpoints, while violating the plan’s requirement that an actual provider module from the application closure own the timer and offer endpoints to dependents. Conversely, an implementer following the plan must reject or redesign that example. The documents currently give incompatible acceptance criteria for the same input.

**Required correction.** State precisely that the plan supersedes the allocation design’s cross-instance group ownership and the worked `SameTimer` example, or revise those documents to show provider-module composition. Distinguish explicit cross-module groups from automatic co-location.

**Closure test.** Walk through the two `MotorModule` instances in the existing example under the revised documents. They must yield one unambiguous answer about who owns and configures the timer, whether the original `SameTimer` spelling remains valid, and how each consumer obtains its endpoint.

### [P2-2] Phase gates can accept an incomplete solver as an allocation failure

**Location and invariant.** Phase 1 requires a two-owner greedy-trap case (plan lines 113–125), but its exit gate asks only for a coexistence counterexample (lines 127–133). Phase 2 requires deterministic output for zero, one and two owners (lines 150–154). Neither gate settles completeness over declared candidates, a supported search envelope, or a distinct search-exhaustion result. The controlling design requires a complete assignment whenever one exists in the supported candidate model and expressly separates resource-limited exhaustion from unsatisfiability (`GrevirTimerAllocationDesign.md` lines 84–89, 347–351, 410–425); the declarative contract likewise requires legal alternatives to be explored and these failure classes distinguished (`GrevirDeclarativeIntegrationDos.md` lines 82–98).

**Counterexample and impact.** A solver can pass the specified two-owner trap yet stop after a bounded search and report “no allocation” for three owners: A permits T0 or T1, B permits T0 or T2, and C permits only T2. If it first tries A→T0 and B→T2, it must revisit A to find A→T1, B→T0, C→T2. The plan’s stated exit tests do not require this feasible assignment or an exhaustion diagnostic if a limit prevents the search. A supported application could therefore be rejected as impossible while every stated phase gate passes.

**Required correction.** Make Phase 1 decide the completeness and exhaustion contract for the supported model, and make the Phase 2 gate test it with an independent small feasibility oracle, including a feasible multi-owner backtracking case and a separately identified search-limit outcome.

**Closure test.** Enumerate all assignments for a small synthetic inventory and compare them with the solver: every feasible instance produces a plan, every fully searched infeasible instance reports conflict, and a forced search limit reports exhaustion rather than impossibility.

## 2. Invariant analysis

The plan preserves the controlling distinction between common intent and resident-target constraints (plan lines 108–111; `GrevirTimerAllocationDesign.md` lines 105–169), and it scopes Pico to a design stress case without claiming an RP2040 backend (plan lines 51–68, 127–133). Its mock, AVR and classic ESP32 roles are consistent with the interrupt architecture’s named adapters and the target policies. The plan also preserves one validated binding plan as generator authority and rejects generated-source read-back (plan lines 186–200; `GrevirInterruptBindingArchitecture.md` lines 191–210, 248–253). Its startup phase can be read consistently with the interrupt architecture’s masked-source, terminal failed-start contract, provided the eventual independent-service policy respects that contract (`GrevirInterruptBindingArchitecture.md` lines 291–324).

The two findings concern places where that otherwise coherent direction lacks a single controlling answer or a sufficient gate. They do not challenge the deferred choice of configurator representation, first capability slice, RP2040 implementation, or automatic sharing.

## 3. Risks and next action

The Phase 1 design still needs to settle several intentionally open API and capability choices; their open status is not a defect in this draft. Revise the plan and its dependent sharing example for P2-1, and add the completeness/exhaustion decision and evidence gate for P2-2. Re-review those counterexamples at the resulting exact tuple.
