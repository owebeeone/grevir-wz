# Interrupt binding architecture — CONSISTENCY-AXIS REVIEW, ROUND 3

**Review object:** `dev-docs/GrevirInterruptBindingArchitecture.md`, production design specification, Git blob `2dbe0d63fa8227cd34c17c69f37917bdb3d73a18`
**Baseline:** Repository HEAD `6e561be2921bcd9045759413b513dd38a739c76e`; prior design blob `033a30d45c04e3697c5d5b3c22b8abd361008e6c`. Other working-tree changes were out of scope. HEAD and the architecture file’s hash matched this tuple at the start and end.
**Date:** 2026-09-24
**Axis:** Internal consistency, controlling-contract alignment, prior-finding closure, and evidence-gate satisfiability. Independent, adversarial, read-only. This verdict does not rely on the current-round Safety review.

**Verdict: GO** — the round-two Consistency P2-1 finding is closed. No new finding or architectural root cause was established.

---

## Prior-finding closure table

| ID | Disposition claimed | Original counterexample retraced on revised blob | Status |
| --- | --- | --- | --- |
| Round-two Consistency P2-1 | Separate the settled setup outcome from the per-call disposition. | A first successful call returns `success` with `initiated`; a later call returns the same outcome with `replayed`. A caller arriving during setup receives the settled outcome with `waited`. After second-registration failure, each caller retains the originating failure and any cleanup failure, while its disposition describes its own call. Lines 90–97 and 309–322 agree. | Closed as a design contract; implementation tests remain a gate. |
| Round-one Consistency P2-1 and P2-2 | Use a current-attempt ready marker; assign current-source freshness to build adapters and strict compilation. | This revision does not alter those boundaries. A failed generation attempt remains unready, and standalone `emit` promises internal plan validity only. | Remain closed at the design-contract level. |
| Round-one Safety P2-1, P2-2 and P2-3 | Specify terminal failed-start ownership, atomic output-set readiness, and pending-event startup policy. | The changed result wording retains terminal failure, masking, handle ownership and cleanup reporting. The generation and pending-event clauses are unchanged. | Remain closed at the design-contract level. |

## Changed-range analysis

The diff changes only the public `StartResult` description at lines 90–97 and the matching lifecycle and mock-test wording at lines 314–322. Both locations define the same two-field result. The nearby mock-backend reference to “repeated-start behavior” at line 332 describes the replay case and does not prescribe a conflicting outcome.

**NEW ARCHITECTURAL root cause:** None established.

## 0. Evidence base

I read the revised design blob with `git show`, compared it with the prior blob using `git diff`, and retraced the round-two Consistency P2-1 counterexample against the second remediation plan and the prior Consistency and Safety reports. I checked `AGENTS.md`, `AGENTS_GWZ.md`, the CrossMcu, Avr and Esp32 policies, the earlier architecture review and investigation, and relevant current application setup declarations. The review used read-only inspection commands; I made no edits and ran no builds or tests.

At both checks, `git rev-parse HEAD` returned `6e561be2921bcd9045759413b513dd38a739c76e` and `git hash-object dev-docs/GrevirInterruptBindingArchitecture.md` returned `2dbe0d63fa8227cd34c17c69f37917bdb3d73a18`.

## 1. Findings

No findings.

## 2. Invariant analysis

The public result has one meaning across the specified call sequences. Its outcome reports the settled setup result; its independent disposition reports whether this call initiated setup, waited during it, or replayed the result afterward. A replay cannot erase success, the originating registration failure, or a cleanup failure. Concurrent callers do not initiate another registration.

The mock test obligation now asserts both fields for first success and replay, concurrent success, and second-registration failure with concurrent and later calls. This is consistent with the shared runner contract and the ESP32 task-safe state-machine requirement. It is a specification obligation, not evidence that the current code implements it.

The revision leaves the previously reviewed generation, ownership and pending-event contracts intact. AVR vector behavior and classic ESP32 callback behavior remain target-specific implementation gates under the shared 1+N policy. Silicon validation remains held.

## 3. Risks and next action

The `StartResult` semantics still require implementation and the stated mock and named ESP32 checks. The object-section transport, build adapters, AVR entry mapping and hardware behavior remain separate implementation gates.

This axis accepts the pinned design blob.
