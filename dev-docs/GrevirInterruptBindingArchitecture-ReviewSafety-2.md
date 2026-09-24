# Interrupt binding architecture — SAFETY-AXIS REVIEW, ROUND 2

**Review object:** `dev-docs/GrevirInterruptBindingArchitecture.md`, production design specification, Git blob `033a30d45c04e3697c5d5b3c22b8abd361008e6c`
**Baseline:** Repository HEAD `6e561be2921bcd9045759413b513dd38a739c76e`; original reviewed blob `670412dd24e0d35854509ab9883d83d371e1006e`. The exact blobs were read with `git show` and compared with `git diff`. Other working-tree changes were out of scope.
**Date:** 2026-09-24
**Axis:** Safety of degraded, interrupted and mixed-version paths. Independent, adversarial, read-only; the other current-round report did not inform this verdict.

**Verdict: GO** — the three prior safety findings close on the revised design. No new blocking finding was established.

---

## Prior-finding closure table

| ID | Disposition claimed | Verified on corrected design | Status |
| --- | --- | --- | --- |
| Safety P2-1 | Terminal failed-start state, handle cleanup and concurrent caller result | After a second registration fails, all owned sources remain masked; acquired handles are released in reverse order where possible. Failed releases remain recorded and reported. Concurrent and later callers receive the settled failure without another registration attempt (lines 306–318). | Closed |
| Safety P2-2 | One ready marker commits the generated output set | A generation attempt invalidates its marker before `plan` writes JSON. An interruption after JSON, header or source leaves no ready marker. Both build adapters require the marker for the current attempt and matching identity before compilation or export; neither falls back to older output (lines 252–267, 377–396, 427–458). | Closed |
| Safety P2-3 | Explicit startup pending-event policy | Every offered event declares preservation/delivery or intentional discard subject to a stated quiescent precondition. There is no unconditional clear. Preservation must survive an assertion during snapshot, dispatch and acknowledgement; a backend unable to guarantee its policy cannot offer the event (lines 290–304). | Closed |

The first-round consistency findings were also checked as context. The ready-marker boundary addresses Consistency P2-1. The design now assigns current-source freshness to the build adapter and strict compilation, and limits standalone `emit` to internal plan validation, addressing Consistency P2-2 (lines 188–205).

## Changed-range analysis

The revision changes the generator handoff, failed-start lifecycle, startup pending-event handling, and CMake/Arduino dependency descriptions. The marker is published only after all three output writes succeed; failed or interrupted generation has no ready set. The terminal failure rule closes the retry and leaked-handle ambiguity. The event policy closes the unspecified acknowledgement window.

**NEW ARCHITECTURAL root cause:** None established in the changed range. The specification makes guarantees that still require implementation and named-target tests; those are explicit gates, not evidence that the design permits a particular unsafe interleaving.

## 0. Evidence base

I read the complete revised blob and its diff from the original, the remediation-plan blob `fa601025de138de2c66e87d5cf3ec5fda50f07ff`, both first-round report blobs (`7ef5a78f272d8c5bab5e7132706e8e4ac55fb189` and `40ad2829bf8d3f63d8f9475f9a91b362038f7f78`), the earlier investigation and self-review, `AGENTS.md`, `AGENTS_GWZ.md`, and the CrossMcu, Avr and Esp32 review policies. I inspected relevant existing application code and documentation with `rg`. I made no edits and ran no builds or tests.

At both the start and end, `git rev-parse HEAD` returned `6e561be2921bcd9045759413b513dd38a739c76e`, and `git hash-object dev-docs/GrevirInterruptBindingArchitecture.md` returned `033a30d45c04e3697c5d5b3c22b8abd361008e6c`. The review tuple did not move.

## 1. Findings

No findings.

## 2. Invariant analysis

For the former publication failure, I traced a successful attempt A followed by attempt B failing during `plan`, after JSON, after the header, and after the source. Each B path invalidates the ready marker before modifying outputs, and the adapters refuse B without its current marker. The private attempt directory and single adapter owner rule exclude concurrent writers within the specified build path. Matching plan and emitter identities in the generated C++ add a strict compilation check (lines 245–267).

For the former registration failure, I traced two ESP32 source groups with the second registration failing while another task calls `start()`. The new terminal state prevents a second installation; all sources remain masked, acquired handles have specified cleanup ownership, and both callers receive the settled result (lines 306–318). This is a design obligation, not a claim that the current ESP32 adapter implements it.

For the former pending-event loss, I traced an assertion after module initialization and another during the snapshot/acknowledgement sequence. The revised contract forbids an unconditional clear and requires the selected backend to preserve/deliver without losing the later assertion, or to use deliberate discard only under a stated quiescent precondition. A backend that cannot meet the policy must withhold that event (lines 290–304).

The shared one-owner/source rules remain applicable to mock, AVR and ESP32. AVR-specific vector and acknowledgement behavior and classic ESP32 registration, affinity and callback behavior remain subject to their respective supplements. No target compilation or silicon behavior was inferred from this read-only design review.

## 3. Risks and next action

The object-section transport, ready-marker implementation, strict compile checks, fault injection, AVR vector behavior and ESP32 registration cleanup remain implementation gates stated in the specification. Silicon validation remains deferred.

The safety axis is ready to accept this revised design blob. Implement the mock pipeline and its failure-injection gates before claiming production behavior.
