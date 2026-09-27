# Grevir event dispatch checkpoint — STATE-AXIS REVIEW

**Review object:** First `MainLoop`/`Elide` event dispatch slice, each specified commit versus its parent.  
**Baseline:** Exact tuple below, verified unchanged at review start and end.  
**Date:** 2026-09-28, Australia/Sydney.  
**Axis:** STATE — queue invariants, synchronization, startup, recovery and fail-closed behavior.  
**Verdict:** **NO-GO: one P2 correctness finding.** A bounded consumer-entry guard and regression test can close it.

| Repository | Reviewed commit |
| --- | --- |
| Workspace root | `f1acfc67d87efd632acc281e779c80404b580f09` |
| grevir-core | `95b9c438bef5f07946bb51a066a3f77c7388758b` |
| grevir-test-support | `c6a618d549ac61e6c9fc51f8e15e5a718914a07c` |
| grevir-peripherals | `4612cecdc99c22f326a0c1498846fea34c0c596c` |
| grevir-avr | `032f907fe430408c2d214431b68162cff1c85977` |

## 0 Evidence base

This was an independent, read-only source review. No files, Git state, builds or generated artifacts were changed. All five working trees were clean at both tuple checks. No other review reports were consulted.

Controlling material:

- `AGENTS.md` and `AGENTS_GWZ.md`.
- `dev-docs/GrevirEventActivationDesign.md` at the specified root commit.
- `dev-docs/GrevirEventContextsAndDispatchDesign.md`, retaining its queue semantics while treating its older handler syntax as superseded.
- `dev-docs/GrevirDeclarativeIntegrationDos.md` and `GrevirDeclarativeIntegrationDontDos.md`.
- `dev-docs/review-policies/CrossMcu.md`, plus `Avr.md` for the ATmega328P instantiation and `Esp32.md` for the unsupported deferred-backend boundary.

Inspection covered the new queue and route, interrupt dispatcher and startup changes, generated binding application identity, AVR and mock locks, start policies, examples, and the new queue test.

Target scope is the shared event contract, the deterministic host mock, and classic ATmega328P with its conventional 16-bit `int` ABI. ESP32 deferred dispatch remains excluded. Prior compiler/simavr evidence is acknowledged as reported in the controlling activation document; it was not rerun or independently reproduced here. Physical silicon validation remains held.

## 1 Findings

### STATE-1 — P2: Dispatch permits overlapping callback lifetimes within one supposedly serial context

**Location:** `grevir-core/src/grevir/event/queue.hpp:73–87`, particularly the unlocked call at line 85 and absence of an active-consumer check at dispatch entry.

**Scope and classification:** Shared contract correctness defect, affecting mock and AVR. Introduced by this checkpoint’s new `MainLoopQueue`.

**Violated invariant:** The context contract promises serial callback execution (`GrevirEventContextsAndDispatchDesign.md:188–190,234`). Releasing the queue lock before application code is necessary, but it does not by itself serialize callback lifetimes. No consumer-entry guard or documented non-reentrant `dispatch` precondition exists.

**Concrete reproduction, using one consumer thread and one catalogued event:**

1. Prepare a capacity-one queue and post event A.
2. Call `dispatch<App>(1)`.
3. A’s handler records entry, posts A again, then calls `dispatch<App>(1)` before recording exit. Restrict this nested action to its first invocation to keep the reproduction finite.
4. The outer dispatch cleared A’s pending mark at line 81. The repost therefore succeeds.
5. The nested dispatch acquires the now-free queue lock, removes A, and invokes the same handler while its previous invocation is still active.

The observed callback trace is `A-enter, A-enter, A-exit, A-exit`; maximum active callback depth is two. This requires neither multiple consumers on different threads nor an interrupt nesting policy. With A and B queued, the same construction lets B execute before A returns.

**Impact:** Application code relying on the promised serial context can be reentered while its state is temporarily inconsistent. A handler that repeatedly reposts and invokes a shared loop-pump helper can also recurse without a queue-capacity bound. The existing test verifies reentrant **posting**, but never attempts reentrant **dispatch**, so it does not expose the distinction.

**Correction direction:** Track active dispatch ownership per application queue. Acquire that ownership under `EventLock`, refuse or explicitly diagnose a nested/competing dispatch, and release ownership on every exit. Continue invoking callbacks outside the queue lock. Do not use the existing nonrecursive mock mutex across callbacks: that would deadlock legitimate callback posting.

If reentrant dispatch is intended instead, the serial execution contract must be explicitly revised and its application-state obligations documented; the current implementation cannot substantiate the existing promise.

**Regression/closure test:** On the mock, have A repost itself and attempt nested dispatch. Assert maximum callback depth remains one, the nested call does not consume a record, and the repost remains available to a subsequent outer dispatch. Repeat with A and B to verify ordering, and retain the current reentrant-post test. The guard’s AVR state must use the existing masked critical section.

