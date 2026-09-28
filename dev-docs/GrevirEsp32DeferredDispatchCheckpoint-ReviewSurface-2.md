# GrevirEsp32DeferredDispatchCheckpoint — SURFACE-AXIS REVIEW 2

**Decision: GO.** I found no P0–P3 public-surface finding in the reviewed changes. This was a read-only review of `docs/`, the ESP32 example, and the ESP32 package README. I did not inspect implementation source or run builds.

**Pinned tuple matched at both the start and end of review:**

| Repository | Commit |
| --- | --- |
| `/Users/owebeeone/limbo/grevir-wz` | `81d0a1de7178b56b7d9118a2c1147a4e712f94be` |
| `grevir-core` | `9eb02e8ce61a8a0ada061bd084602dbf67059d5e` |
| `grevir-arduino-esp32` | `6b5a1776d2c2eba1fa0e00729c5f9e3ac5a79e5f` |
| `grevir-avr` | `322e8b9fc520d7ba60b46104b14f86e2eaf79c9c` |
| `grevir-test-support` | `b13fea29e82d813f2ae2f6719b0adb2fb6a8cb45` |

`gwz status` reported `On branch main`.

| Prior Surface finding | Closure |
| --- | --- |
| P3: `ticks` suggested a count of every hardware period, although `Elide` can coalesce firings and a full queue can drop them. | **Closed.** The [handler](/Users/owebeeone/limbo/grevir-wz/docs/examples/interrupt-esp32/esp_app.hpp:6) increments `delivered_callbacks`; the [board example](/Users/owebeeone/limbo/grevir-wz/docs/examples/interrupt-esp32/esp_app_base.hpp:26) and [guide](/Users/owebeeone/limbo/grevir-wz/docs/guides/interrupts.md:147) explicitly distinguish delivered callbacks from hardware periods. |

The root range `e242a92..81d0a1d` changes the ESP32 API page, interrupt guide, support claims, and examples. The ESP32 adapter range `69dff90..6b5a177` changes its README. The AVR and mock example additions supply an explicit main-loop context policy and agree with the guide’s board contract. The public claims consistently identify the classic ESP32 Dev Module, Arduino-ESP32 3.3.11, a bounded `MainLoop`/`Elide` queue, and target compile/link evidence. The [support page](/Users/owebeeone/limbo/grevir-wz/docs/supported.md:15) expressly limits that evidence to compilation and linking; the [README](/Users/owebeeone/limbo/grevir-wz/grevir-arduino-esp32/README.md:1) does not imply physical validation.

A first-day user can declare a catalogued event and `on_event` specialization, provide `EventLock`, `MainLoopContext`, and queue capacity as shown in the [ESP32 board example](/Users/owebeeone/limbo/grevir-wz/docs/examples/interrupt-esp32/esp_app_base.hpp:30), then use the [staged build command](/Users/owebeeone/limbo/grevir-wz/docs/guides/interrupts.md:193). The sketch starts the application in `setup()`, dispatches up to four callbacks per `loop()` call only after successful startup, reports and clears a sticky overrun flag, and prints startup failures ([sketch](/Users/owebeeone/limbo/grevir-wz/docs/examples/interrupt-esp32/interrupt-esp32.ino:9)). The guide explains coalescing, full-queue drops, `not_ready`, task ownership, and loop-cadence latency.

The stated validation remains 192 host CTests, mock generator coverage, and classic ESP32/Uno compile and link; this review did not rerun it. Cross-core runtime behavior and physical silicon remain unverified. ESP32-S2/S3, named contexts, software events, and notifications remain deferred.
