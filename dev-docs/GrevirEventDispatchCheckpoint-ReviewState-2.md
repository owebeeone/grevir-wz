# Grevir event dispatch checkpoint — STATE-axis review, round 2

**Review object:** First `MainLoop`/`Elide` dispatch checkpoint and its remediation, compared with the original tuple in `dev-docs/GrevirEventDispatchCheckpoint-ReviewState.md`.  
**Axis:** Queue state, ISR/loop synchronization, callback serialization, startup and failure behavior.  
**Verdict:** **NO-GO — one P2 correctness finding.** The prior callback-reentrancy defect is closed, but an out-of-range board capacity can be narrowed into a valid plan capacity and, on the ATmega328P ABI, a one-slot runtime queue.

| Repository | Reviewed HEAD |
| --- | --- |
| Workspace root | `3ea2c2d31ab50b099247687274c0571e300352ca` |
| grevir-core | `0944c59b0c7a1ca53656f4a71fd62f1cce8969f5` |
| grevir-test-support | `d8fdd2e86c07ff62d08e282fc7fdb9990cdf9fec` |
| grevir-peripherals | `4612cecdc99c22f326a0c1498846fea34c0c596c` |
| grevir-avr | `5f4feffa15110aac9bf27f0bc17c2ce668b003d6` |

All five HEADs matched this tuple at review start and end. Each repository’s `git status --porcelain=v1 --untracked-files=all` was empty at both checks.

## Prior-finding closure

| Prior finding | State-axis assessment |
| --- | --- |
| STATE-1, overlapping callbacks | **Closed at source level.** `dispatch()` sets `dispatching_` under `EventLock` before releasing the lock for callbacks. Nested and competing calls return zero without dequeuing. The new mock test covers A reposting itself and attempting nested dispatch. |
| CODE-01, capacity absent from plan | Capacity and lock-policy identity now enter the probe record, canonical JSON and fingerprint, with strict generated assertions. The new two-event fixture checks these fields and stale-value diagnostics. **The range-validation gap in P2-1 remains.** |
| SURFACE-01, overflow inspection absent from walkthrough | The guide, Core API page and mock example now name `overrun()` and `clear_overrun()` and describe their effects. No separate state defect was found in these calls. |

## Changed-range analysis

The Core change adds dispatch ownership in `event/queue.hpp`, derives selected deferred-context facts in `interrupt/binding.hpp`, gates startup preparation and failure cleanup in `interrupt/start.hpp`, and carries capacity and policy through the probe, protocol and emitter. The mock and AVR lock changes add stable policy identities; they do not change lock mechanics. The root change adds a two-event mock fixture and documentation. Peripherals did not change from the original review tuple.

The new ownership guard addresses the original A/B counterexample. If A and B are queued in a capacity-two queue, dispatch dequeues A and clears A’s pending mark. A can repost itself into the freed slot, yielding queue order B, A. A’s nested `dispatch(1)` sees `dispatching_ == true` and consumes nothing. After A returns, the outer dispatcher may consume B if its budget remains; otherwise a later dispatch consumes B before the reposted A. Callback depth stays one.

## 0 Evidence base

This was read-only source inspection. I ran no compiler, host test, simulator or hardware validation and made no edits or Git mutations. Reported test behavior below describes inspected test code, not a test run. The controlling material was `AGENTS.md`, `AGENTS_GWZ.md`, `dev-docs/GrevirEventActivationDesign.md`, both declarative integration Dos/DontDos documents, the prior-round reports and remediation plan, and the CrossMcu, Avr and Esp32 policies. The older contexts design supplied the queue semantics; its handler syntax is superseded by the activation design. I did not inspect current-round peer reports.

Evidence commands and results:

