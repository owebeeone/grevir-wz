# Timer integration plan review — remediation record

Object reviewed: `GrevirTimerIntegrationRemediationPlan.md` at root commit
`2c7d5dd821308225a8bd16d05697b4aa161268d3`. The independent
[consistency](GrevirTimerIntegrationRemediationPlan-ReviewConsistency.md) and
[safety](GrevirTimerIntegrationRemediationPlan-ReviewSafety.md) reports both
returned NO-GO. This record merges their six blocking findings into one
documentation revision. It does not claim the timer API or implementation is
accepted.

| Finding | Disposition in this revision | Closure check for reviewer |
| --- | --- | --- |
| Consistency P2-1 — competing `SameTimer` contract | The plan explicitly supersedes cross-owner `SameTimer`; `GrevirTimerAllocationDesign.md` and `GrevirTimerApiExamples.md` now describe one provider module and dependent consumers. | Revisit section 3's two `MotorModule` instances: they need distinct timers as written; one-timer use requires a provider in the closure with two offers, and no second physical claims. |
| Consistency P2-2 — incomplete solver can masquerade as conflict | Phase 1 must settle completeness and separate search exhaustion; Phase 2 requires an independent small feasibility oracle and a backtracking three-owner case. | Enumerate a synthetic three-owner inventory, including feasible, fully infeasible and forced-search-limit cases, and check each required outcome in the plan. |
| Safety P2-1 — GPIO alias conflict deferred past selection | Canonical identity and board-alias normalization for every resource in the first supported slice is a Phase 2 prerequisite; Phase 4 completes the broader inventory. | Trace a PWM route and an explicit GPIO claim through different aliases of PB1 in both declaration orders; the Phase 2 gate must reject the conflict. |
| Safety P2-2 — multiple writers of a shared clock domain | Phase 1 identifies domain ownership; Phase 2 requires one selected writer with ordered lifecycle or rejects the candidate while unsupported. | Trace two timer owners demanding D=8 and a D=8/D=64 conflict with an alternative; no successful plan may permit two domain writers, including cleanup/update. |
| Safety P2-3 — different register programs can both be wrong | Phase 2 adds independent expected timing/capability and applied-effect checks at TOP boundaries, pin routing and interrupt-source compatibility. | A pair of wrong-but-different TOP programs or an unsupported event source must fail the gate; physical silicon behavior remains out of scope. |
| Safety P2-4 — nonresident SDK dependency can pass target examples | Phase 2 requires shared declarations to compile without the other SDK and inactive-section edits to preserve the active plan and diagnostics. | Compile the common/AVR/ESP32 declaration once per resident target without the nonresident SDK; inspect disabled branches and change only the inactive section. |

The revision changes only the plan and its two cited timer-design documents.
The stashed implementation remains outside the review object. Each reviewer
must recheck its own original counterexamples against one exact committed
revision before either axis can close its findings.