**Policy basis:** Dos §1 requires independently defined execution meaning; Dos §12 requires explicit concurrency obligations and negative evidence. DontDos §8 prohibits treating the existing queue or target smoke tests as proof of this untested execution property.

## 2 Invariant analysis

### Queue arithmetic and pending state held

For the declared supported capacity interval `1..255`, the ring arithmetic is safe under ATmega328P’s 16-bit `int` model:

- `head_` is at most 254.
- On successful insertion, `count_` is at most 254 because the full test precedes insertion.
- `head_ + count_` and `head_ + i` remain below 509.
- `head_ + 1` is at most 255.
- Incrementing `count_` can reach 255 but cannot wrap because insertion stops at capacity.
- The clearing loop terminates at `i == count_`; a full capacity-255 queue does not require incrementing `i` beyond 255.

The pending invariant holds through the inspected transitions:

- Successful posting writes the record, sets its event’s pending mark, and increments count within one critical section.
- A repeated pending event coalesces without changing queue position.
- A full queue records overrun without setting the rejected event’s pending mark.
- Dequeue clears the pending mark while removing the record, allowing a firing during its callback to create a new record.
- `clear_overrun` does not remove records or pending marks.
- `prepare` and `stop` clear pending marks for every occupied record before resetting indices.

The existing capacity-two test exercises A/B ordering, C overflow and retry, sticky overrun clearing, and A reposting from its callback. This supports the basic state transitions but does not cover boundary capacities or the dispatch-entry finding.

### Record lifetime held

The record’s opaque argument points to `Pending<Spec, Event>::value`, which has static storage duration. The callback is a static function specialization. There is no borrowed stack object or owned payload whose lifetime can expire while queued.

Queue and pending storage are keyed by application specification, avoiding cross-application pending-bit sharing for the same event type.

### AVR publication and restored interrupt state held at source level

`grevir-avr/src/grevir/avr/event_lock.hpp:13–17` saves SREG, disables interrupts, and restores the saved SREG. Both assembler operations include a compiler memory clobber.

Consequently, queue reads and writes remain within the intended interrupt-masked region, including multi-byte callback and argument pointers. An ordinary ISR entered with interrupts disabled remains disabled when the lock is destroyed. A caller entered with interrupts enabled has that state restored. Nested lock scopes save and restore their respective prior states.

An interrupt between reading SREG and executing `cli` occurs before queue mutation; after its return, the interrupted producer enters its critical section. It does not create two simultaneous queue writers.

These are source-level synchronization conclusions, not measured latency or silicon timing claims.

### Mock queue producer synchronization held; controller concurrency is separate

The mock lock uses one static mutex, serializing queue mutation and publishing records between producers and the consumer. Callback code runs outside it, so ordinary callback posting does not deadlock.

The separate mock `InterruptController` has unsynchronized state. Concurrent calls to its hardware-simulation methods are not established as safe by the new queue mutex. That controller is inherited, so this review does not attribute its concurrency limitations to this checkpoint.

### Startup ordering and terminal failure held under the setup/loop lifecycle

`interrupt/start.hpp:27–41` masks owned sources, prepares the queue, configures and initializes modules, installs bindings, settles pending state, and only then enables sources. This places queue readiness before an enable operation that may immediately deliver a pending interrupt.

Failure at configuration, registration or pending settlement follows `start.hpp:47–55`: mask sources, stop and clear the queue, then clean up. A cleanup failure preserves the original failure separately.

The host and single-thread start policies cache the terminal result. Repeated successful startup therefore does not reset queued events; repeated failed startup does not reopen the stopped queue. The mock example checks successful replay, although it does not check replay with an event still queued.

These conclusions assume the intended setup-then-loop lifecycle. The queue does not establish callback quiescence against a concurrent `stop`, nor prevent dispatch during unfinished module setup. Concurrent lifecycle changes should not be advertised as supported without an explicit owner protocol and corresponding tests.

### Binding and unsupported-route boundaries held

The generated binding now carries the application specification, and the deferred dispatcher posts into that application’s queue. Handler kind, context and delivery are still compared with generated metadata.

A board without queue capacity and lock support fails the deferred dispatcher’s availability check. `Stream` reaches the queue’s explicit unsupported-route assertion. The existing ESP32 direct path does not acquire an implied deferred or cross-core synchronization guarantee.

## 3 Risks and next action

Close STATE-1 with a consumer-entry guard and the focused recursive-dispatch regression. The basic queue storage algorithm and AVR critical section do not need replacement to address it.

Additional evidence should exercise startup failures with pending queue records, repeated start with queued work, capacities one and 255, and multiple concurrent mock queue producers with one consumer. These are evidence gaps rather than separately proven defects.

The verdict applies only to this event checkpoint and this state axis. It does not certify earlier timer/board ownership architecture, deferred ESP32 execution, software-only event discovery, named contexts, `Stream`, deadline services, or physical hardware behavior.
