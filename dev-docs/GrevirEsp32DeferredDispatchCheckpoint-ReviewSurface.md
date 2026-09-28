# GrevirEsp32DeferredDispatchCheckpoint — SURFACE-AXIS REVIEW

**Decision: GO for the public surface, subject to the stated validation limits.** I found no P0–P2 surface defect in the reviewed `HEAD^..HEAD` change. This was a read-only review of public docs, the ESP32 example, and the package README; it does not establish implementation or silicon correctness.

## Finding

**P3 — The example’s `ticks` name now implies a count it cannot guarantee.** The [ESP32 handler](/Users/owebeeone/limbo/grevir-wz/docs/examples/interrupt-esp32/esp_app.hpp:6) increments `esp_app::ticks`, while the [board example](/Users/owebeeone/limbo/grevir-wz/docs/examples/interrupt-esp32/esp_app_base.hpp:26) uses the default `MainLoop`/`Elide` route and a four-record queue. As the [guide](/Users/owebeeone/limbo/grevir-wz/docs/guides/interrupts.md:85) explains, repeated firings coalesce and a full queue drops firings. A user treating `ticks` as elapsed 1 ms timer periods can therefore undercount time when `loop()` is delayed. This example-surface issue was introduced by changing the prior direct handler to deferred delivery. Rename it to a delivered-callback count or explicitly explain its meaning beside the example. It does not block this checkpoint.

## Invariants and evidence

- **Declaration and lifecycle:** The example declares `on_event`, provides `EventLock`, `MainLoopContext`, and queue capacity, calls `Application::start()` in `setup()`, and dispatches only after a successful result. A failed start prints the outcome and leaves dispatch disabled.
- **Handler context:** The [guide](/Users/owebeeone/limbo/grevir-wz/docs/guides/interrupts.md:135) says `MainLoop` is bound to the Arduino task running `setup()` and `loop()`; dispatch from another task returns zero. It also explains that polling cadence controls latency.
- **Full queue:** The guide distinguishes `Elide` coalescing from `Stream` records, documents `full`/`not_ready`, and describes the sticky overrun flag. The [sketch](/Users/owebeeone/limbo/grevir-wz/docs/examples/interrupt-esp32/interrupt-esp32.ino:20) drains four callbacks and reports overrun.
- **Validation boundary:** The [support page](/Users/owebeeone/limbo/grevir-wz/docs/supported.md:15) claims a classic ESP32 `MainLoop`/`Elide` compile and link, and expressly excludes cross-core runtime and physical-board validation. The package README is consistent with that boundary. `Stream` is described as supported, but this page reports target compile/link evidence only for `Elide`; that is an evidence limit, not a demonstrated defect.

Physical validation, S2/S3, named contexts, software-only events, and notifications remain explicit deferrals. The pinned root and four member commit IDs matched the supplied tuple at both the start and end of review; `gwz status` reported `On branch main`.
