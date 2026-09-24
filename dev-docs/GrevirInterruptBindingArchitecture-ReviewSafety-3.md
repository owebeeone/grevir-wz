# Interrupt binding architecture — SAFETY-AXIS REVIEW, ROUND 3

**Review object:** `dev-docs/GrevirInterruptBindingArchitecture.md`, production design specification at Git blob `2dbe0d63fa8227cd34c17c69f37917bdb3d73a18`
**Baseline:** Repository HEAD `6e561be2921bcd9045759413b513dd38a739c76e`; prior design blob `033a30d45c04e3697c5d5b3c22b8abd361008e6c`. The design and prior reports were read with `git show`; the design revisions were compared with `git diff`. Other working-tree changes were out of scope.
**Date:** 2026-09-24
**Axis:** Safety of degraded, interrupted and mixed-version paths. Independent, adversarial, read-only. The parallel current-round report did not inform this verdict.

**Verdict: GO** — the revised public result preserves the settled failure for concurrent and later callers. The prior safety counterexamples remain closed. No new blocking finding was established.

---

## Prior-finding closure table

| ID | Disposition claimed | Original counterexample retraced on corrected design | Status |
| --- | --- | --- | --- |
| Consistency round 2 P2-1 | `StartResult` separates the settled setup outcome from the per-call disposition. | After first-call success, a later call returns success with `replayed`. A caller arriving during setup waits and returns the same settled outcome with `waited`. After registration failure, all callers retain the originating failure and any cleanup failure; no call substitutes a repeated-start error (lines 90–97, 309–322). | Closed |
| Safety P2-1 | Failed startup is terminal, with all owned sources masked and acquired handles released where possible or retained and reported. | If the second registration fails, cleanup ownership remains defined. A concurrent caller waits for the settled failure; a later caller replays it without another registration attempt (lines 309–322). | Closed |
| Safety P2-2 | One current-attempt ready marker commits the generated output set. | A failed or interrupted generation invalidates the marker before modifying outputs. The adapters require the current marker and matching identity, so an older JSON/header/source set cannot serve as fallback (lines 255–270, 383–402, 433–464). | Closed |
| Safety P2-3 | Each offered event declares a startup pending-event policy. | An event asserted between module initialization and source enable must be preserved and delivered through a race-safe acknowledgement sequence, or intentionally discarded under a stated quiescent precondition. An incapable backend cannot offer the event (lines 293–307). | Closed |

## Changed-range analysis

The diff from `033a30d4` to `2dbe0d63` changes the entry-point result description at lines 90–97 and the failed-start lifecycle and mock assertions at lines 309–324. It assigns `initiated`, `waited`, or `replayed` per call while keeping the setup outcome common to every caller. The ready-marker, source-ownership and pending-event rules did not change.

**NEW ARCHITECTURAL root cause:** None established. In particular, the per-call disposition does not erase a terminal error or authorize another registration attempt.

## 0. Evidence base

I read the complete controlling design blob, its exact diff from the prior blob, the second remediation plan (`f164651df6bd8f1f577245c268796760557b5dde`), and the round-two Consistency and Safety reports (`968b31ef4124e8961902f30ab39f4140cda3e092` and `4ec0dff43d40b0d9e188a93568540f4555090aa5`). I also read `AGENTS.md`, `AGENTS_GWZ.md`, the CrossMcu, Avr and Esp32 review policies, the first safety report and remediation plan, the earlier design investigation and self-review, and the current application setup code. I made no edits and ran no builds or tests.

At both the start and end, `git rev-parse HEAD` returned `6e561be2921bcd9045759413b513dd38a739c76e`, and `git hash-object dev-docs/GrevirInterruptBindingArchitecture.md` returned `2dbe0d63fa8227cd34c17c69f37917bdb3d73a18`. The review tuple did not move.

## 1. Findings

No findings.

## 2. Invariant analysis

I traced a successful initiating call followed by a concurrent caller and a later caller. Each receives the same success outcome; the disposition records whether it initiated, waited or replayed. I then traced a second-source registration failure with a caller waiting during setup and another calling after failure. All owned sources remain masked, cleanup retains or releases acquired handles under the stated rule, and every caller receives the originating failure plus any cleanup failure. The design permits no retry in that firmware session.

I retraced interruption after each generated-output write with an older valid output set present. The current attempt lacks a ready marker until all three outputs succeed, and the adapters reject an earlier marker. I also retraced an event assertion before source enable and another during snapshot and acknowledgement. The declared policy must cover both without an unconditional clear.

The shared contract applies to the host mock, ATmega328P AVR and classic ESP32 Arduino target. AVR vector and acknowledgement behavior and ESP32 registration, callback and task behavior remain separate named-target implementation checks. This review establishes a design verdict, not target execution or silicon behavior.

## 3. Risks and next action

The start-state synchronization, cleanup fault injection, ready-marker transaction, pending-event policies and target adapters remain implementation gates. Silicon validation remains held.

The safety axis is ready to accept this design blob. Implement the mock pipeline and its stated failure tests, then perform the named AVR and ESP32 checks before claiming production behavior.
