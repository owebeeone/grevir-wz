# Grevir deferred context identity limit — Surface review

**Object:** Root diff `e903337..a2e0badd2aa4e3d7592ee50d27fc6913602fe547`, especially `docs/guides/interrupts.md`  
**Baseline:** Root `e903337`  
**Date:** 2026-09-28  
**Axis:** Public guide and user-visible diagnostic contract; read-only  
**Verdict: GO.** No open P0–P2 finding.

## 0. Evidence base

I read `AGENTS.md`, `AGENTS_GWZ.md`, the CrossMcu and Esp32 review policies, the public interrupt guide and ESP32 example declarations, and [Surface Review 3](/Users/owebeeone/limbo/grevir-wz/dev-docs/GrevirEsp32DeferredDispatchCheckpoint-ReviewSurface-3.md). I assessed the public user path before reading the controlling [draft contract](/Users/owebeeone/limbo/grevir-wz/dev-docs/GrevirDeferredContextIdentityLimit.md:6). I also inspected the root diff and public entry, API, and support pages. I did not inspect implementation code, write files, or run builds.

The requested commit tuple matched at the start and end:

| Repository | Commit |
| --- | --- |
| Root | `a2e0badd2aa4e3d7592ee50d27fc6913602fe547` |
| `grevir-core` | `f389c902d9a73e9a0579dce0dc08e34cf5af2f7b` |
| `grevir-arduino-esp32` | `6b5a1776d2c2eba1fa0e00729c5f9e3ac5a79e5f` |
| `grevir-avr` | `322e8b9fc520d7ba60b46104b14f86e2eaf79c9c` |
| `grevir-test-support` | `b13fea29e82d813f2ae2f6719b0adb2fb6a8cb45` |

The sole root status entry was the explicitly excluded untracked `grevir-source-docs.aiar.sh.txt`; I left it untouched.

## 1. Findings

No actionable P0–P3 surface finding. The public guide states that the **complete generated identity for both policies** must fit in 255 bytes and that a longer identity fails compilation with `GREVIR_EVENT_POLICY_ID_TOO_LONG` ([interrupts.md](/Users/owebeeone/limbo/grevir-wz/docs/guides/interrupts.md:81)). This agrees with the draft’s complete encoded identity limit, rather than promising 255 bytes to each input separately.

## 2. Invariant analysis

A board author can reach the [interrupt guide](/Users/owebeeone/limbo/grevir-wz/docs/index.md:17), see the required `EventLock` and `MainLoopContext<Application>` policies, and compare them with the [classic ESP32 board declaration](/Users/owebeeone/limbo/grevir-wz/docs/examples/interrupt-esp32/esp_app_base.hpp:24). The adjacent guide text identifies the combined generated identity as the bounded value and provides the exact diagnostic to recognize if it is too long. The documented [staged ESP32 build](/Users/owebeeone/limbo/grevir-wz/docs/guides/interrupts.md:195) is the path on which compilation reports it. The guide does not promise that either individual policy identity may use all 255 bytes.

The draft says rejection occurs during C++ template instantiation before probe serialization ([contract](/Users/owebeeone/limbo/grevir-wz/dev-docs/GrevirDeferredContextIdentityLimit.md:6)). The public statement “fail compilation” is consistent with that behavior. The [checkpoint](/Users/owebeeone/limbo/grevir-wz/dev-docs/GrevirEsp32DeferredDispatchCheckpoint.md) records boundary and diagnostic evidence, but this read-only surface review did not independently rerun it.

## 3. Risks and next action

This verdict covers the documented user path for the classic ESP32 Dev Module with Arduino-ESP32 3.3.11. Physical hardware, S2/S3, software-only events, and named contexts remain deferred. Proceed on the surface axis; implementation and state closure require their separate reviews.
