# GrevirTimerOwnershipCheckpoint — STATE-AXIS REVIEW

**Decision: NO-GO.** Two open lifecycle defects prevent this checkpoint from establishing the stated owner setup boundary.

**Reviewed tuple, verified at start and end:** root `09b954e2b9fe11f1df93ef278533e39afb81ca00`; grevir-core `1e4dae202da99e2064b92d7c68fb2ed981155598`; grevir-peripherals `74eb361b705b0e043841d91ebcf8475391fbcf69`; grevir-avr `bb40a478500968044887abd63c8131355154f8b0`.

**Evidence base:** Read-only inspection of each member’s `HEAD^..HEAD` implementation diff, the checkpoint and controlling design documents, `CrossMcu.md`, `Avr.md`, and `Esp32.md`. I traced Core lifecycle dispatch, AVR candidate selection and register setup, resource claims, and the changed host tests. No build, tree mutation, or silicon test was performed.

## Findings

### P1-1 — Timer register writes precede dependency setup

- **Location:** `grevir-core/src/grevir/core/allocated_application.hpp:56-58`; `grevir-core/src/grevir/core/application.hpp:26-29`; effects begin at `grevir-avr/src/grevir/avr/devices/atmega328p/pwm_program.hpp:124`.
- **Trigger:** Give a timer-owning module a dependency whose `runSetup()` enables its clock or establishes another required hardware state. The application calls **all** modules’ `paramsSetup()` before **any** module’s `runSetup()`. The selected timer is programmed in the owner’s parameter phase, so its dependency has not run.
- **Scope:** Shared lifecycle contract; affects an AVR owner with a runtime setup dependency. The same ordering issue applies to future backends.
- **Classification/provenance:** Correctness defect introduced by moving selected timer setup into `SelectedTimerParameter`; source-proven from the two lifecycle passes.
- **Violated invariant/consequence:** The design requires selected owner effects after dependencies are ready. A dependency-first module order cannot satisfy that invariant across these separate passes. Timer writes can occur against an unprepared peripheral, and dependent setup can subsequently change the state assumed by the selected payload.
- **Remedy:** Dispatch selected owner setup in a dependency-aware setup stage, after prerequisite modules complete their setup and before the owner’s consumers run. Define where owner parameter initialization belongs in that order.
- **Regression test:** Add a dependency that sets a mock `ready` flag in `runSetup()` and have register access record whether every timer write observes `ready == true`. Test both descriptor orders.

### P2-1 — Binding suppresses a module’s custom parameter lifecycle hooks

- **Location:** `grevir-core/src/grevir/core/allocated_application.hpp:57-58`; dispatch at `grevir-core/src/grevir/core/module.hpp:23-30`.
- **Trigger:** `Module<Allocation>` defines its own `paramsSetup()` or `paramsLoop()`, as the existing Core lifecycle permits. `RequestedModule::Bind` defines methods with the same names and invokes only `Params::ParamsRunner`; the implementation’s hooks are never called.
- **Scope:** Shared Core behavior, including AVR applications.
- **Classification/provenance:** Correctness defect introduced by this binding change; source-proven.
- **Violated invariant/consequence:** Binding an allocated module must preserve its lifecycle behavior while adding selected owner setup. Custom preparation, loop work, or cleanup coordination placed in those hooks silently disappears.
- **Remedy:** Compose the selected owner action with the implementation’s lifecycle hooks without invoking its original parameters twice. Preserve the implementation’s intended ordering explicitly.
- **Regression test:** Bind a module whose custom `paramsSetup()` and `paramsLoop()` increment counters; verify both execute once and selected timer setup executes once.

## Invariant analysis and limits

The selected candidate key resolves to one `Choice::hardware` payload, and `Selected::setup()` uses that payload for TOP, waveform, clock select, and selected output initialization. The AVR claims include the whole `HardwareTimer` and selected pins; candidate compatibility rejects two owners on one timer. Duplicate owner descriptors with separate local uses encounter duplicate physical claims at final application validation. The changed mock checks one Timer1 TOP write for the one-owner case, but its two-owner test checks final register values rather than one-write-per-owner counts.

No fallible setup operation exists in this fixed-PWM slice. Duty updates validate their inputs before writes. Thus partial setup failure and cleanup cannot be demonstrated from this code; their future contract remains open. Repeated `App::runSetup()` reprograms timers, but this checkpoint does not define repeated startup semantics, so I record it as an **unverified concern**, not a defect. ISR coexistence, interrupt masking, and silicon behavior are deferred; the source comment requires callers to leave timer interrupts disabled. A source search of the changed public and backend headers found no direct `TCCR`/`TIMSK`, `_BV`, or `ISR(...)` macro use.

The review remains **NO-GO while P1-1 and P2-1 are open**.