- `git rev-parse HEAD` and `git -C <member> rev-parse HEAD` returned the five HEADs in the tuple table at both boundaries.
- `git status --porcelain=v1 --untracked-files=all` and the same command with `git -C` for each member returned empty output at both boundaries.
- `git diff --stat f1acfc67d87efd632acc281e779c80404b580f09 3ea2c2d31ab50b099247687274c0571e300352ca` showed the root documentation, fixture and plan changes. Equivalent member `git diff --stat` commands showed seven changed Core files, one mock-lock file, one AVR-lock file and no Peripherals change.
- `git -C grevir-core diff 95b9c438bef5f07946bb51a066a3f77c7388758b 0944c59b0c7a1ca53656f4a71fd62f1cce8969f5 -- src/grevir/event/queue.hpp src/grevir/interrupt/binding.hpp src/grevir/interrupt/start.hpp src/grevir/interrupt/probe_record.hpp tools/grevir_irqgen/emit.py tools/grevir_irqgen/protocol.py` exposed the remediation’s exact edits.
- `nl -ba` and `sed -n` inspections of those files, `grevir-core/tests/runtime/event_queue_test.cpp`, both `event_lock.hpp` files, the start policies and `scratch/interrupt-implementation-gates/check_roundtrip.py` supplied the line evidence below.
- `rg -uu -n` searches traced `DeferredContextPlan`, `event_queue_capacity`, queue operations and startup calls across the workspace.

The scope is shared C++23 behavior, the deterministic host mock and classic ATmega328P AVR. ESP32’s direct route remains in scope only as a boundary; ESP32 deferred delivery, `Stream`, software-only events, named contexts, deadlines and silicon validation are deferred. AVR claims use the policy’s conventional 16-bit `int` ABI. No instruction-count or silicon-timing claim is made.

## 1 Findings

### P2-1 — Capacity validation occurs after a narrowing conversion

**Location:** `grevir-core/src/grevir/interrupt/binding.hpp:170–185`, especially `static_cast<unsigned>(Board::event_queue_capacity)` at line 172; `grevir-core/src/grevir/event/queue.hpp:28–30`, where the board value is converted to `std::size_t` before its range assertion.

**Trigger:** On the supported ATmega328P ABI, declare `inline static constexpr auto event_queue_capacity = 65537UL;` on a board with a selected deferred event. `unsigned` is 16-bit on that ABI, so the plan’s conversion yields `1`. AVR `size_t` is also 16-bit through the target C header used by `grevir/base/compat/cstddef.hpp`; the runtime queue’s conversion likewise yields `1`. Both subsequent `1..255` checks accept the converted value. The probe records capacity one, and a second distinct event posted while the first remains queued gets `full`, despite the declared capacity being 65537.

**Scope, classification and provenance:** Correctness defect in the shared capacity contract, exposed by the ATmega328P instantiation. The queue’s premature conversion was present in the first checkpoint; the remediation introduced a second conversion in `DeferredContextPlan` and now serializes the false capacity as validated plan data. This is a **new plan-fidelity root cause layered on an inherited runtime range-check weakness**. It does not rely on applying AVR storage costs to the generic API.

**Violated invariant:** Active capacities must be validated in the declared value domain before conversion into backend storage or the canonical plan. `GrevirEventContextsAndDispatchDesign.md:186–187,342–354` requires a fixed, plan-checked capacity. Dos §§6–8 require modeled errors to fail before effects and lowering to remain faithful to the validated plan; DontDos §5 forbids applied behavior diverging from the selected plan.

**Impact:** An invalid board declaration can compile into a valid-looking one-slot deferred plan and queue on AVR. Overflow remains observable, so this does not prove memory corruption, but the plan falsely describes the author’s configuration and the capacity error is not diagnosed. On a host with wider `size_t`, the plan can still narrow a sufficiently large value to one before the runtime queue rejects it later; discovery itself is unsound.

**Correction:** Check that the original `Board::event_queue_capacity` is an integral, representable value in `1..255` before either cast. Derive one checked capacity for serialization and queue storage so the two cannot validate different narrowed values. Preserve zero for an unselected context without instantiating queue storage.

**Closure test:** During the target probe, require the dedicated capacity diagnostic for `0`, `256`, `65537UL` and a negative signed value when deferred delivery is selected. Verify `1`, `2` and `255` record their exact capacities and that a direct-only or zero-demand board may still expose inactive capacity zero. On the AVR-target compile, confirm `65537UL` fails before plan emission. No such test was run in this review.

