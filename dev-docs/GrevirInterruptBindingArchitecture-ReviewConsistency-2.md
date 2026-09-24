# Interrupt binding architecture — CONSISTENCY-AXIS REVIEW

**Review object:** `dev-docs/GrevirInterruptBindingArchitecture.md`, production design specification, Git blob `033a30d45c04e3697c5d5b3c22b8abd361008e6c`
**Baseline:** Repository HEAD `6e561be2921bcd9045759413b513dd38a739c76e`; original reviewed blob `670412dd24e0d35854509ab9883d83d371e1006e`. Other working-tree changes were out of scope. HEAD and the architecture file’s blob hash matched the specified tuple at the start and end.
**Date:** 2026-09-24
**Axis:** Internal consistency, controlling-contract alignment, prior-finding closure, and evidence-gate satisfiability. Independent, adversarial, read-only; this report does not rely on the current-round Safety review.

**Verdict: NO-GO** — one new P2 finding blocks. The five round-one findings are closed at the design-contract level. I would change this axis to GO if P2-1 is resolved without another blocking inconsistency.

---

## Prior-finding closure table

| ID | Disposition claimed | Original counterexample retraced on revised blob | Status |
| --- | --- | --- | --- |
| Consistency P2-1 | A current-attempt ready marker commits the JSON/header/source set; adapters reject older attempts. | After successful set A, generation of B invalidates B’s marker. Failure during `plan`, `emit`, or an individual output write leaves B unready. The adapter cannot select A as the current attempt. | Closed as a design contract; fault tests remain an implementation gate. |
| Consistency P2-2 | `emit` validates JSON internally; the build adapter owns current-input freshness. | Direct `emit` of old but valid JSON is now expressly permitted to establish only internal validity. The supported adapters run a current probe and check target, board and compiler identity; strict compilation compares the current binding plan. The former emitter-side freshness promise has been removed. | Closed as a design contract. |
| Safety P2-1 | Failed startup is terminal; all owned sources remain masked; acquired handles are released where possible or retained and reported. | If the second of two registrations fails, the first handle has an explicit cleanup/ownership rule. Concurrent and later callers do not repeat registration and receive the settled failure. | Closed as a design contract; mock and ESP32 checks remain. |
| Safety P2-2 | The ready marker is published only after all three artifacts are written. | Interruption after JSON, header, or source publication leaves the current attempt without a marker. The adapter rejects older attempts and does not compile or export a mixed set. | Closed as a design contract; interruption tests remain. |
| Safety P2-3 | Offered events declare a startup preserve/deliver or intentional-discard policy. | A pending event raised after module initialization has a declared outcome. Preserve/deliver requires a race-safe snapshot, dispatch and acknowledgement; an incapable backend cannot offer the event. | Closed as a design contract; mock and named-target checks remain. |

## Changed-range analysis

The revision narrows standalone `emit` freshness at lines 194–206, adds emission and ready-marker identity at lines 245–267, specifies failed-start and pending-event behavior at lines 291–318, and changes Arduino and CMake generation transactions at lines 377–458. Those changes address the remediation plan’s stated dispositions. The CMake description can use a stable output path private to each target/configuration while changing the attempt ID on regeneration; I did not treat “private output directory” as requiring a new CMake path on every run.

**NEW ARCHITECTURAL root cause:** The changed failed-start state-machine wording creates an inconsistent public `start()` result contract. It is reported below.

## 0. Evidence base

I read the complete revised architecture through `git show`, diffed it against the original reviewed blob, and read the remediation plan plus both round-one reports from their specified blobs. I checked `AGENTS.md`, `AGENTS_GWZ.md`, the CrossMcu, Avr and Esp32 policies, the prior self-review and investigation, the timer allocation design, and the current `AllocatedApplication` setup call. I used read-only `git`, `rg`, `cat` and `nl` commands. I made no edits and ran no builds or tests. This is a design verdict; mock execution, target compilation and silicon behavior are not claimed.

## 1. Findings

### [P2-1] Repeated `start()` has two incompatible result contracts

**Location:** Architecture lines 90–94 and 306–316. **Scope/classification:** Shared public runner API, affecting mock, AVR and ESP32; new architectural API/diagnosability defect introduced by the failed-start revision.

**Violated invariant:** The documented `start()` result must have one defined meaning for every call. Lines 90–93 require it to distinguish success from a repeated start. Lines 312–314 require concurrent callers to receive the same settled success or failure and a later `start()` to return “that result.” After a successful first call, the second call therefore must both report a distinct repeated-start result and return the first call’s success result. After a failed first call, reporting only “repeated start” would instead hide the terminal configuration or registration failure.

**Credible sequence and impact:** Call `start()` successfully, then call it again. An implementation following the lifecycle paragraph returns the original success; one following the entry-point paragraph returns repeated-start. Applications cannot reliably distinguish a duplicate invocation from initial success. With concurrent callers and a failed registration, an undifferentiated repeated-start code could also erase the failure reason that the terminal-state rule requires all callers to see.

**Required correction:** Define the result model explicitly. One workable contract is a settled setup outcome, retaining the original failure category, together with a per-call disposition such as first caller, waited caller or later caller. Alternatively, remove the promised repeated-start distinction and state that every caller receives the same settled outcome. Specify the behavior for both successful and failed starts.

**Closure test:** In the mock, exercise first success followed by another call, two concurrent calls during success, and second-registration failure followed by concurrent and later calls. Assert the exact returned values and retained failure reason for every caller; apply the same stated result semantics to the named ESP32 start-state check.

## 2. Invariant analysis

The revised generation contract now has one declared publication point. Its current-attempt marker, rejection of earlier attempts, and compiler checks give the original JSON/C++ mismatch sequences a defined failure path. Freshness is assigned to the adapters rather than claimed by a JSON-only emitter. These are specification checks, not evidence that CMake, Arduino CLI or object-section extraction implements them yet.

The pending-event and failed-registration counterexamples also have explicit outcomes. Source masking covers all owned sources on failure; unreleased handles remain recorded in a terminal state. An offered event must state whether startup pending status is delivered or deliberately discarded. The mock uses the same probe and allocator contract, while AVR vector behavior and ESP32 callback behavior remain separate target gates under the shared 1+N policy.

The public start-result invariant fails because the new replay rule and the earlier repeated-start category prescribe different observable results for the same second call.

## 3. Risks and next action

The preallocation handler-body cycle, section transport, Arduino compilation-database behavior, AVR acknowledgement rules and ESP32 registration behavior remain implementation gates. Silicon validation is deferred; none of these unperformed checks was treated as a finding.

Resolve P2-1’s result semantics in the architecture and its mock/ESP32 closure gates, then re-verdict this revised public contract.
