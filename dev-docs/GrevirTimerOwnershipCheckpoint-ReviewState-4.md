# Grevir Timer Ownership Checkpoint — STATE-AXIS REVIEW (focused re-verdict)

**Date:** 2026-09-27. **Verdict: NO-GO.** The complete-footprint collision and template-template view rebinding counterexamples are closed. A foreign caller can still reinitialize another module’s timer through a public Core lifecycle parameter. This is the previously known writable-authority root, not a new architectural root.

**Reviewed tuple, verified at start and end:** root `66b751a771b186bbaf964ed42180eed8a729a9b0`; grevir-peripherals `0cf6a7d7d3358dcb3df9474c8e190d5217029e69`; grevir-avr `9c1b570d43ed28e8d1d799786de8572b087d4cc5`; grevir-core `c593678a36814f62655d1bbb976a94a9c42c97e3`; grevir-base `4821b8d44c79adf93a9695d74d4d917e041c0552`; grevir-arduino-avr `0a5e262f77ce07f9e86cceb7d76184df24c008a5`. All six working trees were clean.

| Prior finding | Focused disposition |
| --- | --- |
| Code/State P0-1: role versus another candidate’s binding pin or endpoint | **Closed.** One `each_resource` traversal feeds pairwise compatibility. Source logic is symmetric; constexpr cases reject role–pin and role–endpoint collisions with opposite candidate orders, while the disjoint pair succeeds. |
| Code P1-1: public AVR setup authority | **Still open through a different public call path.** Direct `Allocation::setup_owner()` and `Allocation::setup()` are private, but the public, friended `SelectedTimerParameter::runSetup()` can call `setup_owner()` for an owner request supplied by a foreign caller. |
| State P1-1: template-template rebinding of the foreign view | **Closed for the reported pattern.** The view is now a nested opaque type. The new negative probe attempts the reported partial-specialization deduction; the positive owner binding remains present. |

**Changed-range analysis:** Relative to root `542505d3bc03cc2af934a9fda65837daa92cb355`, Peripherals `6553200b95f533ba77e5b123c9cc1f1896e1cc7f`, and AVR `8a67186d5ad9cbcf59a2cb03836fa0349d7f103a`, Peripherals replaces separate resource comparisons with `each_resource` and adds constexpr collision cases. AVR changes the view’s type shape, makes setup entry points private, friends Core’s selected-timer parameter, and adds compile-negative probes. Core, Base, and Arduino AVR source commits are unchanged. Root changes document the remediation and prior reports.

## 0 Evidence base

I read `AGENTS_GWZ.md`, the CrossMcu and Avr review policies, `GrevirTimerModuleDesign.md`, the controlling checkpoint, RemPlan-3, the round-3 Code and State reports, and the State erratum. I inspected the baseline-to-current diffs and connected allocator, AVR binding, Core lifecycle, claim, and test sources. This was read-only review: no edits, builds, tests, or hardware runs. The lane owner reports a full host build, 187 CTests, and an installed example; I did not repeat those gates.

## 1 Findings

### P1-1 — Public lifecycle parameter still permits foreign timer reconfiguration

- **Location:** `grevir-core/src/grevir/core/allocated_application.hpp:31–37`; `grevir-avr/src/grevir/avr/devices/atmega328p/pwm_program.hpp:54–55,178–183,229–246`.
- **Trigger:** In a foreign module’s `runSetup()`, call `grevir::nfp::SelectedTimerParameter<App::Allocation, setl::TypeArgs<MotorRequest>>::runSetup()`. The template and `runSetup()` are public. Its `Claims` alias is merely formed by this invocation; it is not added to that module’s application claims. The AVR allocation friends every `SelectedTimerParameter` specialization, so this call reaches private `setup_owner<MotorRequest::name>()` and repeats the selected motor timer’s register setup.
- **Scope, classification, provenance:** AVR owner authority through the shared Core lifecycle; correctness defect from the **known writable-authority root**. The current change closes direct allocation calls but leaves this callable surrogate. This is source-derived C++ access and call-path evidence; I did not run a compile probe.
- **Impact:** A module with no timer request can stop and reconfigure the motor owner’s timer without an allocator claim or owner service. **P1 release blocker.**
- **Correction and closure test:** Ensure the callable lifecycle setup authority cannot be selected and invoked by arbitrary module code using a public `Allocation` and request type. Preserve dependency-ordered owner setup. Add a compile-negative foreign-module probe using the exact `SelectedTimerParameter<Allocation, MotorRequests>::runSetup()` expression, and a positive owner setup and hook test. Probes 13 and 14 currently cover only direct `App::Allocation` calls, so their failures do not establish this boundary.

## 2 Invariant analysis

`each_resource()` enumerates timer, binding endpoints, nonzero pins, and extra roles for both reservation checks and candidate compatibility. Its nested comparison rejects the original cross-field collision regardless of operand order. The constexpr cases exercise role–pin and role–endpoint conflicts in opposite orders, and the existing tests retain reserved pin `301`, reserved endpoint `201`, an unrelated reservation, and disjoint selection. AVR translates selected PWM endpoints into the common candidate and turns physical timer and pin claims into reservations. Core’s final claim check precedes setup; the unchanged Arduino AVR adapter normalizes board pins to device identities.

The nested view shape prevents the reported `RebindView<ViewTemplate<OldRequests>>` deduction from recovering a rebindable one-parameter view template. The negative probe specifically uses that pattern. The source also retains an owner binding positive control.

The owner’s normal lifecycle remains connected: Core orders prerequisite modules, invokes selected timer parameter setup, then invokes the implementation parameter hook. The fixture checks prerequisite visibility at register writes, callback order, setup and loop hooks, and one TOP write. These controls establish intended operation but do not reject the public lifecycle-parameter call in Finding P1-1. Active unsupported AVR event uses remain an explicit rejection in this checkpoint.

## 3 Risks and next action

Keep the checkpoint at **NO-GO** until the remaining owner-write route is closed and its exact negative probe passes alongside the positive dependency and hook controls. No genuinely new architectural root was found, so the third-root lane cap does not apply. Silicon behavior, non-PWM AVR backend work, and the ESP32 timer backend remain deferred.
