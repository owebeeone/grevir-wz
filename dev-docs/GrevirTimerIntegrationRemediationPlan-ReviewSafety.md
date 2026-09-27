# GrevirTimerIntegrationRemediationPlan — SAFETY-AXIS REVIEW

**Review object:** `dev-docs/GrevirTimerIntegrationRemediationPlan.md` at `2c7d5dd821308225a8bd16d05697b4aa161268d3`; proposed work plan, draft-stage review  
**Baseline:** Root HEAD `2c7d5dd821308225a8bd16d05697b4aa161268d3`. The committed plan was read with `git show`; controlling documents were read with numbered-line inspection. HEAD matched and the working tree was clean at both the start and end of review.  
**Date:** 27 September 2026  
**Axis:** Safety: attack configurations and failure paths that the written gates could accept. Independent, adversarial, read-only; nothing here relies on the parallel review.

**Verdict: NO-GO** — four P2 findings block. I pre-commit to GO on a revision that resolves P2-1 through P2-4 as specified.

---

## 0. Evidence base

I read the plan at lines 1–212; `GrevirDeclarativeIntegrationDos.md` at lines 34–197; `GrevirDeclarativeIntegrationDontDos.md` at lines 28–129 and 146–164; `GrevirTimerAllocationDesign.md` at lines 73–169, 223–307 and 378–408; `GrevirTimerApiExamples.md` at lines 14–51 and 210–265; `GrevirInterruptBindingArchitecture.md` at lines 11–29, 99–163, 272–353 and 466–511; and the CrossMcu, Avr and Esp32 review policies. I also read `AGENTS.md`, `AGENTS_GWZ.md` and the review-loop process and prompt template. Inspection used `git show`, `nl -ba`, `rg`, `git rev-parse HEAD` and `git status --porcelain`; no files were changed and no builds were run.

These findings concern the plan’s acceptance gates. They do not assert defects in the stashed implementation, demand RP2040 implementation or silicon validation, or treat an open Phase 1 design choice as a defect.

## 1. Findings

### [P2-1] Phase 2 can accept a GPIO conflict that Phase 4 is assigned to discover

**Location and invariant.** The plan places canonical GPIO identities and alias normalization in Phase 4 (lines 172–184), after Phase 2 has implemented selection and passed its exit gate (lines 135–154). A selected timer output must conflict with an explicit claim on the same physical pin, regardless of the names used. The controlling allocation design requires ordinary claims to normalize to automatic binding identities and treats missing identity metadata as a model error (`GrevirTimerAllocationDesign.md`, lines 266–290); the declarative rules require the same alias check (`GrevirDeclarativeIntegrationDos.md`, lines 67–80).

**Counterexample and impact.** A board reserves its alias for ATmega328P PB1 while a PWM candidate routes Timer1 OC1A to PB1 using the current invented `pad_b1 = 101` identity. Phase 2 can select and program that PWM candidate, vary its selected settings successfully, and pass its stated zero/one/two-owner checks. Its conflict check can miss the board claim because Phase 4 has not unified those identities. The resulting plan falsely composes two uses of one pin.

**Required correction and closure test.** Make canonical identity and claim interoperability a prerequisite for Phase 2 selection for every resource in its first supported slice, or require Phase 2 to reject a candidate whose physical claim cannot yet be normalized. Keep broader identity migration in Phase 4 if desired. The Phase 2 gate must test a board alias and typed device pin for the same PB1, with PWM allocation and an explicit GPIO claim producing one conflict in either declaration order.

### [P2-2] Phase 2 does not require a single writer for a configurable domain shared by two timers

**Location and invariant.** Phase 1 asks who owns shared clock domains (plan lines 100–107), but Phase 2 directs setup through selected *timer owner* identities and its exit gate checks changed settings and owner counts without requiring a composed domain setup owner (lines 137–154). The controlling resource contract says agreeing uses of a shared divider must compose **one** domain setup owner; individual timer owners cannot independently reconfigure it (`GrevirTimerAllocationDesign.md`, lines 292–305). Selected hardware effects must belong to their owners (`GrevirDeclarativeIntegrationDos.md`, lines 164–178).

**Counterexample and impact.** Two modules select different physical timers that both need shared divider D=8. Strict whole-timer allocation succeeds. Each timer owner receives its plan-derived clock setup input and writes D during startup; a later update or cleanup from either owner can change D underneath the other. The Phase 2 gate can pass because both timers were selected, each applied its plan input, and no whole-timer collision occurred. Timer ownership alone has not secured ownership of D.

