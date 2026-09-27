# Grevir Timer Ownership Checkpoint — STATE-AXIS REVIEW

**Object:** Remediation since first timer checkpoint root `09b954e2b9fe11f1df93ef278533e39afb81ca00`; prior grevir-core `1e4dae202da99e2064b92d7c68fb2ed981155598`, grevir-peripherals `74eb361b705b0e043841d91ebcf8475391fbcf69`, grevir-avr `bb40a478500968044887abd63c8131355154f8b0`.

**Reviewed tuple, verified at start and end:** root `747ce149d027de8654ea8e8c908dd54e280897cf`; grevir-base `4821b8d44c79adf93a9695d74d4d917e041c0552`; grevir-core `c593678a36814f62655d1bbb976a94a9c42c97e3`; grevir-peripherals `a89e6a8fa98c041f89a5d88eb72ff48a22574fa1`; grevir-avr `a883814ace2203b08eb1186c18c137c8ef74eb8f`; grevir-arduino-avr `0a5e262f77ce07f9e86cceb7d76184df24c008a5`. Other members were outside this review.

**Date:** 2026-09-27. **Axis:** State, setup and loop ordering, dependency closure, effect timing, claim timing, repeat setup, fail-closed composition, and retained hooks. **Verdict: NO-GO.** One new common allocator defect accepts a selected use whose declared pin or endpoint is reserved.

## Prior-finding closure

| Prior finding | Disposition on this tuple |
| --- | --- |
| State P1-1, timer writes before dependency setup | **Closed for the stated trigger.** `AllocatedApplication::runSetup()` calls the dependency-ordered `runSetupInterleaved()`; each prerequisite's parameter and setup phases finish before the owner's selected setup. `portable_pwm_test.cpp:5-40` checks both descriptor orders and records whether register writes see the prerequisite flag. |
| State P2-1, implementation parameter hooks suppressed | **Closed.** `RequestedModule::Bind::paramsSetup()` invokes selected setup and then `Impl::paramsSetup()` once; `paramsLoop()` invokes `Impl::paramsLoop()` once. The fixture counts both custom hooks. |
| Code P0-1, Uno/Nano board D9 versus device PB1 | **Closed for the Arduino AVR adapter path inspected.** `CanonicalGPIO<9>` resolves to the extracted PB1 identity, Core's claim comparison uses the canonical value, and `ReservationsFromClaims` applies that comparison to candidate pins before selection. The adapter's claim probe and mapping cover D9/PB1; this does not close the separate common allocator finding below. |

## Changed-range analysis

The Core change composes the existing implementation hooks with selected owner setup and interleaves setup by dependency. Its bound module still receives only its declared owner view. The AVR change derives one selected payload per owner, attaches whole-timer and pin claims to that owner, and turns existing module claims into reservations before solving. Arduino AVR adds canonical board-pin mapping at its adapter include boundary. Those paths resolve the prior state and alias triggers by source trace.

**NEW ARCHITECTURAL root cause:** the new use-neutral `timer::Candidate` envelope represents a binding's endpoint and pin separately from `exclusive_roles`, but `timer::solve()` does not consult those binding fields when applying reservations. The AVR translator duplicates both fields into roles, masking this defect for its current PWM candidates. The common solver's supported mock joint candidate does not duplicate its pin into roles, so the common plan is not fail-closed on its own declared footprint.

## 0 Evidence base

Read-only inspection of the exact member diffs and current sources: `grevir-core/src/grevir/core/{allocated_application,module,application,lifecycle_order,resource_claims,resource_checks}.hpp`; `grevir-peripherals/src/grevir/peripherals/timer/{own,owner_allocator}.hpp`; `grevir-peripherals/tests/timer_owner_static_tests.cpp`; `grevir-avr/src/grevir/avr/devices/atmega328p/{pwm_backend,pwm_candidates,pwm_program}.hpp`; AVR portable PWM fixture, test and probe; and Arduino AVR GPIO claim mapping and probe. Authority: `AGENTS_GWZ.md`, `CrossMcu.md`, `Avr.md`, `GrevirTimerModuleDesign.md`, the checkpoint, the first State review, remediation plan, and declarative dos/don't-dos. No build, test, hardware run, or source mutation was performed during inspection. The previously reported full host build and 187 passing CTests were accepted as existing evidence, not independently repeated.

