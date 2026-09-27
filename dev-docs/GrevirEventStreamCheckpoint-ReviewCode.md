# Grevir Event Stream checkpoint — Code review

**Tuple:** root `2c2ea1e1b3326caf41f38acdb8f79c02b21b4895`; grevir-core `6d09870d78b0628da4c6a0007c4a7d45e4ae5473`; grevir-test-support `d8fdd2e86c07ff62d08e282fc7fdb9990cdf9fec`; grevir-peripherals `4612cecdc99c22f326a0c1498846fea34c0c596c`; grevir-avr `5f4feffa15110aac9bf27f0bc17c2ce668b003d6`.

**Date:** 28 September 2026  
**Axis:** Code — architecture, interfaces, declarative activation, plan and generator fidelity, target call paths  
**Verdict:** **GO**, with one P3 coverage finding. No P0–P2 defect was established.

## 0. Evidence base

I reviewed root diff `3e09c15bdeb8397a55ff6f88b95ecc1184fa0a6a..HEAD`, Core diff `e37470b5e6ffa628e0788727c56704c394a46a07..HEAD`, and the controlling DRAFT `dev-docs/GrevirEventStreamCheckpoint.md`. I read the workspace instructions, both declarative integration rules, CrossMcu, Avr, and Esp32 policies, and the review-loop process. The review was **peer-blind and read-only**; I did not inspect peer reports, modify files, or run builds, tests, generators, or hardware commands. Findings below derive from source inspection. The claimed host, Pi, AVR, Windows, and ESP32 validation results were not reexecuted.

The exact five-repository tuple matched at both start and end. Each inspected worktree was clean at both checks.

## 1. Finding

### P3-1 — The new AVR Stream probe is outside the tracked validation gates

- **Root cause and location:** `scratch/interrupt-implementation-gates/avr_stream_queue_probe.cpp:1–20` adds a named AVR Stream instantiation, but no tracked gate references that file. A workspace-wide source search found `avr_stream_queue_probe.cpp` and `stream_queue_probe` only in that file. The checkpoint claims AVR GCC compiled it at `dev-docs/GrevirEventStreamCheckpoint.md:26`.
- **Trigger and impact:** A later change that breaks the AVR Stream queue instantiation can leave the existing AVR gate green because the new translation unit is never compiled by it. This is a bounded coverage and auditability gap, not evidence that the current AVR code fails.
- **Scope and classification:** AVR target instantiation; concrete validation coverage defect, **P3**.
- **Provenance:** Introduced with this checkpoint’s new standalone probe. It does not affect the existing Elide gate.
- **Correction:** Add the probe to the named AVR compiler gate with its required ATmega328P/C++23 flags, and record that gate’s command and result with the checkpoint evidence.
- **Closure test:** Intentionally make the probe ill-formed in a temporary validation checkout and confirm the named AVR gate fails; restore it and confirm the gate passes. The reviewer should then inspect the gate invocation and recorded output. Silicon and simavr Stream behavior remain outside this closure test.

## 2. Invariant analysis

The Stream route passed the source-level activation trace. `EventCatalog<Spec>` derives its finite event set from the module closure. `HandlerChoice<Event>` detects the visible `on_event<Event>()` specialization and classifies `MainLoop`/`Stream` as deferred. `DemandData` records handler kind, context, and delivery alongside the event key. The timer allocator consumes that same demand set when selecting a period-capable source; it does not use a parallel application binding list.

The probe record and existing schema carry `"stream"` delivery. Python plan validation accepts the `event/main_loop/stream` combination and requires one binding per demand. The emitter copies delivery into the generated demand and event binding, emits the selected source entry, and calls `dispatch_bound_interrupt<Event>()`. Strict compilation compares the live demand set with the recorded one and checks the binding’s handler, context, and delivery. The new mock round trip exercises a Stream-generated entry and rejects an Elide-route strict build against its Stream plan.

The generated dispatcher posts deferred events through `post_from_isr`. For Stream, `MainLoopQueue::post` skips the Elide pending test, appends a record with a null pending pointer, and returns `full` with sticky overrun once the board-selected capacity is reached. Dispatch removes one record under the board lock and invokes its handler once outside the lock. Null-pointer checks in dispatch and queue clearing preserve mixed Elide/Stream records. The index arithmetic remains within the selected 1–2048 capacity, including the AVR 16-bit `int` model. Existing Elide and direct branches retain their previous selection paths.

I found no new target-wide cost assumption, parallel hardware inventory, individual conditional attribute, or unbraced control-flow body in the changed C++ code. The ESP32 direct path remains separate; this review makes no claim about deferred ESP32 execution.

## 3. Risks and next action

File P3-1 as a nonblocking validation follow-up. The AVR compiler claim is limited to the reported named instantiation; the tracked gate currently cannot reproduce it. Source inspection supports the checkpoint’s mock and AVR Stream call path, but it does not establish simulated Stream timing or silicon behavior.
