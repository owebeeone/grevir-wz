# Grevir Timer Ownership Checkpoint — CODE-AXIS REVIEW (final focused re-verdict)

**Date:** 2026-09-27. **Verdict: GO for the stated fixed-PWM checkpoint.** The three prior public access paths now fail at their intended private members. I found no P0–P2 defect or genuinely new architectural root cause.

**Tuple, verified clean at start and end:** root `e364345c0e0465007ba607bc2c6a4d8c4c078ef6`; Core `852a38aedf9bc7bc7569e971b91d4a8d08821b11`; AVR `9f6bc02cc232d8a60a7b16a1ecdf16bfc8c42e0a`; Peripherals `0cf6a7d7d3358dcb3df9474c8e190d5217029e69`; Base `4821b8d44c79adf93a9695d74d4d917e041c0552`; Arduino AVR `0a5e262f77ce07f9e86cceb7d76184df24c008a5`.

| Prior path | Closure |
| --- | --- |
| `SelectedTimerParameter<Allocation, MotorRequests>::runSetup()` | **Closed.** Nonempty-request `runSetup()` is private; probe 15’s recorded compiler error identifies that member. |
| `MotorModule::Bind<App::Allocation>::Impl` and owner-view trait extraction | **Closed at the reported alias.** `Impl` is private; probe 16’s recorded error identifies it. |
| `App::Allocation` exposure | **Closed.** `Assemble::Allocation` is private; probe 17’s recorded error identifies it. |
| Earlier complete-footprint collision | **Remains closed.** Peripherals is unchanged; one `each_resource()` traversal feeds reservation and pairwise checks. |

The changed source range is one Core header adding these access boundaries, three AVR negative probes, and the probe driver. AVR implementation, Peripherals, Base, and Arduino AVR have no source change from the specified baseline. Root changes record the remediation and prior reviews.

## 0 Evidence base

I read `AGENTS_GWZ.md`, the CrossMcu and Avr policies, `GrevirTimerModuleDesign.md`, the checkpoint, RemPlan-4, and prior Code/State round-4 reports. I inspected baseline-to-current diffs, the connected Core lifecycle and AVR binding code, the common allocator, positive fixtures, and the existing host probe logs. This was read only; I ran no build or test. The lane owner’s full build, 187 CTests, and consumer result are separate evidence.

## 1 Findings

**No P0–P2 findings within the checkpoint’s stated boundary.** The new probe logs fail specifically on private `runSetup`, `Impl`, and `Allocation`, rather than on unrelated template, allocation, or syntax errors. The positive owner path remains connected: `RequestedModule::Bind::paramsSetup()` invokes the friended selected parameter, then the module’s parameter hook; the owner fixture retains its setup and PWM write controls.

## 2 Invariant analysis

Core now exposes the selected plan and claims for inspection while keeping the assembled requests, allocation, and bound module list private. The AVR allocation’s setup operations and raw writer remain private. A foreign module receiving an empty opaque view cannot rebind that view through the previously reported template-template deduction, and the exact owner `Impl` and application allocation extraction paths are inaccessible.

The unchanged common allocator compares each selected candidate’s timer, binding endpoints, nonzero pins, and roles against the same complete footprint. The existing constexpr cases reject role–pin and role–endpoint overlap in opposite orders and retain a disjoint positive selection. AVR’s active non-PWM request still reaches the explicit `GREVIR_PWM_BACKEND_UNSUPPORTED_TIMER_USE` assertion. The compile-time claims and dependency-ordered setup checks remain in place.

## 3 Risks and next action

The public backend templates and `RequestedModule::Bind` can still be deliberately instantiated with a separately reconstructed allocation; its public `paramsSetup()` and inherited module API then warrant care. RemPlan-4 explicitly excludes a code-level security claim against that reconstruction, and I found no path from a foreign module’s supplied opaque view to the actual application allocation. This is a bounded limitation of the stated contract, not a new blocker.

Proceed with the checkpoint’s focused closure. Silicon behavior, additional AVR timer modes, and an ESP32 timer backend remain outside this verdict.
