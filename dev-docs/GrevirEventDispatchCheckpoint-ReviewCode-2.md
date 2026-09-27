# Grevir event dispatch checkpoint — CODE-axis review, round 2

**Reviewed tuple:** root `3ea2c2d31ab50b099247687274c0571e300352ca`; Core `0944c59b0c7a1ca53656f4a71fd62f1cce8969f5`; Test Support `d8fdd2e86c07ff62d08e282fc7fdb9990cdf9fec`; Peripherals `4612cecdc99c22f326a0c1498846fea34c0c596c`; AVR `5f4feffa15110aac9bf27f0bc17c2ce668b003d6`.

**Verdict: NO-GO on the CODE axis.** The prior queue-capacity finding is closed, but one P2 strict-build defect remains in the checkpoint’s hardware-event activation path. This is a **distinct, newly uncovered architectural root cause**, rather than a recurrence of CODE-01’s omitted capacity field. It constitutes a third root cause for the review-loop cap if the other two axes have already identified two. The defect originated in the first checkpoint; the remediation neither introduced nor closed it.

| Prior finding | Closure assessment |
| --- | --- |
| CODE-01, P2: queue capacity absent from canonical plan | **Closed by source inspection.** `DeferredContextPlan` derives selection from demand, records active capacity and lock-policy identity, and rejects an unavailable or out-of-range active context. Probe schema 3 serializes both values; Python validates and fingerprints them; generated strict C++ compares them. The two-event scratch case checks capacity two, changed-capacity fingerprint, and dedicated stale-capacity/policy diagnostics. |
| STATE-1, P2: nested dispatch | **Implementation observed, outside this axis’s independent verdict.** The queue now guards dispatcher entry under `EventLock` and releases the lock for callbacks; a focused runtime case was added. |
| SURFACE-01, P3: overflow guidance | **Documentation edits observed, outside this axis’s independent verdict.** The public example, guide, and Core API reference now show inspection and clearing. |

## Changed-range analysis

Core changes cover `DeferredContextPlan`, probe encoding, Python validation and emission, demand-selected startup, and a dispatcher guard. Test Support and AVR add stable lock-policy identities. The root adds the two-event mock round trip and documentation. Peripherals is unchanged from the first checkpoint. The new context fields are derived from C++ demand and board policy; the generator lowers them without choosing capacity or lock implementation. Direct-only and zero-demand generated plans use `{capacity: 0, policy: ""}`. The AVR board-owned Timer1 setup remains inherited and was not worsened by these changes.

## 0 Evidence base

This was a read-only source review. I ran no builds, tests, hardware operations, or mutations. The exact tuple and empty `git status --short` were checked at both start and end. Representative inspection commands and their results:

| Command | Result relevant to this review |
| --- | --- |
| `git rev-parse HEAD; git status --short` and the same commands with `git -C` for each four member repositories | All five hashes matched the tuple above; every short status was empty at start and end. |
| `git -C grevir-core diff --name-only 95b9c43..HEAD` | Seven changed Core files: queue, binding, probe record, startup, runtime test, Python emitter, and protocol. |
| `git diff --name-only f1acfc6..HEAD` | Root changes include design/remediation documents, overflow guidance, and the two-event scratch round trip. |
| `nl -ba grevir-core/src/grevir/interrupt/demand.hpp \| sed -n '1,90p'` | Catalog-wide `DemandData` and its `HandlerChoice` enumeration exist only under `GREVIR_IRQ_PROBE`. |
| `nl -ba grevir-core/tools/grevir_irqgen/emit.py \| sed -n '58,170p'` | Strict mode emits a fixed `DemandSet` from the probed plan and dispatches only generated bindings. |
| `rg -n -uu 'HandlerChoice<\|DemandSet<\|STALE' grevir-core/src/grevir/interrupt grevir-core/tools/grevir_irqgen scratch/interrupt-implementation-gates` | No strict-mode catalog-wide live-demand comparison was found. Existing handler/route checks are reached through generated bound dispatchers. |
| `nl -ba scratch/interrupt-implementation-gates/mock_app_base.hpp \| sed -n '1,110p'` and `nl -ba scratch/interrupt-implementation-gates/mock_zero_main.cpp \| sed -n '1,30p'` | The existing zero-demand mock fixture has a catalogued event, capacity zero, and a successful no-binding startup path. |
| `nl -ba grevir-core/src/grevir/interrupt/binding.hpp \| sed -n '138,190p'` | Both `BindingPlan` and `DeferredContextPlan` consume `DemandSet<Spec>::value`; in strict mode that is the generated specialization. |

The controlling design was `dev-docs/GrevirEventActivationDesign.md` at the pinned root. I also read `AGENTS.md`, `AGENTS_GWZ.md`, the CrossMcu, AVR, and ESP32 policies, both declarative integration rule documents, the first CODE report, and the merged remediation plan. No current-round peer report was consulted. Source inspection supports the findings below; it does not establish new host, AVR compiler, simavr, or silicon test results.

## 1 Findings

### P2-1 — A newly added strict-build handler can remain absent from the generated plan

