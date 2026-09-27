# Grevir Timer Ownership Checkpoint — STATE-AXIS REVIEW (round 3)

**Date:** 2026-09-27. **Verdict: NO-GO.** The original reservation counterexample is closed, but the common allocator still accepts two owners with overlapping declared footprints. The foreign-module write boundary also remains bypassable.

**Reviewed tuple, verified clean at start and end:** root `542505d3bc03cc2af934a9fda65837daa92cb355`; grevir-peripherals `6553200b95f533ba77e5b123c9cc1f1896e1cc7f`; grevir-avr `8a67186d5ad9cbcf59a2cb03836fa0349d7f103a`; grevir-core `c593678a36814f62655d1bbb976a94a9c42c97e3`; grevir-base `4821b8d44c79adf93a9695d74d4d917e041c0552`; grevir-arduino-avr `0a5e262f77ce07f9e86cceb7d76184df24c008a5`.

**Baseline:** root `747ce149d027de8654ea8e8c908dd54e280897cf`; grevir-peripherals `a89e6a8fa98c041f89a5d88eb72ff48a22574fa1`; grevir-avr `a883814ace2203b08eb1186c18c137c8ef74eb8f`. Core, Base, and Arduino AVR are unchanged.

| Prior finding | Round-3 disposition |
| --- | --- |
| State P0-1, reserved binding pin or endpoint accepted | **Closed for its stated trigger.** The reservation loop now checks timer, every binding endpoint, every nonzero binding pin, and extra roles. New constexpr assertions cover pin `301`, endpoint `201`, and unrelated reservation `999`. |
| Code P0-1, same reservation defect | **Closed for its stated trigger.** AVR no longer has to copy endpoints and pins into extra roles for reservation checking. |
| Code P1-1, foreign writable binding | **Still open in effect.** Direct naming of `RawPwm` and `View` is blocked, but template deduction can rebind the view type supplied to a foreign module. The new negative probes cover direct names only. |

## Changed-range analysis

Peripherals adds five reservation comparisons and four constexpr assertions. AVR makes the selected binding and owner view private, removes redundant role copies, and adds two direct-access negative probes. Root changes update the checkpoint and remediation record, test documentation, and example packaging. Core’s dependency ordering, hooks, and final claim validation are unchanged from round 2.

**NEW ARCHITECTURAL root cause:** The common allocator represents one canonical resource footprint across timer, bindings, and extra roles, but `compatible()` compares those fields in separate categories. The reservation repair checks all categories against an external ID; pairwise candidate compatibility still does not. This is a distinct false-composition path and, under the stated third-root-cause rule, stops this lane.

## 0 Evidence base

I read `AGENTS_GWZ.md`, the CrossMcu and Avr review policies, `GrevirTimerModuleDesign.md`, the controlling checkpoint, remediation plan 2, and the prior Code and State reports. I inspected the exact baseline-to-current diffs and the connected allocator, AVR translation, Core lifecycle, claims, and changed tests. This was read-only source review: no edit, build, test, or hardware run. The lane owner’s full host build, 187 passing CTests, and installed example are reported evidence, not independently repeated here.

## 1 Findings

### P0-1 — Two owners can select the same canonical resource across footprint fields

