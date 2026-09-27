# Deferred event dispatch follow-up — State-axis review

**Date:** 2026-09-28  
**Axis:** Queue state, ISR/main-loop synchronization, callback serialization, startup and failure behavior  
**Verdict:** **GO on the State axis.** No open P0–P2 State defect was established by source inspection. The original State P2-1 capacity-narrowing counterexample is closed.

| Repository | Reviewed HEAD |
| --- | --- |
| Workspace root | `2f25854834f83a2f9fbb0372a352d5bb5f1a1be0` |
| grevir-core | `e37470b5e6ffa628e0788727c56704c394a46a07` |
| grevir-test-support | `d8fdd2e86c07ff62d08e282fc7fdb9990cdf9fec` |
| grevir-peripherals | `4612cecdc99c22f326a0c1498846fea34c0c596c` |
| grevir-avr | `5f4feffa15110aac9bf27f0bc17c2ce668b003d6` |

All five HEADs matched this tuple at the beginning and end of review. `git status --porcelain=v1 --untracked-files=all` was empty in each repository at both checks.

## 0 Evidence base

This was an independent, peer-blind, read-only review. I did not inspect the current Code or Surface reviewers’ reports, modify files, or run builds, tests, simulators, or hardware. Evidence is limited to `git show`/`git diff`, `rg`, `sed`, and numbered source inspection. Existing validation claims in the controlling draft were not reproduced.

The object was the root diff `3ea2c2d31ab50b099247687274c0571e300352ca..HEAD`, the Core diff `0944c59b0c7a1ca53656f4a71fd62f1cce8969f5..HEAD`, and the controlling draft `dev-docs/GrevirEventDispatchFollowupRemediation.md`. I followed `AGENTS.md`, `AGENTS_GWZ.md`, the review-loop skill, both declarative integration Dos/DontDos documents, and the CrossMcu policy. The AVR supplement applies to the ATmega328P instantiation and its 16-bit `int`; the ESP32 supplement applies only to the direct-route boundary here. Inherited board-owned timer setup, `Stream`, other contexts, silicon, and unrelated packages were excluded.

## 2 Invariant analysis

**Original State P2-1 — closed.** `grevir-core/src/grevir/event/queue_capacity.hpp:13–26` checks the board’s original integral value against `1..2048` before conversion to `size_t`. On the AVR ABI, `65537UL` remains a 32-bit unsigned-long operand during comparison and fails the upper bound; negative signed values, zero, and 2049 also fail. `SelectedQueueCapacity` converts only the checked value. `DeferredContextPlan` uses that selected result, while `MainLoopQueue` uses `QueueCapacity<Board>::value`, so plan and storage derive from the same validated declaration. The previous route by which `65537UL` became a valid-looking one-slot plan and queue is gone.

**Capacity and ring arithmetic.** Values 1, 2, and 255 select an 8-bit unsigned index; 256 through 2048 select a 16-bit unsigned index via `TypeForMaxValue`. At capacity 2048, the largest `head_ + count_` is 4095, within the AVR 16-bit unsigned range; at capacity 255 it is 509, within AVR signed `int`. Count can represent the full selected capacity, and `records_[capacity]` allocates the selected record count. Enqueue, dequeue, wrap, and `clear_records()` do not require an extra sentinel slot. This is a source-level arithmetic proof, not a claim that every selected count fits a particular board’s RAM.

**Publication, elision, and overrun.** In `event/queue.hpp:52–72`, the readiness check, pending check, full check, record write, pending mark, and count increment occur under `Board::EventLock`. A full rejection sets sticky overrun without marking the rejected event pending. Dequeue clears the mark under the same lock before invoking the callback; a firing during that callback can append a new record behind older work. `clear_overrun()` changes only the diagnostic. The implementation retains multi-producer locking and these semantics rather than substituting Base `CircularBuffer`, whose one-writer and post-overrun behavior differs.

**Callback serialization.** `dispatch()` sets `dispatching_` under the lock, then releases the lock before callbacks. Nested or competing dispatch returns zero, and ordinary completion clears the guard under the lock. A callback can post while running, but another dispatcher cannot overlap its invocation or take the reposted record. The earlier overlapping-callback attack remains closed.

**Startup and failure.** `Application::start()` masks owned sources, prepares a selected deferred queue before configuration and source enablement, and stops it after configuration, registration, or pending-policy failure. The host start policy serializes callers and replays the terminal result; the AVR policy explicitly assumes nonconcurrent setup. Direct-only and zero-demand plans select capacity zero and empty policy without instantiating or preparing queue storage, even when a board declares inactive capacity zero.

**Plan agreement.** Schema 4 writes capacity as a 16-bit field; the Python protocol reads the same width and accepts `1..2048` only for active deferred demand. The generated strict unit checks the live demand set and compares selected capacity and lock-policy identity against emitted values. A changed active capacity therefore fails strict agreement; a newly invalid capacity fails during C++ validation. Inspection of the added round-trip probes shows cases for 0, negative, 2049, 65537UL, 1, 255, 256, and 2048, plus an AVR negative probe. Their execution is outside this review’s evidence.

## 3 Risks and next action

`stop()` and `prepare()` do not wait for an already-running callback. Concurrent teardown or reinitialization is not promised by the current setup-then-loop lifecycle, so this is a contract boundary rather than a finding on this object. Source inspection also cannot establish target memory fit, compiled diagnostics, or physical interrupt timing.

The State axis can proceed. Run the planned probe, strict-compilation, queue, AVR, and target gates before the lane owner merges this verdict with the independent axes.
