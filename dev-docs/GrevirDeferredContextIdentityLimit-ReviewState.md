# Grevir deferred context identity limit — State review

**Review object:** Root `e903337..a2e0bad`, Core `a711cb7..f389c90`, against the controlling draft `dev-docs/GrevirDeferredContextIdentityLimit.md` at root HEAD.  
**Baseline:** Root `a2e0badd2aa4e3d7592ee50d27fc6913602fe547`; Core `f389c902d9a73e9a0579dce0dc08e34cf5af2f7b`; Arduino ESP32 `6b5a1776d2c2eba1fa0e00729c5f9e3ac5a79e5f`; AVR `322e8b9fc520d7ba60b46104b14f86e2eaf79c9c`; Test Support `b13fea29e82d813f2ae2f6719b0adb2fb6a8cb45`. The tuple matched at the start and end. Root status showed only the out-of-scope untracked `grevir-source-docs.aiar.sh.txt`; member statuses were clean.  
**Date and axis:** 2026-09-28; State — generated-plan freshness, failure direction, and queue/runtime effects.  
**Verdict:** **GO** for the operator-approved 255-byte contract on the documented classic ESP32 Dev Module path. No open P0–P2 state defect was found.

## 0. Evidence base

I read `AGENTS.md`, `AGENTS_GWZ.md`, the CrossMcu and Esp32 policies, the declarative Dos/DontDos, prior State Review 3, Code Review 3, ReviewOutcome, the controlling draft, the specified diffs, and the relevant probe, generator, strict-output, startup, queue, and ESP32 context paths. This was read-only source inspection; I ran no builds and did not touch the archive. The reported 255-byte mock probe/generation/strict link/dispatch, 256/257-byte Clang and MSVC diagnostics, 192 native tests, Uno and ESP32 compile/link, distinct collision plans, and stale-binding rejection are supplied evidence.

## 1. Findings

**P0–P3: none.**

## 2. Invariant analysis

The accepted boundary is coherent. [ContextPolicyIdentity](/Users/owebeeone/limbo/grevir-wz/grevir-core/src/grevir/event/context_policy.hpp:36) computes the full length-prefixed identity and rejects lengths above 255 at template instantiation. [DeferredContextPlan](/Users/owebeeone/limbo/grevir-wz/grevir-core/src/grevir/interrupt/binding.hpp:189) instantiates it for a selected deferred route and validates its components as ASCII letters, digits, or underscores. Thus the length counts bytes as the probe and Python decoder do. A 255-byte value fits [the probe writer’s one-byte text length](/Users/owebeeone/limbo/grevir-wz/grevir-core/src/grevir/interrupt/probe_record.hpp:74); 256 and 257 bytes fail before serialization. Direct-only plans retain an empty, unselected deferred policy.

The probe decoder places that policy in the [canonical plan](/Users/owebeeone/limbo/grevir-wz/grevir-core/tools/grevir_irqgen/protocol.py:193), whose fingerprint includes it. The emitter copies it into generated C++ and [compares it with the live policy](/Users/owebeeone/limbo/grevir-wz/grevir-core/tools/grevir_irqgen/emit.py:111). The length-prefixed representation still distinguishes the original collision pair. An over-limit edit makes probe compilation fail, so it cannot generate a ready artifact or enter startup. The CMake build depends on the probe and verified generated output; the staged Arduino build uses a fresh work directory and clears its firmware-ready marker before starting.

The changed Core code only narrows a compile-time identity limit. [Queue preparation and dispatch](/Users/owebeeone/limbo/grevir-wz/grevir-core/src/grevir/event/queue.hpp:37), [startup failure handling](/Users/owebeeone/limbo/grevir-wz/grevir-core/src/grevir/interrupt/start.hpp:26), and the ESP32 per-application task owner are unchanged. The prior State Review 3 conclusions about active reprepare rejection, owner admission, stopped-queue transfer, and cleanup therefore remain applicable. The draft and public guide describe the new limit consistently with the implementation.

## 3. Risks and next action

The 255-byte end-to-end result is supplied mock evidence; the selected Uno and ESP32 builds establish integration for their existing shorter identities. Source inspection covers the shared C++/probe boundary for accepted policy values, while physical interrupt delivery, cross-core timing, task-handle lifetime beyond the documented `setup()` → `loop()` path, S2/S3, software events, and named contexts remain outside this checkpoint. Proceed to the independent review outcome; this State axis raises no acceptance blocker.