## 2 Invariant analysis, including failed attacks

**Queue transitions and boundaries.** For a genuinely checked capacity in `1..255`, enqueue, dequeue, wraparound and `clear_records()` preserve the ring count and pending marks. On the ATmega328P 16-bit `int` model, the largest index sums are below 509. A successful post writes its record, marks the event pending and increments count within one lock. A duplicate pending event leaves its position unchanged. A full-queue rejection sets sticky `overrun_` without marking the rejected event pending. Dequeue clears its mark under the same lock before callback invocation, permitting a new firing to queue behind older records. `clear_overrun()` changes no records. `prepare()` clears occupied pending marks and the diagnostic; `stop()` clears occupied pending marks and makes later posts return `not_ready`. The inspected runtime test covers capacity two, A/B order, C overflow and retry, sticky diagnostic clearing, and A reposting. It does not establish boundary-capacity behavior by execution.

**Callback serialization.** The new entry guard is acquired under the queue lock and released on all ordinary exits after the callback loop. Callbacks execute with the queue lock released, so callback posting is possible. A nested or competing dispatch returns zero without stealing a record. For A and B, A’s repost remains after B; the prior STATE-1 attack no longer creates overlapping callback lifetimes. The inspected regression explicitly checks one callback depth and a zero nested-dispatch count. It uses A reposting itself; the B ordering conclusion is source reasoning rather than a newly run A/B nested test.

**ISR publication and lock scope.** AVR `EventLock` still saves SREG, executes `cli` and restores the saved register with compiler memory clobbers. The queue’s record and pending-mark mutations remain inside that masked scope. An interrupt between the SREG read and `cli` completes before the interrupted writer mutates the queue. The mock lock remains a single mutex; callbacks execute after its release. The separate mock `InterruptController` is not itself synchronized for concurrent calls, an inherited limitation rather than a change in this remediation.

**Startup, failure and selection.** `DeferredContextPlan::selected` scans demanded contexts. `start()` prepares only a selected queue after masking sources and before configuration, installation, pending settlement and enablement. Configuration, registration or pending-policy failure masks sources and stops that queue before board cleanup. The host start policy serializes concurrent start callers and caches the terminal result; the AVR policy is explicitly single-threaded. Replayed startup therefore does not reset queued work or reopen failure state. For direct-only or zero-demand plans, the selected flag is false, the canonical deferred context is capacity zero with empty policy, and startup neither instantiates nor prepares queue storage. These paths are coherent apart from P2-1’s unchecked original capacity value.

**Plan agreement and unsupported routes.** The probe serializes selected capacity and lock-policy identity into schema 3; the protocol requires a nonzero active context and zero inactive context; the fingerprint includes both fields. The generated strict unit compares them with the current `DeferredContextPlan`. The inspected round-trip script asks for a two-event capacity-two plan, checks changed fingerprints and dedicated stale capacity/policy diagnostics. These guards compare **converted** capacity values, which is why P2-1 survives. `Stream` reaches an explicit unsupported-route assertion when the deferred queue path is instantiated. ESP32 direct handling remains independent of this queue; absent ESP32 deferred lock/capacity support does not acquire an implied queue implementation.

**Lifecycle quiescence limit.** Public `stop()` and `prepare()` do not wait for an already-running callback. A concurrent stop can return while a callback is still active. The checkpoint’s supported lifecycle is setup then loop, with startup terminally cached; I found no changed code that newly advertises concurrent teardown or restart. This remains a contract/evidence boundary, not a separate defect finding.

## 3 Risks and next action

Close P2-1 by validating the board’s original capacity value before narrowing, then run focused probe and AVR negative cases alongside the existing queue and round-trip gates. The prior serial-callback counterexample is closed by the new guard. The inherited AVR board-owned timer setup remains an explicitly open architectural issue; this remediation does not worsen it or claim to repair it.
