# GrevirTimerOwnershipCheckpoint — CODE-AXIS REVIEW

**Decision: NO-GO.** The fixed-PWM path has a pin-alias collision that permits false composition, exposes writable timer bindings outside the owning module, and makes the new `timer::Own` boundary depend on PWM-only request and candidate shapes.

## Reviewed tuple and evidence

| Repository | HEAD verified at start and end |
| --- | --- |
| Workspace root | `09b954e2b9fe11f1df93ef278533e39afb81ca00` |
| `grevir-core` | `1e4dae202da99e2064b92d7c68fb2ed981155598` |
| `grevir-peripherals` | `74eb361b705b0e043841d91ebcf8475391fbcf69` |
| `grevir-avr` | `bb40a478500968044887abd63c8131355154f8b0` |

I inspected each repository’s `HEAD^..HEAD` change, the checkpoint and controlling design, the integration remediation and declarative guidance, and the CrossMcu, Avr, and Esp32 policies. This was a read-only source and call-graph review. I made no edits and ran no builds or hardware validation. The supported implementation target assessed here is ATmega328P fixed-frequency PWM; ESP32 timer implementation, non-PWM modes, and silicon validation are deferred.

## Findings

### P0-1 — Arduino pin aliases evade the PWM pin claim

- **Location:** `grevir-avr/src/grevir/avr/devices/atmega328p/pwm_candidates.hpp:11`, `grevir-avr/src/grevir/avr/devices/atmega328p/pwm_backend.hpp:17`; existing alias facts at `grevir-arduino-avr/src/grevir/arduino_avr/boards/uno.hpp:26` and claim at `grevir-peripherals/src/grevir/peripherals/gpio/output.hpp:43`.
- **Trigger:** On an Uno, combine a Timer1 PWM request for PB1 with an existing module using `ardo::arduino::OutputPin<9>`. The board maps digital 9 to PB1. PWM derives ID `rrPORTB::addr * 8 + 1` (41 from the extracted `0x05` address), while the GPIO module claims `GPIOResource<9>`.
- **Scope / classification / provenance:** AVR application composition; correctness defect. The numeric-claim alias gap was inherited from the earlier PWM integration and remains in this checkpoint’s new derived-ID path.
- **Violated invariant and consequence:** Board and device names for one pad must normalize before allocation and final conflict checking. `ReservationsFromClaims` checks only `GPIOResource<41>` and Core compares the distinct types `<41>` and `<9>`, so both modules can pass allocation and configure the same physical pin. This is an active false-composition path.
- **Remedy:** Resolve board GPIO claims and PWM routes to a common device-scoped physical identity before reservations and final claim checks, using the maintained board mapping.
- **Regression test:** Compose a PB1 PWM owner with `ardo::arduino::OutputPin<9>` and require a pre-effect conflict; also check a different board pin remains legal. Test both declaration orders.

### P1-1 — Any module can obtain a writable binding to another module’s timer

- **Location:** `grevir-avr/src/grevir/avr/devices/atmega328p/pwm_program.hpp:157–159,184–188`; `grevir-core/src/grevir/core/allocated_application.hpp:51–57`.
- **Trigger:** A `RequestedModule` with an empty request list can instantiate `Allocation::Pwm<"motor","left">` in its own parameter or callback and call `write()` or `writeTicks()`. The template looks up the binding by public string key and declares an empty `Claims`; it does not check the requesting module’s owner identity.
- **Scope / classification / provenance:** Shared module contract and AVR binding; API/ownership correctness defect. The public `Pwm` accessor and empty claim are inherited, but they now bypass the checkpoint’s explicit whole-timer module ownership.
- **Violated invariant and consequence:** Internal timer uses belong to the owning module; another module may depend on its service but may not directly update its timer endpoint. The application passes the full allocation type to every module, so an unrelated module can change the motor’s duty without owning or claiming the timer.
- **Remedy:** Bind an owner-scoped view to each module and expose writable endpoint operations only for uses declared under that module instance. Cross-module behavior should go through an interface supplied by the owner.
- **Regression test:** A module with no `"motor"` request attempting `Plan::Pwm<"motor","left">::write()` must fail at compile time; the motor owner’s identical update must compile.

### P1-2 — The new timer-owner boundary is structurally PWM-only

- **Location:** `grevir-peripherals/src/grevir/peripherals/timer/own.hpp:11`; `grevir-peripherals/src/grevir/peripherals/pwm/requirements.hpp:80–98,215–220`; `grevir-peripherals/src/grevir/peripherals/pwm/allocator.hpp:34–44,67–79`; `grevir-peripherals/src/grevir/peripherals/pwm/validation.hpp:48–59`.
- **Trigger:** Declare an event-only `timer::Own<EventUse<...>>`, or add a period event to an owner’s PWM use. The former fails the `IsPwmRequest` static assertion. The latter is treated as an option, produces no use binding, and reaches `Apply` as unsupported. The common request requires frequency, duty step, and pin; every candidate endpoint must be a channel with a pin.
- **Scope / classification / provenance:** Shared API and allocation architecture; API/design blocker. The PWM model is inherited; making `timer::Own` an aliasing wrapper around it introduces that shape as the new owner boundary.
- **Violated invariant and consequence:** `Own` should collect all uses of one timer into a complete, jointly legal candidate. A backend-specific event candidate builder alone cannot extend this contract: the common request, candidate, validation, and solver must first be redesigned. This obstructs the controlling design’s event-only and PWM-plus-period-event examples even though their backend implementation is deferred.
- **Remedy:** Give the common owner plan use-neutral identities, guarantees, bindings, and footprints. Let PWM and event calculators contribute typed use records and one complete backend payload; validate each use against its own semantics.
- **Regression test:** A synthetic event-only owner without a pin and a joint PWM-plus-period-event owner must each select a complete legal mock candidate; conflicting event/register roles must reject that candidate.