## 1 Findings

### P0-1 — A reservation of a candidate binding pin or endpoint can be accepted

- **Location/root cause:** `grevir-peripherals/src/grevir/peripherals/timer/owner_allocator.hpp:206-214` compares each reservation only with `candidate.timer` and `candidate.exclusive_roles`. The same candidate explicitly carries `bindings[i].endpoint` and `bindings[i].pin` at lines 37-54. `valid_candidate()` at lines 95-121 does not require either field to be repeated in `exclusive_roles`.
- **Trigger/reproduction:** Use the existing supported mock `drive` candidate from `grevir-peripherals/tests/timer_owner_static_tests.cpp:23-28`: its PWM binding has endpoint `201` and pin `301`, while its exclusive roles are `402,403`. `timer::compile(timer::Problem{timer::demands<Drive>(), std::array{drive}, std::array{301u}})` returns `Status::success` by the shown solver branches; replacing `301u` with `201u` also returns success. The candidate's own declared resource is reserved. This is a source-derived constant-evaluation reproduction; no new probe was executed.
- **Scope/classification/provenance:** Shared timer owner allocator and its supported host mock; correctness defect introduced by the new common envelope. The current ATmega328P PWM translation at `pwm_program.hpp:101-107` repeats endpoints and pins in `exclusive_roles`, so this exact trigger does not establish an AVR PWM failure. A later backend following the envelope's field semantics is affected without extra copying.
- **Impact/invariant:** The plan reports success and permits provider effects despite a stated reservation of the same pin or endpoint. Static selection therefore cannot establish exclusive ownership from the candidate's complete declared footprint. This is false composition at the common boundary, and the defect is hidden by the existing mock's empty-reservation tests.
- **Correction:** Check reservations against every occupied binding endpoint and nonzero pin as well as whole timer and extra exclusive roles. Keep canonical physical identities consistent across those fields; validate or normalize any deliberate duplicate representation.
- **Closure test:** With the existing `drive` candidate, assert `reserved` for reservations `301` and `201`, and success for an unrelated identity. Repeat with reversed demand/candidate order and with a candidate whose pin is also copied into roles. Keep an AVR D9/PB1 reservation check through `ReservationsFromClaims` and a final cross-module claim conflict check.

## 2 Invariant analysis

`ModuleClosure` gathers descriptor dependencies; `LifecycleOrder` places them before their consumers. For the `MotorModule` fixture, the prerequisite's `runSetup()` runs before `SelectedTimerParameter::runSetup()` and before the motor callback in either descriptor order. The selected setup executes through the owner descriptor's `Requests`, and `setup_owner<Name>()` resolves that owner's selected choice. Empty-request modules receive an empty selected parameter; foreign `Pwm` access is statically rejected by `OwnerView`. `Impl::paramsSetup()` and `Impl::paramsLoop()` remain reachable once per normal application call.

For ATmega328P PWM, `ReservationsFromClaims` checks whole-timer claims, timer range/shared-use claims, and each generated physical output pin before the owner solver. Arduino AVR's `CanonicalGPIO` maps a board pin claim such as D9 to the extracted PB1 identity when the adapter header is used. Final Core claim checking also compares canonical GPIO identities and selected owner claims before `runSetup()` can execute. These are distinct from the common `Problem::reservations` gap in P0-1.

`Selected::setup()` stops the chosen timer, writes count and selected TOP, initializes the chosen outputs, then writes waveform and clock-select fields. It is reached once per owner descriptor during one `AllocatedApplication::runSetup()` call. A second explicit call to `runSetup()` repeats this sequence and repeats module hooks; the checkpoint defines no idempotent/restart API, so this is recorded as a bounded lifecycle contract risk, not a defect. The existing `Application::runLoop()` runs all parameter loops before module loops; the changed allocation path retains that behavior. No source-backed violation of the checkpoint's fixed-PWM loop contract was found.

## 3 Risks and next action

Repair the common reservation comparison and add a negative constant-evaluation case using the already published joint candidate. Then re-review the allocator's complete footprint and repeat the existing AVR/Arduino claim checks. The no-request path, unsupported AVR event-use rejection, dependency-first setup, retained hooks, and selected AVR register path were inspected; no additional actionable state defect was established. Physical silicon behavior, non-PWM AVR implementation, and ESP32 timer implementation remain outside this review.