- **Location and root cause:** `grevir-peripherals/src/grevir/peripherals/timer/owner_allocator.hpp:141–156`. `compatible()` compares timer to timer, binding endpoint to endpoint, binding pin to pin, and extra role to extra role. It never compares an extra role with another candidate’s binding pin or endpoint. The candidate model calls these canonical resource identities at lines 35–53; the repaired reservation check at lines 207–216 treats them as one ID space.
- **Trigger and source-derived reproduction:** Copy `clock` from `tests/timer_owner_static_tests.cpp:18–22` and change its sole extra role from `401` to `301`, the `drive` candidate’s PWM pin at lines 23–27. Compile both owners with no reservations. Both candidates pass `valid_candidate()` and `fits()`. Their timers differ; their endpoints differ; `clock` has no pin; its role `301` differs from `drive`’s roles `402,403`. `compatible()` therefore returns true and the plan reports success. Reversing demand and candidate order does not remove the collision.
- **Scope, classification, provenance:** Shared allocator and its supported mock candidate model; correctness defect inherited from the initial use-neutral envelope, exposed by this round’s complete-footprint audit. Current ATmega328P PWM candidates have no extra roles, so this reproduction does not establish an AVR hardware failure.
- **Impact:** Two selected owners can each receive authority over canonical ID `301`. No generic Core claim pass follows this mock plan, and AVR’s selected claims cover its timer and pins, not arbitrary common extra roles. The common solver cannot prove exclusive ownership for a backend using the documented extra-role field. **P0: false composition.**
- **Fix and closure test:** Compare the union of each candidate’s timer, binding endpoints, nonzero pins, and extra roles against the other candidate’s union, preserving any explicit containment semantics. Add constexpr conflict cases for role versus pin and role versus endpoint in both owner/candidate orders; retain success for disjoint IDs. Recheck AVR reservations and final claims after the common change.

### P1-1 — A foreign module can rebind its private view and write the motor PWM

- **Location and root cause:** `grevir-avr/src/grevir/avr/devices/atmega328p/pwm_program.hpp:49–66,263–297` and `grevir-core/src/grevir/core/allocated_application.hpp:50–61`. `RequestedModule::Bind` passes `Allocation::View<Requests>` as the foreign module’s public template argument. Although `View` is privately named, a generic template can deduce that template from the supplied specialization and instantiate it with different `Requests`. `OwnerScope` trusts the rebound argument to authorize `Pwm`.
- **Trigger and source-derived reproduction:** A module with empty requests receives `Plan = View<setl::TypeArgs<>>`. A partial specialization of the form `template<template<class> class V, class R> struct Rebind<V<R>> { using type = V<setl::TypeArgs<MotorRequest>>; };` recovers `V` without spelling the private member name. `Rebind<Plan>::type::Pwm<"left">` then resolves to the motor’s `RawPwm`, whose `write()` is public and whose `Claims` are empty. The motor request is available as a named application type. The new cases 10 and 11 in `portable_pwm_probe.cpp:72–77` try only direct private-name access; they do not exercise this route.
- **Scope, classification, provenance:** AVR owner capability boundary reached through shared module binding; correctness defect persisting from prior Code P1-1 despite this round’s partial repair. This is a source-derived C++ template-deduction path; no compile probe was run under the read-only constraint.
- **Impact:** An unrelated module can update the selected motor PWM without declaring the use or creating a conflicting claim. **P1: release blocker for the claimed owner-only update boundary.**
- **Fix and closure test:** Bind write authority to the original owner view identity rather than to a caller-supplied `Requests` template argument. Add a compile-negative foreign-module probe that uses template-template deduction to rebind its received view, while retaining the positive motor write probe.

## 2 Invariant analysis

The repaired reservation loop rejects the exact prior mock pin and endpoint counterexamples and accepts the unrelated ID. AVR’s translator now relies on those binding-field checks, while `ReservationsFromClaims` still converts timer and physical pin claims before solving. The unchanged Arduino AVR adapter normalizes Uno/Nano D9 to device PB1, and the unchanged Core final claim check precedes `runSetup()`. Those paths do not cover the cross-category common candidate collision in P0-1.

The unchanged dependency closure and `LifecycleOrder` place prerequisite setup before the owning module’s selected timer setup. `RequestedModule::Bind::paramsSetup()` then runs selected setup and the implementation parameter hook; `paramsLoop()` retains the implementation loop hook. The existing fixture checks prerequisite visibility at register writes and hook counts. The no-request plan remains empty. Active AVR event uses still reach an explicit unsupported-use assertion. Physical silicon timing, non-PWM AVR implementation, and ESP32 timer implementation are deferred and are not claimed here.

## 3 Risks and next action

Stop the checkpoint lane under the stated third new architectural root-cause rule. Repair complete-footprint pairwise compatibility and the foreign-view rebinding path, then review the concrete negative probes and unchanged AVR/Core claim and setup paths on a newly pinned tuple. The changed reservation tests are useful for their direct inputs, but they do not establish either missing invariant.