**Required correction and closure test.** Require Phase 2 either to create and dispatch exactly one selected setup owner for every configurable shared domain admitted by its supported slice, with ordered acquisition and cleanup, or to reject such candidates until that mechanism exists. Test two timers requiring D=8, a D=8/D=64 conflict with a legal backtracking alternative, and a recorded effect trace showing exactly one D writer through setup and cleanup.

### [P2-3] The Phase 2 gate tests whether settings change, not whether they implement the selected semantics

**Location and invariant.** The Phase 2 exit gate says changing selected mode, clock, period or source must change the applied program or fail (plan lines 150–154). That differential check cannot establish that either program implements its claimed timing, TOP, routing or interrupt behavior. The controlling rule explicitly warns that recognizing a configuration name and varying register settings is insufficient (`GrevirDeclarativeIntegrationDontDos.md`, lines 94–109). The allocation design separately requires correcting AVR TOP/frequency calculations before treating them as a capability oracle and checking expected mock register writes and duty updates (`GrevirTimerAllocationDesign.md`, lines 385–403).

**Counterexample and impact.** For the proposed Timer1 period/PWM slice, two selected periods produce two distinct register programs, but an off-by-one TOP calculation makes both realized periods wrong; an interrupt source may likewise be enabled under a mode that cannot provide the promised event. The stated Phase 2 gate passes while the selected plan and applied hardware behavior disagree.

**Required correction and closure test.** Add independent expectations for the first supported target configurations: calculate realized period and duty capability from authoritative clock and mode facts, and compare selected plan fields with expected register, TOP, interrupt-source and pin-routing effects. Include at least one negative incompatible mode/source case and boundary values where TOP arithmetic differs by one. This is modeled and mock evidence, not a claim of physical timing validation.

### [P2-4] The first implementation gate can miss a nonresident SDK dependency

**Location and invariant.** Phase 1 says nonresident declarations must remain inert without importing their SDK (plan lines 108–111), but the Phase 2 exit gate only says AVR, ESP32 and mock exercise the common contract for implemented capabilities (lines 150–154). The later validation paragraph says source checks of inactive branches should be *distinguished* (lines 202–210); it does not make them a Phase 2 exit condition. The accepted target-selection contract requires inactive sections to avoid instantiating drivers and to remain usable without their SDK (`GrevirTimerAllocationDesign.md`, lines 129–155), and its proposed acceptance evidence explicitly includes inactive-section invariance and absence of nonresident SDK dependencies (lines 393–396).

**Counterexample and impact.** A shared timer option declaration includes an ESP32 driver header or instantiates an ESP32 capability check while AVR is resident. Separate AVR and ESP32 target examples with both SDKs installed can pass Phase 2, while an AVR consumer without the ESP32 SDK cannot even parse the public declaration. A change confined to the inactive ESP32 section can also alter AVR diagnostics, violating target applicability.

**Required correction and closure test.** Make SDK-free nonresident parsing and lazy instantiation part of the Phase 2 exit gate. Compile a shared declaration with common, AVR and ESP32 sections for each resident target with the other target’s SDK unavailable; change only the inactive section and verify the active plan and diagnostics remain identical. Inspect disabled platform branches with a source check, as required by `GrevirDeclarativeIntegrationDontDos.md` lines 146–164 and `CrossMcu.md` lines 67–71.

## 2. Invariant analysis

The plan does protect several attacked paths in its text. It requires one physical timer owner per module and forbids silent co-location (lines 23–36 and 137–141). Its Phase 1 cases include ICR-as-TOP versus capture, a greedy allocation trap, runtime updates, and zero- and two-owner plans (lines 113–133). Phase 3 expressly requires `runLoop()` to enforce the chosen failure policy and forbids a dependent module from running after its provider fails (lines 156–170). Those are substantive constraints, and I found no separate startup finding that the plan’s wording supports.

The gap is when the plan permits Phase 2 implementation to pass before resource identity, shared-domain effects, independent applied-program checks, and inactive-target evidence are enforced. A later phase may repair some of these, but the earlier exit gate would still certify a selected configuration that can conflict with another claimant or differ from the behavior it promises.

## 3. Risks and next action

RP2040 remains a design stress target, and the first capability slice may be narrow. Physical timing, electrical behavior and silicon interrupt behavior remain unverified under the stated hold; none is required to close these findings.

Revise the Phase 2 prerequisites and exit gate to cover P2-1 through P2-4, then re-review the corrected committed plan against the original counterexamples.
