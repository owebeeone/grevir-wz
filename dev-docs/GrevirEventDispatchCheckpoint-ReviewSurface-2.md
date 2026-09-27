# Grevir Event Dispatch Checkpoint — Surface Review 2

**Verdict: GO, with P3 documentation corrections.** The public guide and runnable mock and Uno sources provide a path to declare a payload-free deferred event, generate its binding, start the application, dispatch callbacks, and read and clear the sticky overrun flag. I found no demonstrated P2 API surface defect. The gaps below require readers to reconcile conflicting support claims or guess details when handling publication failures.

## Evidence base

Reviewed only user-facing pages under `docs/`, runnable sources under `docs/examples/`, and the required workspace review policies. I did not inspect implementation, design documents, remediation plans, or other reviewer reports. I ran no build, test, generator, simulator, or hardware command.

Commands used: `git rev-parse HEAD`, `git status --porcelain=v1` for the root and four named members at both the start and end; `rg --files -uu docs`; `rg -n -i 'deferred|event|dispatch|overrun|mainloop|elide' docs`; `nl -ba` on the interrupt guide, Core API, support page, module concept page, index, installation page, relevant target API pages, and all three interrupt example source sets. I also read `AGENTS_GWZ.md` and the required CrossMcu, Avr, and Esp32 review policies.

The tuple matched at both checks, and all five working trees were clean:

| Repository | Revision |
| --- | --- |
| Root | `3ea2c2d31ab50b099247687274c0571e300352ca` |
| `grevir-core` | `0944c59b0c7a1ca53656f4a71fd62f1cce8969f5` |
| `grevir-test-support` | `d8fdd2e86c07ff62d08e282fc7fdb9990cdf9fec` |
| `grevir-peripherals` | `4612cecdc99c22f326a0c1498846fea34c0c596c` |
| `grevir-avr` | `5f4feffa15110aac9bf27f0bc17c2ce668b003d6` |

## Findings

### S2-01 — P3: ESP32 interrupt support is described inconsistently

**Location:** `docs/api/arduino-esp32.md:24–26`; `docs/guides/interrupts.md:3–6, 92–94, 140–153`; `docs/examples/interrupt-esp32/esp_app_base.hpp:7, 57–88`.

**Trigger and impact:** A reader checking target support finds the ESP32 API page saying the adapter “does not implement an ESP32 timer, interrupt,” while the interrupt guide supplies a classic ESP32 TG0/T0 direct interrupt example and a staged build command. The reader must guess whether that selected direct path is supported or the API page supersedes it. This is a documentation correctness issue for a named target, not evidence that deferred ESP32 delivery works.

**Remedy:** Qualify the API page: the selected classic ESP32 TG0/T0 direct interrupt example exists and compiles/links; a general timer or peripheral allocator and deferred ESP32 dispatch are outside the current scope. Keep its hardware validation limit explicit.

**Closure test:** Read the API page, interrupt guide, support table, and example together and confirm they make one consistent claim about direct versus deferred ESP32 delivery. Rebuild the named example when target validation is authorized. Provenance relative to earlier revisions was not assessed.

### S2-02 — P3: Programmatic publication outcomes lack copyable API spelling

**Location:** `docs/guides/interrupts.md:58–71`; `docs/api/core.md:24–30`; `docs/examples/interrupt-mock/main.cpp:5–27`.

**Trigger and impact:** A loop publisher needs to react to `full` or `not_ready`. The guide names `post<Application, Event>()`, `post_from_isr<Application, Event>()`, and four outcomes, but gives neither the return type nor qualified outcome names. None of the runnable examples compares a publication result. The reader must inspect a header or guess an enum spelling to handle failure. Scope is the shared deferred-event documentation; this is a bounded documentation gap, not a demonstrated API defect.

**Remedy:** Add a short, compilable mock example that calls `post`, compares its result against the public qualified outcomes, and explains when `not_ready` is returned.

**Closure test:** Compile that example through the documented mock CMake path and verify the guide uses the same names. No such build was run in this review. Provenance relative to earlier revisions was not assessed.

### S2-03 — P3: The complete Uno sketch omits the overrun handling shown in the guide

**Location:** `docs/guides/interrupts.md:43–55, 58–69`; `docs/examples/interrupt-avr/interrupt-avr.ino:16–26`.

**Trigger and impact:** Copying the linked “complete Uno Timer1 example” yields a `loop()` that only dispatches. The guide correctly says a full queue drops a firing and sets a sticky flag, and shows how to inspect and clear it, but that diagnostic is absent from the runnable Uno sketch. A first-day user following the complete example can lose firings without any visible indication. Scope is the Uno example’s robustness; the underlying flag contract is documented.

**Remedy:** Add a minimal overrun observation and clear path to the Uno sketch, or explicitly point from its `loop()` to the guide’s required diagnostic integration.

**Closure test:** Inspect the sketch and guide together, then compile the named Uno example when target validation is authorized. No build or simulator run was performed here. Provenance relative to earlier revisions was not assessed.

## First-day walkthrough and invariants

The entry index links the interrupt guide and complete examples (`docs/index.md:17`). The guide declares an `EventKey`, event type, request, and module relationship (`docs/guides/interrupts.md:9–18`); the mock and Uno application headers show those declarations within an `ApplicationSpec` and place `on_event` after the handler include (`docs/examples/interrupt-mock/mock_app.hpp:14–19, 89–96`; `docs/examples/interrupt-avr/avr_app_base.hpp:14–19, 113–115`; `docs/examples/interrupt-avr/avr_app.hpp:4–9`). The mock CMake source shows binding generation, and the guide gives runnable host and staged Uno commands (`docs/examples/interrupt-mock/CMakeLists.txt:15–22`; `docs/guides/interrupts.md:123–138, 163–179`). The mock main and Uno sketch show `Application<GrevirApplication>::start()` followed by dispatch (`docs/examples/interrupt-mock/main.cpp:5–15`; `docs/examples/interrupt-avr/interrupt-avr.ino:16–26`).

The guide states the default `on_event` route is `MainLoop`/`Elide`; a repeated firing coalesces while queued, a full queue drops a firing and sets a sticky overrun flag, callbacks run outside the queue lock, and nested or competing dispatch returns zero (`docs/guides/interrupts.md:39–71, 88–96`). It distinguishes `on_interrupt` and explicit `IsrLevel`/`Direct` handling from deferred delivery (`docs/guides/interrupts.md:73–91`). The mock example verifies two firings elide to one dispatched callback and checks that overrun stayed clear (`docs/examples/interrupt-mock/main.cpp:11–27`). It does not exercise a full queue or a failed start. The documented startup failure boundary says the queue stops if startup fails (`docs/guides/interrupts.md:65–69`); publication error handling remains the concrete gap in S2-02.

The supported-platform page confines current deferred evidence to generated mock dispatch and simulated ATmega328P Timer1 delivery, with physical silicon held (`docs/supported.md:7–21`). The interrupt guide explicitly defers ESP32 queue synchronization, software-only catalogues, Stream delivery, and additional dispatch contexts (`docs/guides/interrupts.md:88–96`). Those limits are clear aside from the ESP32 API page conflict.

**Next action:** Correct the three documentation gaps, then perform the stated closure checks within the project’s validation holds. This surface review makes no claim about implementation correctness or unrun target builds.
