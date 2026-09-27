# Grevir Timer Ownership Checkpoint — STATE-AXIS REVIEW (final focused re-verdict)

**Date:** 2026-09-27. **Verdict: GO for the focused ownership checkpoint.** I found no P0–P2 defect in the specified in-application lifecycle and binding routes.

**Tuple verified at start and end; all six working trees clean:** root `e364345c0e0465007ba607bc2c6a4d8c4c078ef6`; Core `852a38aedf9bc7bc7569e971b91d4a8d08821b11`; AVR `9f6bc02cc232d8a60a7b16a1ecdf16bfc8c42e0a`; Peripherals `0cf6a7d7d3358dcb3df9474c8e190d5217029e69`; Base `4821b8d44c79adf93a9695d74d4d917e041c0552`; Arduino AVR `0a5e262f77ce07f9e86cceb7d76184df24c008a5`.

| Prior root or route | Final focused disposition |
| --- | --- |
| Cross-field candidate footprint collision | **Closed in the prior tuple; unchanged here.** Peripherals has no source change in this range. |
| Public AVR allocation setup calls | **Closed.** `setup_owner()` and the aggregate setup functions remain private. |
| Public `SelectedTimerParameter::runSetup()` surrogate | **Closed.** `runSetup()` is private; `RequestedModule` alone is friended for the intended binding call. |
| Public bound `Impl` alias revealing the owner view | **Closed for the reported route.** `Bind::Impl` is private. |
| Public application allocation alias | **Closed.** `Assemble::Allocation`, requests, claims, and modules are private; read-only plan and claim inspection remain public. |
| Earlier template-template view rebinding | **Closed in the prior tuple; unchanged here.** |

**Changed-range analysis:** Relative to Core `c593678a36814f62655d1bbb976a94a9c42c97e3`, Core adds private boundaries around the lifecycle call, bound implementation alias, and application assembly types. Relative to AVR `9c1b570d43ed28e8d1d799786de8572b087d4cc5`, source behavior is unchanged; the fixture now uses the public inspection aliases, and compiler cases 15–17 attempt the exact prior wrapper, `Impl`, and allocation accesses. Peripherals, Base, and Arduino AVR are unchanged. Root changes record the remediation and review reports.

## 0 Evidence base

I read `AGENTS_GWZ.md`, the CrossMcu and AVR policies, `GrevirTimerModuleDesign.md`, the checkpoint, RemPlan-4, and the prior Code and State reports. I inspected the changed diffs and connected Core lifecycle, AVR allocation and binding, positive fixture, and compiler-probe sources. This was read-only review: no edits, builds, tests, or hardware runs. The lane owner reports a full build, 187 tests, and an installed consumer; those are separate validation evidence.

## 1 Findings

**No actionable P0–P2 finding.** The exact formerly public call `SelectedTimerParameter<Allocation, Requests>::runSetup()` now fails access control at [allocated_application.hpp](/Users/owebeeone/limbo/grevir-wz/grevir-core/src/grevir/core/allocated_application.hpp:39). The reported `Bind::Impl` and `App::Allocation` accesses likewise encounter private aliases at lines 60 and 89–91. AVR’s private setup entry remains at [pwm_program.hpp](/Users/owebeeone/limbo/grevir-wz/grevir-avr/src/grevir/avr/devices/atmega328p/pwm_program.hpp:177).

## 2 Invariant analysis

The positive lifecycle remains connected: Core orders dependency modules, invokes the bound owner’s `paramsSetup()`, calls private selected-timer setup, then calls the module’s parameter hook and setup callback. The fixture checks prerequisite visibility during register writes, callback order, hook execution, and one TOP write. The changed probes cover the intended access rejections; I inspected their source and did not execute them.

The friendship is broad across `RequestedModule` specializations, and `Bind::paramsSetup()` remains public. Using that to act as a foreign owner requires deliberately reconstructing an AVR allocation type from public templates. I found no path from the actual application’s public inspection surface or a foreign module’s opaque view to its private allocation or the owner’s bound view without that reconstruction. Under this review’s stated concrete in-application scope, this is a bounded design limitation, not a release-blocking finding. It is a continuation of the known writable-authority concern, not a new architectural root cause.

## 3 Risks and next action

The focused checkpoint can advance on the lane owner’s reported validation. Its scope remains fixed-frequency ATmega328P PWM; joint PWM/event AVR candidates, the ESP32 timer backend, and silicon behavior remain outside this verdict.