### P2-1 — Candidate keys depend on capability enumeration order

- **Location:** `grevir-avr/src/grevir/avr/devices/atmega328p/pwm_candidates.hpp:79–89,116–118`; plan stores those keys at `grevir-peripherals/src/grevir/peripherals/pwm/allocator.hpp:105–108`.
- **Trigger:** Reorder otherwise identical mode or divider entries in the backend traits. `c.key` is assigned from `choices.size() + 1`, and `c.configuration` initially inherits that key. The same physical timer, WGM, divider, and TOP can receive different keys; `Plan::candidates` consequently changes.
- **Scope / classification / provenance:** AVR backend and shared canonical plan; correctness/parity defect. Sequential keys are inherited from the prior generator.
- **Violated invariant and consequence:** Equivalent capability inventories must yield the same canonical plan independently of enumeration order. The plan’s advertised stable binding identity changes after a metadata reorder, making plan comparison and generated-consumer agreement unreliable.
- **Remedy:** Derive stable candidate and configuration identities from normalized physical timer and setting tuples, independent of traversal position; sort and deduplicate on those identities.
- **Regression test:** Supply two semantically identical mock mode/divider inventories in opposite orders and compare complete plans, including candidate and configuration IDs.

### P2-2 — Exact-timer/width contradictions lose the required diagnostic detail

- **Location:** `grevir-peripherals/src/grevir/peripherals/pwm/allocator.hpp:82–94`; `grevir-peripherals/src/grevir/peripherals/pwm/model.hpp:48–61,95–98`; `grevir-core/src/grevir/core/allocated_application.hpp:77–78`.
- **Trigger:** Require Timer0 and at least 16 counter bits on ATmega328P, as in the new negative probe. Candidate filtering removes Timer0 and returns `Status::no_candidate` with a request key but no timer, width, target, or conflicting-requirement detail. Application instantiation emits the generic `GREVIR_APPLICATION_ALLOCATION_FAILED` assertion.
- **Scope / classification / provenance:** Shared diagnostics with AVR facts; concrete diagnosability defect. The width and exact-timer intersection is new; the generic failure channel is inherited.
- **Violated invariant and consequence:** The controlling design specifically requires the module key, target, timer, and conflicting width in this compile-time contradiction. The current failure is indistinguishable at the enforced gate from an ordinary absence of PWM candidates.
- **Remedy:** Detect mandatory physical-timer/capability contradictions before candidate search and carry structured constraint identities into a diagnostic that the compile gate exposes.
- **Regression test:** Assert the negative Timer0/16-bit case’s diagnostic category and detail fields, separately from a valid but unsupported frequency on Timer0.

## Invariant assessment

| Invariant | Code-axis result |
| --- | --- |
| Exact timer and counter-width intersection | Candidate filters enforce the intersection; contradiction diagnostics are insufficient (P2-2). |
| Two PWM uses within one owner | Requests with the same instance key form one solver unit and one candidate. |
| Different owners cannot share a timer | Candidate compatibility rejects overlapping timer resources; writable binding access still escapes the owner (P1-1). |
| Deterministic declaration reorder | Requests are sorted by key; inventory reorder changes plan keys (P2-1). |
| Canonical pin identity and claims | Typed port/bit facts supply PWM IDs, but Uno numeric aliases remain separate claims (P0-1). |
| Selected claims and setup on a real module | `SelectedTimerParameter` attaches selected claims and setup to `RequestedModule::Bind`. The publicly accessible PWM update handle weakens that ownership boundary. |
| Nonresident options | `For` avoids instantiating inactive option handlers; source inspection found no active nonresident SDK dependency in this slice. |
| Future modes and backends | The current `timer::Own` contract requires PWM-shaped use records (P1-2). Actual ESP32 backend behavior is unimplemented and unassessed. |

## Separate unverified concerns and limits

The public `timer::atmega328p::Timer0/1/2` markers map to handwritten numeric IDs in `TimerIdentity` (`requirements.hpp:174–184`), rather than resolving directly to the authoritative typed timer definitions. I did not establish a current ATmega328P misbinding from that mapping, so this remains an architectural provenance concern, not a separate defect finding.

The `Plan` contains request keys and candidate numbers but not the controlling design’s full choice provenance or rejected-alternative reasons. I did not establish a consumer that currently requires those records, so I have not assigned defect severity. The checkpoint expressly defers ESP32 timer support, non-PWM execution, and silicon behavior; no claim about those implementations follows from this review.
