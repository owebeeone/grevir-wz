# GrevirTimerOwnershipCheckpoint — SURFACE-AXIS REVIEW

**Decision: NO-GO for the published interface walkthrough.** The guide shows one PWM use, but the new whole-timer owner interface is meant to support multiple uses within one module. A reader cannot complete that central workflow from the reviewed public pages.

**Settled tuple:** root `09b954e2b9fe11f1df93ef278533e39afb81ca00`; grevir-core `1e4dae202da99e2064b92d7c68fb2ed981155598`; grevir-peripherals `74eb361b705b0e043841d91ebcf8475391fbcf69`; grevir-avr `bb40a478500968044887abd63c8131355154f8b0`. All four hashes matched at the start and end of this read-only review.

## Finding

**P2 — The two-use owner workflow is absent from the public contract.**  
**Location:** `docs/guides/pwm.md:22–32, 35–40, 47–59`; `docs/examples/pwm-host.cpp:38–59`.  
**Trigger and scope:** A module needs two synchronous PWM outputs on ATmega328P Timer1, using PB1 and PB2, with the timer pinned to Timer1 and a minimum 16-bit counter width. This concerns the shared declaration and allocation contract as presented to an AVR user.  
**Classification:** Public documentation correctness defect, introduced with the `timer::Own` documentation change. The preceding guide documented a single `p::Instance`; the settled guide introduces `Own` but still demonstrates only one `PwmRequest` and one allocated output.  
**Evidence and consequence:** The guide says one module owns the whole timer and its internal uses receive typed bindings, yet its only declaration is `Own<PwmRequest<"pwm", …>, …>` and its only lookup is `Allocation::Pwm<"motor">`. It never explains the roles of `"pwm"` and `"motor"` when there are two uses, how to place a second use in `Own`, or how to obtain each output binding. The “complete” host example repeats the same single-output path. A first-day reader cannot implement or review the checkpoint’s multi-use case without leaving the public documentation.  
**Correction and closure test:** Add a complete two-output example with one `Own`, distinct internal use names, PB1/PB2 requirements, the Timer1 and width constraints, both allocation lookups, and writes to both outputs. Check both compare registers and the shared TOP in the host example; record an AVR compile for that exact declaration if target coverage is claimed.

## Advisory concern

**P3 — Validation evidence for this specific new path is difficult to identify.**  
**Location:** `docs/supported.md:12, 17–20, 31–34`; `docs/examples/pwm-host.md:3–12`.  
**Trigger and scope:** A reader choosing the new `timer::Own` API for an Uno/Nano. This is an evidence and discoverability concern, not an asserted implementation defect.  
**Evidence and consequence:** The support page says selected AVR sketches cover “PWM/pin” compositions, while the linked new-interface example explicitly runs on a native host with memory-backed registers. The pages do not identify whether the selected AVR evidence includes `timer::Own`, much less two uses in one owner. A reader can correctly understand the host example’s limits, but cannot assign a precise AVR evidence level to this new composition.  
**Correction and closure test:** Name the exact owner-based AVR composition covered by target compilation or simulation, or state that this particular composition has host evidence only. Keep physical-board behavior unclaimed.

## Invariants and exclusions

The reviewed pages do convey that a module owns the **whole timer** (`docs/guides/pwm.md:3, 39–40`), so another module cannot independently take Timer1’s other channel under that stated contract. They show the Timer1 pinning and 16-bit width constraints (`:25–31`), application binding (`:47–59`), compile-time failure for unsupported active mandatory requirements (`:35–36`), and Arduino Timer0 reservation (`:71–75`). The ESP32 request section is explicitly inert under the AVR backend, and non-AVR PWM backends are marked future work (`:11–13, 75–76`). The host example accurately disclaims physical AVR and silicon evidence. I found no basis in this surface review for a P0 or P1 finding, or for claiming an implementation failure.

**Next action:** Complete and publish the two-use owner walkthrough, then make the support page identify its exact evidence level. This review read public documentation and review policies only; it did not inspect implementation code, build, or run target validation.
