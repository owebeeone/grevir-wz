# Grevir Timer Ownership Checkpoint — CODE-AXIS REVIEW (focused re-verdict)

**Date:** 2026-09-27. **Verdict: NO-GO (P1).** The footprint defects and the exact foreign-access probes from round 3 are closed. Two source-derived paths still let foreign code exercise another module’s timer authority. Both belong to the previously known owner-authority root; I found no genuinely new architectural root cause.

**Reviewed tuple, verified clean at start and end:** root `66b751a771b186bbaf964ed42180eed8a729a9b0`; grevir-peripherals `0cf6a7d7d3358dcb3df9474c8e190d5217029e69`; grevir-avr `9c1b570d43ed28e8d1d799786de8572b087d4cc5`; grevir-core `c593678a36814f62655d1bbb976a94a9c42c97e3`; grevir-base `4821b8d44c79adf93a9695d74d4d917e041c0552`; grevir-arduino-avr `0a5e262f77ce07f9e86cceb7d76184df24c008a5`. All six working trees were clean.

| Prior finding | Focused disposition |
| --- | --- |
| Code/State P0-1, cross-field footprint collision | **Closed.** `each_resource()` visits timer, binding endpoints, nonzero binding pins, and roles. Both reservations and pairwise compatibility use it. Changed constexpr cases reject role-versus-pin and role-versus-endpoint collisions in opposite candidate orders while preserving disjoint selection. |
| Code P1-1, public setup entry points | **Direct calls closed; authority remains exposed.** `setup_owner()`, `setup_selected()`, `setup_all()`, and `setup()` are private. Finding 1 identifies a public Core wrapper that can still invoke `setup_owner()` for a named foreign owner. |
| State P1-1, template-template view rebinding | **Exact pattern closed; foreign write remains possible.** The nested `ViewStorage<Requests>::type` prevents deduction of the old one-parameter `View` template. Finding 2 identifies a public bound-module alias that exposes the legitimate owner view itself. |

The changed source range is Peripherals’ footprint traversal and constexpr cases, AVR’s view storage and setup access, and three added AVR negative probes. Core, Base, and Arduino AVR are unchanged from the round-3 tuple. Root changes record the remediation and prior reports.

## 0 Evidence base

I read `AGENTS_GWZ.md`, the CrossMcu and Avr policies, `GrevirTimerModuleDesign.md`, the checkpoint and remediation plan, both round-3 reports, and the State erratum. I inspected baseline-to-current diffs and the connected allocator, AVR binding, Core lifecycle, claim, and probe paths with read-only commands. I made no edits and ran no builds or tests. The lane owner’s full host build, 187 CTests, and installed example are separate evidence; no silicon result is claimed.

## 1 Findings

### P1-1 — The public lifecycle parameter can set up a foreign owner’s timer

- **Location and root cause:** `grevir-core/src/grevir/core/allocated_application.hpp:24–37`; `grevir-avr/src/grevir/avr/devices/atmega328p/pwm_program.hpp:54–55,177–183`. AVR grants friendship to every `SelectedTimerParameter<Allocation_, Requests_>` specialization. Its public `runSetup()` calls private `setup_owner<First::name>()`, which writes the selected timer through `Selected::setup()`.
- **Trigger and source-derived reproduction:** With the probe’s public `App` and `Request` types, foreign code can call `grevir::nfp::SelectedTimerParameter<App::Allocation, setl::TypeArgs<Request>>::runSetup()`. It need not be the motor module, hold its view, or add a timer claim. The new cases 13 and 14 attempt only direct calls to `App::Allocation::setup_owner()` and `setup()`.
- **Scope, classification, provenance:** AVR setup authority reached through a public Core lifecycle template; correctness defect continuing the known foreign-setup root. The private access change was introduced in this range, while the callable wrapper and its public `runSetup()` are inherited.
- **Impact:** Unrelated code can reinitialize the selected motor timer and outputs after owner setup. Static ownership and claims do not constrain that call. **P1 release blocker.**
- **Fix and closure test:** Bind setup authority to the lifecycle instance that owns the declared request, and prevent arbitrary callers from constructing or invoking an equivalent setup operation. Add a compile-negative probe for the exact `SelectedTimerParameter<Allocation, setl::TypeArgs<Request>>::runSetup()` call. Keep the positive dependency-ordered owner setup probe.

### P1-2 — A public bound-module alias reveals the legitimate owner view

- **Location and root cause:** `grevir-core/src/grevir/core/allocated_application.hpp:50–56`; `grevir-avr/src/grevir/avr/devices/atmega328p/pwm_program.hpp:58–72,269–302`. `RequestedModule::Bind<Allocation>::Impl` is a public alias for `Module<View<Requests>>`. A caller can deduce the view type from that alias without naming the private `View` template or rebinding it. The recovered legitimate view exposes `Pwm<"pwm">`, whose `write()` is public.
- **Trigger and source-derived reproduction:** Instantiate the probe’s `MotorModule::Bind<App::Allocation>::Impl`, then partially specialize a trait for `Motor<Plan>` to recover `Plan`. `Plan::Pwm<"pwm">::write(1,2)` reaches the motor’s selected PWM. The foreign module does not declare the motor request. New case 12 tests template-template rebinding of the *foreign* view; it does not test recovery of the *owner’s* view through `Impl`.
- **Scope, classification, provenance:** Shared module binding and AVR owner capability; correctness defect continuing the known foreign writable-binding root. The public `Impl` exposure is inherited; opaque `ViewStorage` does not close this route.
- **Impact:** A foreign module can update the motor’s PWM without owning its timer or pin. **P1 release blocker.**
- **Fix and closure test:** Keep the owner’s writable view inside its binding boundary rather than exposing `Module<View>` through a public alias usable to recover that view. Add a compile-negative probe using `MotorModule::Bind<App::Allocation>::Impl` and a `Motor<Plan>` extraction trait; preserve the positive motor binding and write probe.

Both reproductions follow directly from public template and member access in the pinned source. They still require dedicated compile probes for closure; I did not run a compiler under this read-only assignment.

## 2 Invariant analysis

The shared solver now compares the same complete candidate footprint against reservations and other candidates. The added tests cover the original role-versus-pin and role-versus-endpoint counterexamples in both selection orders, and the disjoint positive plan remains. AVR’s unchanged device-derived timer, channel, and pin identities introduce no new alias IDs in this change; its translation no longer copies binding identities into extra roles.

The direct `Allocation::setup_owner()` and `Allocation::setup()` calls are inaccessible to foreign code. The exact prior template-template rebind no longer matches the opaque nested view type. Positive owner binding still follows `RequestedModule::Bind` → `SelectedTimerParameter::runSetup()` → private `setup_owner()` in dependency order; the probe’s case 0 is the lane owner’s reported positive compile control.

The active AVR backend remains fixed-frequency PWM. `request()` retains its explicit `GREVIR_PWM_BACKEND_UNSUPPORTED_TIMER_USE` assertion for an active non-PWM use. Silicon behavior, a non-PWM AVR backend, and an ESP32 timer backend remain deferred.

## 3 Risks and next action

**NO-GO** until both public authority paths are closed and their exact negative probes pass alongside the positive owner setup and write controls. Recheck the required host gates on the repaired tuple. These are further routes through the previously known owner-authority defect, not a new architectural root that triggers the lane cap.
