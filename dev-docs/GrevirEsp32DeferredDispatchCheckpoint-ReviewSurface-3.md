# Grevir ESP32 Deferred Dispatch Checkpoint — Surface Review 3

**Review object:** Public documentation and ESP32 example for deferred dispatch, from root baseline `e242a92` through `17f0f05f7e99bb831d5f052e85a9fbbcbfb2c70a`  
**Date:** 2026-09-28  
**Axis:** User-facing surface, read-only  
**Verdict: GO.** No open P0–P2 finding or new architectural root cause was identified.

| Repository | Pinned commit, verified at start and end |
| --- | --- |
| Workspace root | `17f0f05f7e99bb831d5f052e85a9fbbcbfb2c70a` |
| `grevir-core` | `a711cb78a4276222e135cfd806959aaf2e391bd1` |
| `grevir-arduino-esp32` | `6b5a1776d2c2eba1fa0e00729c5f9e3ac5a79e5f` |
| `grevir-avr` | `322e8b9fc520d7ba60b46104b14f86e2eaf79c9c` |
| `grevir-test-support` | `b13fea29e82d813f2ae2f6719b0adb2fb6a8cb45` |

## Prior-finding closure

| Prior surface finding | Status and evidence |
| --- | --- |
| P3: `ticks` implied a count of every hardware period despite `Elide` coalescing and queue drops. | **Closed.** The handler increments `delivered_callbacks` in [esp_app.hpp](/Users/owebeeone/limbo/grevir-wz/docs/examples/interrupt-esp32/esp_app.hpp:6). The [board example](/Users/owebeeone/limbo/grevir-wz/docs/examples/interrupt-esp32/esp_app_base.hpp:20) and [guide](/Users/owebeeone/limbo/grevir-wz/docs/guides/interrupts.md:147) explain that it counts delivered callbacks. |
| Surface Review 2 | Reported no P0–P3 finding; no additional surface item requires closure. |

The root range `e242a92..17f0f05` changes the interrupt guide, ESP32 API page, support page, and example. There is **no public-docs change after Surface Review 2** (`81d0a1d..17f0f05`). The final remediation changes internal policy identity encoding, described in RemPlan-2, without changing the documented user path. **NEW ARCHITECTURAL root cause: none identified on this axis.**

## 0. Evidence base

I read `AGENTS.md`, `AGENTS_GWZ.md`, `dev-docs/review-policies/CrossMcu.md`, and `dev-docs/review-policies/Esp32.md`; the prior Surface reports and remediation plans; and only public documentation, package README, and example declaration/usage files for the surface review. I inspected Git history and status and verified the tuple twice. The working tree was clean. I did not inspect implementation source, mutate files, or run builds.

The supported target assessed here is the classic ESP32 Dev Module with Arduino-ESP32 3.3.11 and `esp32:esp32:esp32`. Physical silicon, S2/S3, software-only events, and named FreeRTOS contexts remain outside this checkpoint.

## 1. Findings

No actionable P0–P3 surface finding. There is no demonstrated user path with a misleading public promise, missing required step, or hidden failure condition in the reviewed documents.

## 2. Invariant analysis

A user can reach the feature from the [documentation index](/Users/owebeeone/limbo/grevir-wz/docs/index.md:17) or [ESP32 API page](/Users/owebeeone/limbo/grevir-wz/docs/api/arduino-esp32.md:25), then follow the [interrupt guide’s staged build command](/Users/owebeeone/limbo/grevir-wz/docs/guides/interrupts.md:193). The example supplies an event key and request, an `on_event` specialization, an `EventLock`, an ESP32 `MainLoopContext`, and a four-record capacity in [esp_app_base.hpp](/Users/owebeeone/limbo/grevir-wz/docs/examples/interrupt-esp32/esp_app_base.hpp:13).

The [sketch](/Users/owebeeone/limbo/grevir-wz/docs/examples/interrupt-esp32/interrupt-esp32.ino:9) starts the application in `setup()`, reports startup failure, dispatches up to four callbacks per `loop()` only after successful startup, and reports and clears queue overrun. The [guide](/Users/owebeeone/limbo/grevir-wz/docs/guides/interrupts.md:81) distinguishes `Elide` coalescing, `Stream` records, full-queue drops, `not_ready`, sticky overrun, and callback execution outside the queue lock. It states that ESP32 dispatch belongs to the Arduino task running `setup()` and `loop()` and that polling cadence controls latency ([guide](/Users/owebeeone/limbo/grevir-wz/docs/guides/interrupts.md:141)).

The [support page](/Users/owebeeone/limbo/grevir-wz/docs/supported.md:15) limits ESP32 evidence to compilation and linking and explicitly excludes cross-core runtime and physical-board validation. The [package README](/Users/owebeeone/limbo/grevir-wz/grevir-arduino-esp32/README.md:3) makes the same distinction.

## 3. Risks and next action

The described cross-core queue behavior and timer delivery remain unverified on physical hardware. This is an explicit validation limit, not a surface defect. Proceed with the checkpoint under its stated compile/link evidence; retain the silicon and deferred-feature holds.