**Location:** `grevir-core/src/grevir/interrupt/demand.hpp:6,62–69`; `grevir-core/tools/grevir_irqgen/emit.py:62–76,86–100,140–152`; `grevir-core/src/grevir/interrupt/handler.hpp:79–88`.

**Trigger:** Start with the existing zero-demand mock shape: `mock_app_base.hpp` catalogs `PeriodElapsed`, and `mock_zero_record.cpp` probes it without an `on_event` specialization. In the same application header, make an `on_event<PeriodElapsed>()` specialization visible only to the strict compilation, for example under `#if !defined(GREVIR_IRQ_PROBE)`. Keep the file and board identity unchanged between phases. Run the normal probe, plan, emitter, and strict build.

**Scope, classification, and provenance:** Shared C++23 activation/strict-build contract; concrete correctness and diagnosability defect for the mock and ATmega328P hardware-event paths. It originated in the first checkpoint’s generated-demand design and is newly uncovered in this review. No AVR cost inference is involved. It is **within this checkpoint’s scope**: the controlling design requires a catalogued handler specialization to create demand, allocation, an entry, and strict agreement. It is a distinct architectural root cause from CODE-01: strict mode substitutes a generated `DemandSet` for live discovery, so it cannot notice newly present demand.

**Violated invariant:** One authoritative plan must reflect current application intent, and lowering must agree with the final firmware. See Dos §§2, 6, and 8 and DontDos §5. The activation design’s required causal chain explicitly begins with the handler specialization.

**Evidence and impact:** In probe mode, `DemandData` enumerates `HandlerChoice` for every catalog event. In strict mode, that implementation is absent; `emit.py` writes a fixed `DemandSet` containing only previously probed events. Generated `dispatch_bound_interrupt<Event>()` calls instantiate `HandlerChoice` only for emitted bindings. With zero emitted bindings, they instantiate it for none. `BindingPlan` still validates the generated zero-demand set, and `DeferredContextPlan` still selects no queue despite the newly visible default `MainLoop`/`Elide` handler. The strict build can therefore accept an application whose newly declared hardware-event handler has no physical entry and will never run. A freshness marker cannot detect this unchanged-header, different-definition trigger.

**Correction:** Compute the live catalog-wide handler demand in strict compilation independently of the generated binding gate, and compare its complete event keys, handler kinds, contexts, and deliveries with the generated demand before accepting the final unit. Keep the generated representation as the probed plan rather than as the only strict-mode view of intent.

**Closure test:** Probe the zero-demand mock fixture, then compile its generated strict unit with a strict-only `on_event<PeriodElapsed>` specialization in the same header. Require a dedicated stale-demand failure. Repeat with one previously bound event plus a second newly added catalogued handler, including an AVR-compatible fixture. Confirm the ordinary unchanged builds remain valid.

## 2 Invariant analysis and failed attacks

The CODE-01 counterexample is addressed at its original seam. `DeferredContextPlan` selects `main_loop` from demand, serializes capacity and lock identity in the version-3 probe record, and the decoder includes both in canonical JSON and its fingerprint. Generated assertions compare current capacity and policy with emitted values. The two-event mock fixture exercises capacity two and strict-only capacity one; the scratch script checks the stale-capacity diagnostic and changed fingerprint. These are source and test-definition observations, not claims that I reran the script.

Demand-selected startup now prepares and stops the queue only when a deferred demand exists. Direct-only and zero-demand fixtures can declare capacity zero without instantiating runtime queue storage. The fixed queue still supports distinct event records, coalesces repeated pending events, and bounds accepted records. Its new dispatcher guard prevents a nested or competing consumer from taking a record while a callback runs. Mock and AVR lock primitives remain in their respective backend headers; the new policy identifiers add no board-owned peripheral programming.

The route checks for **already bound** events remain sound under inspection: generated dispatchers compare handler kind, context, and delivery against their binding. This does not cover the newly present, unbound event in P2-1. The generator’s source and entry choices still follow the validated binding plan; no manual second event list or new board-owned timer implementation was introduced. The inherited AVR board timer configuration and its single-source inventory remain open architectural limits, not regressions from this patch.

Inactive and unsupported boundaries remain explicit. `Stream`, software-only event discovery, ESP32 deferred dispatch, deadlines, named contexts, and silicon validation are deferred. The classic ESP32 direct route remains separate. I found no changed individual conditional attributes or unbraced new control-flow bodies in the inspected changes. Host and target build claims recorded in project documents were not treated as architectural proof or as tests performed by this review.

## 3 Risks and next action

Close P2-1 by checking the **complete live strict-mode catalog demand** against the probed/generated demand, then run the targeted zero-to-one and one-to-two handler mismatch cases. This review is NO-GO until that agreement check is present and verified. The prior capacity remediation can remain intact.

The current source review does not establish target runtime timing, AVR code size, or silicon behavior. The ATmega328P assessment uses the documented 16-bit `int` ABI only where relevant; no generic Grevir restriction was inferred from it. The exact reviewed tuple remained unchanged and clean at final verification.
