# Grevir Event Stream checkpoint — Surface review

**Date:** 2026-09-28  
**Axis:** Surface, peer-blind and read-only  
**Verdict:** **NO-GO** — one P2 finding remains open.

| Repository | Reviewed commit |
| --- | --- |
| Root | `2c2ea1e1b3326caf41f38acdb8f79c02b21b4895` |
| grevir-core | `6d09870d78b0628da4c6a0007c4a7d45e4ae5473` |
| grevir-test-support | `d8fdd2e86c07ff62d08e282fc7fdb9990cdf9fec` |
| grevir-peripherals | `4612cecdc99c22f326a0c1498846fea34c0c596c` |
| grevir-avr | `5f4feffa15110aac9bf27f0bc17c2ce668b003d6` |

## 0. Evidence base

I read only `docs/` pages and `docs/examples/` sources, the workspace instructions and the CrossMcu, Avr and Esp32 review policies, plus the requested review-loop skill. I compared the three changed docs pages with root commit `3e09c15bdeb8397a55ff6f88b95ecc1184fa0a6a`. I did not read implementation, checkpoint or design documents, or peer reports. I ran no build or test. The exact tuple above matched at both start and end; all five worktrees were clean at both checks.

The public scope presented by the docs is mock and ATmega328P `MainLoop`/`Elide` and `MainLoop`/`Stream`, with direct delivery on classic ESP32. Physical silicon, ESP32 deferred delivery, event payloads, named contexts, software-only events and deadlines remain outside the stated support.

## 1. Findings

### P2-1 — Shared queue description contradicts Stream delivery

- **Location:** `docs/guides/interrupts.md:81–88`, especially line 83. This text follows the new Stream contract at lines 58–79.
- **Root cause and violated invariant:** The shared queue paragraph retains the Elide-only rule, “Another firing while it is queued coalesces,” without qualifying it by route. The Stream invariant stated immediately above is one record per accepted firing and no `coalesced` result.
- **Trigger and consequence:** A reader selects `MainLoop`/`Stream`, sets queue capacity to one, and receives two firings before dispatch. The second firing is dropped with a full queue and overrun indication under the Stream description, while the later paragraph says it coalesces. That contradiction can lead the reader to size the queue or interpret a missing callback incorrectly. It is a concrete correctness and diagnosability defect in the public contract.
- **Scope/classification/provenance:** Shared user-facing contract affecting mock and ATmega328P Stream users; introduced by adding Stream guidance without updating the existing Elide description. No implementation claim is made.
- **Correction:** State the common queue and lock behavior separately, then make the repeat-firing rule explicit for each delivery mode: Elide coalesces an already pending event; Stream enqueues another record if capacity remains and otherwise drops it, reports `full` to a publisher, and sets overrun.
- **Closure test:** Read the guide alone and trace two firings before dispatch at capacities one and two for both routes. Each trace must yield one unambiguous callback count, post result where applicable, and overrun state.

### P3-1 — Linked runnable examples exercise Elide only

- **Location:** `docs/guides/interrupts.md:148–217` links the buildable examples; `docs/examples/interrupt-mock/mock_app.hpp:91–96`, `docs/examples/interrupt-mock/main.cpp:11–29`, and `docs/examples/interrupt-avr/avr_app.hpp:4–10` contain no Stream route specialization.
- **Root cause and consequence:** Stream is shown as a short route snippet, while every linked deferred runnable example retains the default Elide route. A first-day reader can run the documented CMake and Uno commands, but those builds do not exercise Stream. Even if the reader inserts `RouteFor<PeriodElapsed>` into the mock header, the current mock assertion dispatches only one of two raised events and never checks that the second callback remains queued. Its success therefore does not demonstrate the behavior the reader selected.
- **Scope/classification/provenance:** Public example coverage and usability on the mock and ATmega328P paths; introduced with the new Stream documentation. This does not dispute the separate validation claim in `docs/supported.md`.
- **Correction:** Add a linked, buildable Stream variant or a precise edit-and-build walkthrough, with assertions that two accepted firings produce two callbacks across bounded dispatch calls and that a full queue is distinguishable from an accepted firing.
- **Closure test:** Starting from `docs/index.md`, follow only public links to build a Stream example and observe assertions for repeated accepted firings and a full-queue drop.

## 2. First-day walkthrough and invariant analysis

The entry page directs a new reader to support evidence before selecting a board. The support page distinguishes native mock Stream evidence from ATmega328P target compilation and explicitly says the simavr interrupt example still uses Elide. The ESP32 API page and interrupt guide consistently limit classic ESP32 to direct delivery and target compile/link evidence; neither claims deferred Stream or physical interrupt behavior.

The interrupt guide gives an explicit Stream route specialization and says to place it before the handler. Its “choose either” language, together with “use either `on_interrupt` or `on_event`,” guards against declaring a second handler when read carefully. It states that Stream produces one callback per **accepted** firing, a full queue drops a firing, `post` and `post_from_isr` return `full`, and the hardware entry uses the same queue. It also states that queue records carry no payload and that device bytes or frames belong in an owned buffer. The later unqualified coalescing sentence breaks this otherwise usable account. The linked build commands are concrete, but their example sources select Elide.

## 3. Risks and next action

Correct P2-1 before accepting the public Stream surface. P3-1 can be addressed alongside that correction to make the first Stream build and its expected callback count observable. A focused peer-blind Surface re-review should repeat the two-firing traces from the revised docs and follow the linked Stream build path. This review establishes documentation consistency only; it does not establish implementation behavior or target runtime results.
