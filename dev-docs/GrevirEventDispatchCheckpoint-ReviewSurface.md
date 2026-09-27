# Grevir event dispatch checkpoint — SURFACE-AXIS REVIEW

**Review object:** First MainLoop/Elide event dispatch checkpoint.  
**Baseline:** Exact tuple below, verified at review start and end.  
**Date:** 2026-09-28  
**Axis:** SURFACE — independent, read-only first-day API walkthrough.  
**Verdict:** GO under the specified severity gate, with one open P3 documentation defect. The public walkthrough does not yet complete the overflow-inspection step.

| Repository | Verified HEAD |
| --- | --- |
| Root | `f1acfc67d87efd632acc281e779c80404b580f09` |
| grevir-core | `95b9c438bef5f07946bb51a066a3f77c7388758b` |
| grevir-test-support | `c6a618d549ac61e6c9fc51f8e15e5a718914a07c` |
| grevir-peripherals | `4612cecdc99c22f326a0c1498846fea34c0c596c` |
| grevir-avr | `032f907fe430408c2d214431b68162cff1c85977` |

## 0 Evidence base

Read only the permitted public surfaces:

- `docs/guides/interrupts.md`
- `docs/examples/interrupt-mock/mock_app.hpp`
- `docs/examples/interrupt-mock/main.cpp`
- `docs/examples/interrupt-avr/avr_app.hpp`
- `docs/examples/interrupt-avr/avr_app_base.hpp`
- `docs/examples/interrupt-avr/interrupt-avr.ino`
- `docs/index.md`
- `docs/supported.md`
- Linked public API pages: `docs/api/index.md`, `core.md`, `peripherals.md`, `avr.md`, `arduino.md`, `arduino-avr.md`, and `arduino-esp32.md`.

The controlling public-contract statement is `docs/index.md:3–7`. No implementation, design documents, plans, or other reviewers’ reports were consulted. No builds, edits, or Git mutations were performed.

The five HEAD values matched the supplied tuple at both boundaries. All five `git status --porcelain` checks were empty at both boundaries.

## 1 Findings

### SURFACE-01 — P3: The promised sticky overrun flag has no documented inspection operation

**Exact location:** `docs/guides/interrupts.md:54–62`, especially the promise at line 57. Neither the mock walkthrough at `docs/examples/interrupt-mock/main.cpp:11–23` nor the AVR loop at `docs/examples/interrupt-avr/interrupt-avr.ino:24–26` supplies the missing operation.

**Root cause:** The public contract describes the existence and purpose of overflow state but omits the API needed to observe it.

**Walkthrough trigger:** A first-day user follows the deferred-event example, adds bounded dispatch to `loop()`, then tries to implement the documented overflow diagnostic. The guide supplies exact posting operations and their result labels, but no callable name, result type, or example for inspecting the sticky flag. The linked Core API page also supplies no event-state reference (`docs/api/core.md:1–22`).

**Impact:** That user must guess or leave the public documentation to finish overflow reporting. The explicit-post `full` result does not document how application code observes overflow from generated hardware producers. This is a bounded documentation defect; this review does not establish that the implementation lacks an inspection API or that the supplied single-event examples overflow.

**Remedy:** Add an exact, compilable overflow-inspection snippet beside the dispatch example. State whether inspection clears the flag, how acknowledgment or clearing works if supported, and how startup or startup failure affects that state. Link the owning API reference.

**Closure check:** A reader restricted to the public guide and its linked API documentation can write a loop that dispatches events, detects recorded overflow, and handles that state according to documented semantics without opening library implementation headers.

## 2 Invariant analysis

### Event declaration and handler discovery

The guide identifies an event through `EventKey`, an event type, and a request’s `InterruptEvents` alias (`docs/guides/interrupts.md:9–18`). The complete mock example supplies the missing composition context: `RequestedModule` at `docs/examples/interrupt-mock/mock_app.hpp:14–19` and `ApplicationSpec` at line 89.

Handler visibility during both probe and final compilation is explicit. The documentation also distinguishes declaring an event from activating hardware demand (`docs/guides/interrupts.md:31–37`). The attempted confusion between “catalogued” and “automatically enabled” is therefore addressed.

### Default routing and execution context

The default route is explicitly MainLoop/Elide, with an exact bounded-dispatch call (`docs/guides/interrupts.md:39–52,79–82`). The direct-route specialization spells out both `IsrLevel` and `Direct` (`docs/guides/interrupts.md:64–77`). The prohibition on defining both handler forms for one event is explicit at line 79.

The mock example demonstrates two producer firings before dispatch, no immediate callback, and one dispatched callback (`docs/examples/interrupt-mock/main.cpp:11–20`). This is representative evidence for the advertised elision behavior at the user-facing example level.

The guide states that callbacks run outside the queue lock and may post again (`docs/guides/interrupts.md:54–61`). It does not promise Stream semantics or preservation of every firing.

### Build and startup walkthrough

The guide supplies staged AVR build arguments, including the application header, board identities, compiler identity, and libraries (`docs/guides/interrupts.md:114–129`). It explains that the compiler identity is not an executable path and that direct Arduino compilation does not run generation (`docs/guides/interrupts.md:146–152`).

The native walkthrough provides configure, build, and test commands (`docs/guides/interrupts.md:154–170`). These commands were inspected, not executed.

The actual startup spelling is available in both examples. The mock checks initial success and demonstrates a repeated successful call reported as `replayed` (`docs/examples/interrupt-mock/main.cpp:6–21`). The AVR sketch checks startup failure before dispatching (`docs/examples/interrupt-avr/interrupt-avr.ino:16–26`). Startup and dispatch are distinct operations in the surface; the examples do not suggest that `start()` automatically pumps deferred callbacks.

### Manual board wiring is visible

The complete examples expose substantial board integration rather than concealing it: allocation candidates, selected plans, validation, configuration, installation, and enablement appear in `docs/examples/interrupt-mock/mock_app.hpp:24–85` and `docs/examples/interrupt-avr/avr_app_base.hpp:23–109`.

These are deliberately small examples: one requested event, explicit use of `EventCatalog<Spec>::keys[0]`, and empty module-setup hooks. They do not demonstrate general multi-module integration. However, the guide assigns inventory declarations to the board (`docs/guides/interrupts.md:89–92`), and the complete sources expose the wiring. No unsupported claim that arbitrary board integration is automatic was established.

### Backend and evidence boundaries

Mock and ATmega328P deferred support are explicitly distinguished from classic ESP32 direct delivery (`docs/guides/interrupts.md:79–87`). Stream, extra dispatch contexts, and software-only event catalogues are expressly deferred. The guide also rejects an assumption that multiple logical events sharing one physical source are supported (`docs/guides/interrupts.md:98–99`).

The support table separates native execution, AVR compilation, simavr behavior, and physical hardware (`docs/supported.md:7–21`). Classic ESP32 interrupt routing and physical behavior are not claimed as validated. These boundaries withstand the first-day-reader attack; this review does not treat deferred ESP32, software-only discovery, named contexts, deadline timers, Stream, or silicon evidence as required checkpoint deliverables.

## 3 Risks and next action

Complete the overflow-inspection documentation and demonstrate its use in a public example. This is the one identified gap in the requested first-day sequence.

The GO verdict is limited to the specified severity gate and inspected public surface: no P0, P1, or P2 defect was established. It is not implementation verification, a successful build result, or validation of concurrency and hardware behavior.
