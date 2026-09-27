# Grevir Timer Ownership Checkpoint — CODE-AXIS REVIEW

**Object:** Workspace root `747ce149d027de8654ea8e8c908dd54e280897cf`; `grevir-base` `4821b8d44c79adf93a9695d74d4d917e041c0552`; `grevir-core` `c593678a36814f62655d1bbb976a94a9c42c97e3`; `grevir-peripherals` `a89e6a8fa98c041f89a5d88eb72ff48a22574fa1`; `grevir-avr` `a883814ace2203b08eb1186c18c137c8ef74eb8f`; `grevir-arduino-avr` `0a5e262f77ce07f9e86cceb7d76184df24c008a5`  
**Baseline:** Root `09b954e2b9fe11f1df93ef278533e39afb81ca00`; Core `1e4dae202da99e2064b92d7c68fb2ed981155598`; Peripherals `74eb361b705b0e043841d91ebcf8475391fbcf69`; AVR `bb40a478500968044887abd63c8131355154f8b0`  
**Date / axis:** 2026-09-27 / CODE  
**Verdict: NO-GO.** The new common allocator can select a candidate whose declared endpoint pin is reserved. The attempted owner-scoped view also leaves the writable AVR binding reachable from a foreign module.

| Prior Code finding | Closure assessment |
| --- | --- |
| P0-1, board/device pin alias | Closed for the Arduino AVR adapter path: `CanonicalGPIO` maps Uno/Nano numeric pins through the selected board mapping, and `GrevirArduinoAVR.h` includes that specialization. The AVR reservation check uses `has_conflict`, so it sees the normalized identity when the adapter is included. |
| P1-1, foreign writable binding | **Open.** `OwnerView` gates its own `Pwm` alias, but retains the public `Allocation` type parameter and the public `Allocation::Pwm` accessor. See Finding 2. |
| P1-2, PWM-shaped owner boundary | Closed for the common envelope: `Own` collects use kinds without requiring a PWM pin; the mock solver represents event-only and joint PWM/event candidates. The AVR backend explicitly rejects unsupported event uses. |
| P2-1, order-derived candidate IDs | Closed on the inspected AVR path: requests are sorted and keys derive from hardware settings and owner-local endpoints rather than vector position. |
| P2-2, exact-timer/width diagnostic | Closed: `WidthGate` carries owner, resident target, explicit timer, required width and available width as template arguments. |

The changed range replaces the shared PWM-only owner wrapper with `timer::Own` and `owner_allocator.hpp`, translates AVR PWM choices into that envelope, adds an owner view and width gate, and normalizes Arduino AVR GPIO claims. The new architectural root cause is **incomplete footprint comparison in the common allocator**: endpoint and pin identities are checked between candidates, while reservations are checked only against timer and separately supplied role IDs. The AVR translator compensates by copying endpoints and pins into roles; the common contract does not enforce that duplication.

## 0 Evidence base

I read `AGENTS_GWZ.md`, the CrossMcu and Avr policies, `GrevirTimerModuleDesign.md`, the checkpoint, the prior Code review and remediation plan, and the declarative integration dos and don’ts. I inspected the changed source and its immediate call paths with read-only Git and source commands. The exact tuple above was verified at the start and end; all six repositories had clean status. I made no edits, ran no builds, and claim no silicon evidence. The supplied host-build and 187-test results remain separate evidence.

## 1 Findings

### P0-1 — A reserved endpoint pin remains eligible in the common plan

