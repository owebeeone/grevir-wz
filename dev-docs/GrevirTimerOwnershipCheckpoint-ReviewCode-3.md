# Grevir Timer Ownership Checkpoint — CODE-AXIS REVIEW (round 3)

**Reviewed tuple, verified at start and end:** root `542505d3bc03cc2af934a9fda65837daa92cb355`; grevir-peripherals `6553200b95f533ba77e5b123c9cc1f1896e1cc7f`; grevir-avr `8a67186d5ad9cbcf59a2cb03836fa0349d7f103a`; grevir-core `c593678a36814f62655d1bbb976a94a9c42c97e3`; grevir-base `4821b8d44c79adf93a9695d74d4d917e041c0552`; grevir-arduino-avr `0a5e262f77ce07f9e86cceb7d76184df24c008a5`. All six working trees were clean.

**Date / axis:** 2026-09-27 / CODE. **Verdict: NO-GO.** Two previously identified architectural boundaries remain incomplete: the common solver can accept overlapping selected footprints, and public AVR setup methods let a foreign module reconfigure another owner’s timer. I found no third new architectural root cause.

| Prior finding | Round 3 disposition |
| --- | --- |
| Code P0-1, reserved binding pin or endpoint | **Exact trigger closed.** `solve()` now compares reservations with each binding endpoint and nonzero pin. Constexpr cases cover pin `301`, endpoint `201`, an unrelated reservation, and reversed candidate order. The broader footprint comparison remains open in Finding 1. |
| State P0-1, same reservation defect | **Exact trigger closed** by the same allocator change. |
| Code P1-1, foreign access to raw PWM binding | **Exact extraction path closed.** `View`, `RawPwm`, and `Selected` are private; negative probes cover direct raw access and forged views. The broader owner-only configuration boundary remains open in Finding 2. |

The changed source range is small: Peripherals adds reservation comparisons and constexpr cases; AVR makes the owner view and raw writer private, removes duplicated binding resources from extra roles, and adds compile-negative probes. Core, Base, and Arduino AVR are unchanged from the preceding review. Root changes update the checkpoint and consumer documentation.

## 0 Evidence base

I read `AGENTS_GWZ.md`, `dev-docs/review-policies/CrossMcu.md`, `dev-docs/review-policies/Avr.md`, `dev-docs/GrevirTimerModuleDesign.md`, the controlling checkpoint, round 2 remediation plan, and both prior Code and State reports. I inspected baseline-to-current diffs and immediate source call paths with read-only commands. I made no edits and ran no builds or tests. The lane owner’s full host build, 187 CTests, and installed consumer example are separate evidence; no silicon result is claimed.

## 1 Findings

### P0-1 — Selected candidates can overlap across binding and extra-role fields

- **Location:** `grevir-peripherals/src/grevir/peripherals/timer/owner_allocator.hpp:141–157`, especially `compatible()`’s separate binding-to-binding and role-to-role comparisons.
- **Trigger:** Start with the supported `clock` and `drive` mock candidates in `tests/timer_owner_static_tests.cpp:18–28`. Give `clock.exclusive_roles[0]` the value `301`, which is `drive.bindings[0].pin`. Keep their distinct timers and all other fields. Both candidates pass `valid_candidate()`. `compatible()` returns true because it never compares an extra role with another candidate’s binding pin or endpoint; `compile()` therefore selects both owners.
- **Scope / classification / provenance:** Shared allocator correctness defect, present before this round. It is the **same incomplete-footprint-comparison root cause** identified in round 2, left partially repaired by the reservation-only change. The supported mock model reaches it; this reproduction does not establish an ATmega328P PWM collision.
- **Impact:** The common plan can declare success while two modules occupy the same canonical physical resource. This is false composition at the ownership boundary.
- **Correction and closure test:** Compare all occupied resource identities through one canonical footprint operation for both reservations and candidate compatibility. Add a constexpr two-owner case with one candidate’s extra role equal to the other’s binding pin, then repeat for its endpoint and reversed declaration order; each must reject the joint plan.

### P1-1 — Public AVR setup entry points bypass the owner view

- **Location:** `grevir-avr/src/grevir/avr/devices/atmega328p/pwm_program.hpp:169–176,248–260`. `setup_owner<Name>()` publicly calls private `Selected<...>::setup()`, which writes timer registers at lines 222–240.
- **Trigger:** A module with empty timer requests can define its `runSetup()` after the application alias and call `App::Allocation::setup_owner<MotorRequest::name>()`. It need not name the private `View`, `RawPwm`, or `Selected`, and the method checks the selected owner’s existence rather than the caller’s identity. Public `setup_selected()`, `setup_all()`, and `setup()` provide further setup paths.
- **Scope / classification / provenance:** AVR binding and shared module-ownership contract; correctness defect inherited from the earlier public setup surface. This is the **same broader writable-authority root cause** as prior Code P1-1, although the specific raw PWM extraction route is closed.
- **Impact:** A foreign module can stop and reinitialize the motor owner’s timer and outputs without declaring a use or receiving an owner service. Static claims do not detect that write.
- **Correction and closure test:** Confine selected setup operations to the lifecycle binding for the declared owner, while retaining public read-only plan inspection. A compile-negative probe should attempt the foreign `App::Allocation::setup_owner<MotorRequest::name>()` call; the legitimate owner’s dependency-ordered setup must still compile and run.

## 2 Invariant analysis

The original reserved-pin `301` and reserved-endpoint `201` counterexamples are now rejected by the shared solver. AVR no longer needs to copy those binding identities into `exclusive_roles` for reservation checking. Its active non-PWM use still reaches the explicit unsupported-use assertion. The private nested view prevents the reviewed `Allocation` extraction, raw `Pwm` access, forged view, and direct `Selected` writer attempts. Existing dependency ordering and Arduino AVR board/device pin normalization are unchanged in this range.

Whole-timer exclusivity remains enforced when two candidates name the same timer. Complete footprint exclusivity does **not** hold when one candidate’s extra role equals another’s binding endpoint or pin. Owner-only configuration also does **not** hold while the allocation exposes public register-writing setup methods.

## 3 Risks and next action

Repair both incomplete boundaries and rerun the focused negative probes plus the required host gates before another verdict. These findings extend the two already known root causes; they do not introduce a third new architectural root cause. Physical silicon validation and non-PWM AVR or ESP32 backend implementation remain deferred by the checkpoint.
