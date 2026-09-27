# Grevir Event Dispatch Follow-up — Surface Review

**Date:** 2026-09-28  
**Axis:** Public documentation and examples  
**Verdict:** **GO** for the documentation surface. No actionable P0–P2 finding.  
**Method:** Independent, peer-blind, read-only review. No files were modified; no builds or tests were run.

**Pinned tuple, verified at start and end with clean working trees:**

| Repository | HEAD |
| --- | --- |
| Workspace root | `2f25854834f83a2f9fbb0372a352d5bb5f1a1be0` |
| grevir-core | `e37470b5e6ffa628e0788727c56704c394a46a07` |
| grevir-test-support | `d8fdd2e86c07ff62d08e282fc7fdb9990cdf9fec` |
| grevir-peripherals | `4612cecdc99c22f326a0c1498846fea34c0c596c` |
| grevir-avr | `5f4feffa15110aac9bf27f0bc17c2ce668b003d6` |

## 0. Evidence base

I read only `docs/` pages and `docs/examples/` sources, plus `AGENTS.md`, `AGENTS_GWZ.md`, and the CrossMcu, AVR, and ESP32 review policies. I inspected the `docs/` diff from root `3ea2c2d31ab50b099247687274c0571e300352ca` to the pinned root revision. That diff changes [the interrupt guide](/Users/owebeeone/limbo/grevir-wz/docs/guides/interrupts.md), [the ESP32 API page](/Users/owebeeone/limbo/grevir-wz/docs/api/arduino-esp32.md), and [the Uno sketch](/Users/owebeeone/limbo/grevir-wz/docs/examples/interrupt-avr/interrupt-avr.ino). This review does not independently establish implementation behavior or repeat the documented build evidence.

## 2. First-day walkthrough and invariant analysis

- The reader can follow declaration, handler selection, build, startup, and dispatch through the guide and complete mock, Uno, and ESP32 examples. Each example supplies an application header and board inventory; the guide gives staged Arduino commands and the mock CMake command.
- The support boundary is consistent: [the guide](/Users/owebeeone/limbo/grevir-wz/docs/guides/interrupts.md:103) limits deferred delivery to mock and ATmega328P and identifies classic ESP32 as direct; [the ESP32 API page](/Users/owebeeone/limbo/grevir-wz/docs/api/arduino-esp32.md:25) makes the same distinction. [Support evidence](/Users/owebeeone/limbo/grevir-wz/docs/supported.md:15) limits ESP32 to compile/link and leaves interrupt routing and physical behavior unvalidated.
- The [posting example](/Users/owebeeone/limbo/grevir-wz/docs/guides/interrupts.md:76) uses qualified `grevir::event::PostResult::full` and `::not_ready` outcomes. Its `PeriodElapsed` and `GrevirApplication` names are introduced earlier in the same guide.
- The [Uno sketch](/Users/owebeeone/limbo/grevir-wz/docs/examples/interrupt-avr/interrupt-avr.ino:25) dispatches in `loop()`, records an observed overrun in a persistent diagnostic byte, and clears the queue’s sticky flag.
- The [capacity text](/Users/owebeeone/limbo/grevir-wz/docs/guides/interrupts.md:67) presents 1–2048 as permitted **selected** capacities and explicitly says it is not a preallocated maximum.
- The examples distinguish `on_event` main-loop delivery from `on_interrupt` direct delivery. They do not claim Stream, extra contexts, general allocators, or silicon validation.

## 3. Risks and next action

This is a documentation-surface verdict. The documented build and runtime claims still depend on the separate code and state reviews and their existing validation evidence. No surface correction is required from this review.