- **Location:** `grevir-peripherals/src/grevir/peripherals/timer/owner_allocator.hpp:207–211`; candidate fields at lines 38–53 and validation at lines 95–120.
- **Trigger and reproduction:** Use the existing `drive` candidate shape from `timer_owner_static_tests.cpp`: PWM binding `{"drive","output"}` has endpoint `201` and pin `301`, while its extra roles are `402` and `403`. Call `timer::compile` with that candidate, its matching demand, and reservation `{301}`. `valid_candidate` and `fits` accept it. The reservation loop compares `301` only to the timer ID and roles `402`/`403`, so the plan succeeds despite the reserved PWM pin. The same omission applies to an endpoint ID reserved without a duplicated role.
- **Scope / classification / provenance:** Shared timer allocator; correctness defect introduced with the new use-neutral envelope. This produces a false composition in its supported mock plan. The current AVR translator at `pwm_program.hpp:105–106` manually duplicates each channel and pin into roles, masking this defect for its generated candidates.
- **Impact:** A backend can provide an otherwise valid, complete candidate and a board reservation for its stated pin, yet receive a successful plan. `compatible()` already treats binding pins and endpoints as physical conflicts between two candidates, so the reservation result contradicts the allocator’s own footprint semantics.
- **Correction:** Compare reservations with the timer, every binding endpoint, every nonzero binding pin, and additional exclusive roles. Keep a single typed or canonical footprint representation so translators need not duplicate binding identities into roles.
- **Closure test:** Add a constexpr case using a candidate with pin `301`, distinct extra roles, and reservation `{301}`; require `Status::reserved`. Repeat for its endpoint ID, plus an unrelated reservation that succeeds.

### P1-1 — A foreign module can recover the full allocation and write another owner’s PWM

- **Location:** `grevir-avr/src/grevir/avr/devices/atmega328p/pwm_program.hpp:8–35,260–291`; `grevir-core/src/grevir/core/allocated_application.hpp:48–54,80–82`.
- **Trigger and reproduction:** A module with empty `Requests` receives `OwnerView<Allocation, setl::TypeArgs<>>`. C++ partial specialization can recover that public `Allocation` template argument from its `Plan` type. The module can then name `Allocation::Pwm<"motor","left">` and call `write()` or `writeTicks()`. Those public methods perform no caller-owner check and carry an empty `Claims` type. The new `OwnerView::Pwm` assertion is never instantiated on this path.
- **Scope / classification / provenance:** Shared ownership contract and AVR binding; correctness defect inherited from the public raw binding, left open by this round’s scoped-view repair.
- **Impact:** An unrelated module can update the motor owner’s selected timer endpoint without declaring that use or receiving an owner service. Static resource claims do not detect the write.
- **Correction:** Make raw selected bindings inaccessible through the allocation type exposed to modules. Construct owner-bound capabilities whose writable operations can be obtained only from the matching declared use; cross-module updates should pass through the owner’s service interface.
- **Closure test:** Compile-negative probe: a module with empty requests attempts to recover the allocation from its view and call `Pwm<"motor","left">::write()`. It must fail while the motor owner’s `Plan::Pwm<"left">::write()` compiles.

## 2 Invariant analysis

| Invariant | Result |
| --- | --- |
| One selected whole timer per owner | The common solver groups demands by owner key and rejects two selected candidates on the same timer. |
| Complete use coverage | `fits()` requires each demand to match one binding; unsupported AVR event use reaches a compile-time rejection. |
| Deterministic selection | Demand and candidate sorting, together with stable AVR keys, remove the prior enumeration-order dependency on the inspected path. |
| Exact timer and counter width | AVR candidate filtering intersects the requirements; `WidthGate` exposes the specified contradiction. |
| Canonical board/device pin claims | Arduino AVR mapping closes the prior Uno/Nano alias path when its adapter is included. |
| Reservations cover selected footprint | **Fails:** common reservation checking omits explicit binding endpoints and pins (P0-1). |
| Owner-only update authority | **Fails:** the public raw AVR binding remains recoverable from the owner view’s type (P1-1). |
| AVR integer and target boundary | The changed fixed-PWM update path uses bounded integer arithmetic; I found no new AVR register macro or implicit floating-point path in it. |

## 3 Risks and next action

The common footprint defect is a new architectural root cause in this remediation round. Repair it in the allocator contract, then verify reservation and cross-candidate conflicts from the same canonical resource inventory. Close the foreign-binding path with a negative compile probe that exercises the public module view, including type recovery. Re-review those corrections before treating this checkpoint as a sound ownership boundary.
