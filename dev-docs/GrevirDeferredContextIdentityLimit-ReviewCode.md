# GrevirDeferredContextIdentityLimit — Code Review

**Object:** Root `e903337..a2e0bad`; Core `a711cb7..f389c90`  
**Baseline:** [Deferred context identity length contract](/Users/owebeeone/limbo/grevir-wz/dev-docs/GrevirDeferredContextIdentityLimit.md) at root HEAD  
**Date:** 2026-09-28  
**Axis:** Shared C++ architecture and probe/generator boundary; classic ESP32 Dev Module with Arduino-ESP32 3.3.11  
**Verdict: GO.** No open P0–P2 Code finding under the operator-approved 255-byte contract.

## 0 Evidence base

The repository tuple matched at the start and end: root `a2e0badd2aa4e3d7592ee50d27fc6913602fe547`, Core `f389c902d9a73e9a0579dce0dc08e34cf5af2f7b`, ESP32 `6b5a1776d2c2eba1fa0e00729c5f9e3ac5a79e5f`, AVR `322e8b9fc520d7ba60b46104b14f86e2eaf79c9c`, and Test Support `b13fea29e82d813f2ae2f6719b0adb2fb6a8cb45`. The only reported workspace change was the out-of-scope untracked archive `grevir-source-docs.aiar.sh.txt`, which I left untouched.

This was read-only source inspection under [AGENTS.md](/Users/owebeeone/limbo/grevir-wz/AGENTS.md), [AGENTS_GWZ.md](/Users/owebeeone/limbo/grevir-wz/AGENTS_GWZ.md), the [cross-MCU](/Users/owebeeone/limbo/grevir-wz/dev-docs/review-policies/CrossMcu.md) and [ESP32](/Users/owebeeone/limbo/grevir-wz/dev-docs/review-policies/Esp32.md) policies, and the declarative integration Dos/DontDos. I ran no builds. The reported 255-byte probe/generation/strict link/dispatch success, 256- and 257-byte diagnostic failures on Apple Clang and MSVC, collision and stale-binding checks, 192 native tests, and Uno and classic ESP32 compile/link are supplied evidence, not independently rerun results.

## 1 Findings

**None.** The Code Review 3 counterexample is excluded by the newly documented contract and the [compile-time assertion](/Users/owebeeone/limbo/grevir-wz/grevir-core/src/grevir/event/context_policy.hpp:39). It no longer describes an accepted policy that fails at serialization.

## 2 Invariant analysis

[Component validation](/Users/owebeeone/limbo/grevir-wz/grevir-core/src/grevir/interrupt/binding.hpp:192) admits only nonempty ASCII letters, digits, and underscores. The [encoding](/Users/owebeeone/limbo/grevir-wz/grevir-core/src/grevir/event/context_policy.hpp:34) records each component’s decimal length, so underscores cannot make distinct lock/context pairs collide. ASCII also makes the encoded C++ character count equal the probe byte count and permits direct insertion into the generated C++ string literal.

The complete encoded length is checked before [probe serialization](/Users/owebeeone/limbo/grevir-wz/grevir-core/src/grevir/interrupt/probe_record.hpp:89), whose text writer permits 255 bytes. The decoder reads that one-byte length, and [strict generated output](/Users/owebeeone/limbo/grevir-wz/grevir-core/tools/grevir_irqgen/emit.py:111) compares the emitted policy with the live `DeferredContextPlan`. Source inspection found no accepted deferred policy identity that exceeds the probe field. The added [255-byte assertion](/Users/owebeeone/limbo/grevir-wz/grevir-core/tests/runtime/event_queue_test.cpp:66) checks the calculation; the supplied end-to-end and negative results cover the boundary and diagnostic timing.

The added C++ uses portable C++23 facilities and introduces no conditional-compilation or control-flow-body violation. Direct-only plans do not instantiate the selected policy identity.

## 3 Risks and next action

Code-axis acceptance is supported for the stated shared contract and classic ESP32 instantiation. Silicon behavior, S2/S3, software-originated events, and named contexts remain deferred as documented. The supplied build and dispatch results should remain labeled as supplied evidence in the final review outcome.
