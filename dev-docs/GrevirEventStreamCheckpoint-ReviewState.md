# Grevir event Stream checkpoint — State-axis review

**Date:** 2026-09-28  
**Axis:** FIFO state, Stream/Elide interaction, ISR/loop locking, overflow, lifecycle, and callback reentry  
**Verdict:** **GO on the State axis.** No open P0–P2 defect was established. One P3 documentation finding remains.

| Repository | Reviewed HEAD |
| --- | --- |
| Workspace root | `2c2ea1e1b3326caf41f38acdb8f79c02b21b4895` |
| grevir-core | `6d09870d78b0628da4c6a0007c4a7d45e4ae5473` |
| grevir-test-support | `d8fdd2e86c07ff62d08e282fc7fdb9990cdf9fec` |
| grevir-peripherals | `4612cecdc99c22f326a0c1498846fea34c0c596c` |
| grevir-avr | `5f4feffa15110aac9bf27f0bc17c2ce668b003d6` |

All five HEADs matched the requested tuple at the start and end of review. Each working tree was clean at both checks.

## 0 Evidence base

This was an independent, peer-blind, read-only source review. I did not read the current Code or Surface reviewers’ reports, modify files, or run builds, tests, simulators, or hardware. Test descriptions below refer to inspected test code, not observed execution.

I inspected the root diff `3e09c15bdeb8397a55ff6f88b95ecc1184fa0a6a..HEAD`, Core diff `e37470b5e6ffa628e0788727c56704c394a46a07..HEAD`, and controlling [draft](dev-docs/GrevirEventStreamCheckpoint.md). I followed `AGENTS.md`, `AGENTS_GWZ.md`, the review-loop skill, both declarative integration Dos/DontDos documents, and the CrossMcu policy. The AVR supplement governs the ATmega328P instantiation and its 16-bit `int`; the ESP32 supplement was consulted for the direct-route boundary. ESP32 deferred execution, software-only events, named contexts, deadlines, silicon behavior, and inherited board-owned timer setup were excluded.

Primary state evidence was `grevir-core/src/grevir/event/queue.hpp:17–140`, `queue_capacity.hpp:8–26`, the AVR and mock `event_lock.hpp` implementations, `interrupt/start.hpp:26–56`, `interrupt/handler.hpp:90–117`, and `grevir-core/tests/runtime/event_queue_test.cpp:75–152`.

## 1 Finding

### P3-1 — The public guide applies Elide’s repeat rule to Stream

**Root cause and location:** The new Stream section in `docs/guides/interrupts.md:58–79` was inserted before an existing general queue paragraph. Line 83 still says, without an Elide qualifier, that another firing while an event is queued “coalesces.”

**Trigger and consequence:** An author selects `RouteFor<Event>::Delivery = Stream`, then reads the immediately following queue paragraph. That paragraph describes the opposite of Stream’s actual repeat behavior and of lines 74–77. The contradiction can mislead capacity planning or debugging, although the explicit Stream paragraph states the correct rule. This is a bounded documentation defect, not an observed runtime defect.

**Scope, provenance, and severity:** Public API documentation; introduced as a contradiction by this checkpoint’s Stream documentation. **P3**, because the correct rule is also stated nearby and the implementation follows it.

**Correction:** Qualify the later repeat sentence as applying to `Elide`, or split the shared queue paragraph into delivery-specific sentences.

**Closure test:** Read the guide from the Stream example through the queue paragraph and confirm every repeat and full-queue statement has an unambiguous delivery mode. A focused documentation assertion or review should check that Stream never claims `coalesced`.

## 2 Invariant analysis and failed attacks

**FIFO and mixed delivery.** `post()` holds `Board::EventLock` across readiness, Elide’s pending check, capacity check, record write, pending update, and count increment. Accepted Stream firings each append one record with a null pending argument. Elide appends one marked record, and a repeat while marked returns `coalesced` without moving it. `dispatch()` copies the head record and clears a non-null pending mark under the same lock before advancing the head; callbacks run after unlocking. Thus, for queued `Elide A, Stream S, Stream S`, dequeue order remains `A, S, S`. Reposting A from A’s callback appends it behind the remaining S records. The inspected capacity-two test checks a mixed A/S case, while this longer sequence is a source-level deduction.

**Full queue and recovery.** A full Stream post changes no record, index, count, or pending mark; it sets `overrun_` and returns `full`. Each such call represents one rejected firing. The one-bit diagnostic records that *at least one* firing was dropped; it is not a drop counter. After one dequeue, a later Stream post can occupy the freed slot, and the older record stays ahead of it. Overrun remains set until `clear_overrun()` or `prepare()`. A full Elide post with no pending mark likewise leaves that mark clear, permitting a later retry. An Elide repeat already pending returns `coalesced` before the full check, consistently preserving its existing record.

**ISR/loop publication and callback reentry.** `post_from_isr()` calls the same locked publication path as loop `post()`. AVR `EventLock` saves SREG, disables interrupts, and restores the prior state with compiler memory clobbers; the mock lock uses one mutex. Record publication and dequeue are therefore serialized on the named backends. `dispatching_` is acquired under that lock before callbacks and cleared under it afterward. Nested or competing dispatch returns zero, while callback posting is possible outside the lock. The inspected test exercises nested dispatch and reposting; it does not prove target interrupt timing.

**Stop and reset.** `stop()` makes later posts return `not_ready` and clears pending marks belonging to occupied Elide records; Stream records need no mark cleared. `prepare()` clears occupied marks, empties the ring, clears overrun, and restores readiness. Neither operation waits for an already-running callback. Concurrent teardown or restart during an active callback is not promised by the current setup-then-loop lifecycle, and this behavior predates Stream; I do not classify it as a checkpoint defect.

**Boundary capacities and AVR arithmetic.** `QueueCapacity` validates the original board value in `1..2048` before conversion. At capacities 1–255, the selected index is 8-bit; at 256–2048 it is 16-bit. On the specified AVR ABI, `head_ + count_` is at most 4095 at capacity 2048, within 16-bit unsigned arithmetic, and at most 509 for an 8-bit index, within 16-bit signed `int`. `clear_records()` terminates at a full count, including capacities one and 2048. This is source reasoning; the new native queue test uses capacity two, and the AVR Stream probe is a compile instantiation rather than a runtime boundary test.

## 3 Risks and next action

Correct P3-1 before treating the guide as a settled description of both delivery modes. The lane owner can merge this GO verdict with the independent axes and use the declared test and target-gate evidence for execution claims. This review establishes source-level state behavior only; it does not claim that inspected tests ran or that Stream timing was simulated or measured on silicon.
