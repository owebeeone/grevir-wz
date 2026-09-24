# Interrupt binding architecture — CONSISTENCY-AXIS REVIEW

**Review object:** `dev-docs/GrevirInterruptBindingArchitecture.md`, production design specification, blob `670412dd24e0d35854509ab9883d83d371e1006e`
**Baseline:** Repository HEAD `6e561be2921bcd9045759413b513dd38a739c76e`; reviewed file read from the matching working-tree blob. The HEAD and blob hashes matched at both the start and end of review. Other working-tree changes were out of scope.
**Date:** 2026-09-24
**Axis:** Internal consistency, agreement with the controlling design and existing timer contract, and satisfiability of the proposed build gates. Independent, adversarial, read-only; nothing here relies on the other axis.

**Verdict: NO-GO** — two P2 findings block. I pre-commit to GO on this axis if the revised design closes P2-1 and P2-2 without introducing another blocking inconsistency.

---

## 0. Evidence base

I read the pinned architecture at lines 1–464, the prior self-review (`76fd79adca2572512bd2f65749e871ce9e26c0c8`), the earlier investigation (`ab694b46b8bd90d9c7ec4752b14f9c9a0a09c4a6`), the installed PWM contract, the timer allocation design, the current `AllocatedApplication` source, and the host key-gate fixture. I applied `AGENTS.md`, `AGENTS_GWZ.md`, and the CrossMcu, Avr, and Esp32 review policies. Inspection used `rg`, `nl`, `git show`, `git status --short`, `git rev-parse HEAD`, and `git hash-object`. I made no edits and ran no builds or tests. Target compilation and silicon behavior remain unverified.

## 1. Findings

### [P2-1] The specified two-command pipeline has no complete publication transaction

**Location:** Architecture lines 188–200, 238–244, 256–263, 336–353, and 384–408. **Scope/classification:** Shared host build tooling; proposed-design correctness and recovery defect, introduced by this specification.

**Violated invariant:** The JSON and generated C++ must describe one emission, and prior output must not survive a failed generation transaction. The specified `plan` command publishes JSON before the separate `emit` command produces the header and source. The stated emitter transaction covers its output files, but no step retires the previously published C++ when `plan` succeeds and `emit` fails.

**Credible sequence and impact:** Version A builds successfully. An application change produces plan B; `plan` writes JSON B, then `emit` fails. JSON B remains beside header/source A. This contradicts the promised complete output set and leaves a mixed artifact directory requiring recovery logic that the CMake and Arduino procedures do not define. A failed build should stop, but the artifact state itself violates the specified no-fallback rule.

**Required correction:** Define one owner for staging and promotion of all three artifacts, or explicitly remove/invalidate old generated C++ before publishing a new JSON plan and specify failure cleanup for both commands. State how CMake’s separate custom commands and the Arduino wrapper enforce that rule.

**Closure test:** Start with successful artifacts A, produce JSON B, inject an `emit` failure, and verify that no usable A C++ remains alongside B. Repeat with failure during `plan`, then verify a subsequent build cannot consume either partial state.

### [P2-2] The `emit` freshness gate lacks a current-state comparison input

**Location:** Architecture lines 188–200, 238–261, 311–317, and 400–408. **Scope/classification:** Shared host build tooling; proposed-design compatibility and diagnosability defect, introduced by this specification.

**Violated invariant:** `grevir-irqgen emit` is required to reject a *stale* JSON plan, while the JSON is specified as its structured input and the probe object is parsed only by `plan`. Schema, canonical-order, and internal fingerprint checks can establish that JSON is well formed; they cannot establish that its application and toolchain fingerprints still match the current build without a separately defined current-state input or trusted handoff.

**Credible sequence and impact:** Generate a valid plan, change the application header or selected toolchain, then invoke `emit` on the old JSON. As specified, `emit` has no defined comparison against the changed inputs. It can emit stale files while claiming its stale-plan gate passed. The later strict C++ plan comparison may catch changed bindings, but it does not itself establish the promised emitter-side freshness check for all recorded provenance.

**Required correction:** Specify the current application, backend metadata, board and toolchain evidence supplied to `emit`, and define exactly which fingerprints it recomputes or checks against a fresh probe result. Alternatively, assign freshness exclusively to the build adapters and narrow the claim made for standalone `emit`.

**Closure test:** Generate valid JSON, independently change each recorded provenance input, and invoke `emit` with that JSON. Each stale case must fail at the documented gate; unchanged inputs must emit deterministically.

## 2. Invariant analysis

The central ownership graph is coherent: handler presence supplies demands, allocation selects one configuration, and source groups drive entry generation and enabling (lines 13–29, 108–160, 265–307). The stable-key gate resolves catalog types without requiring the generator to spell C++ event types; the cited host fixture supports the self-review’s narrow syntax claim. The design also explicitly distinguishes an ESP32 peripheral source from a CPU line and requires one owner per source. Its mock path uses the same discovery and plan protocol rather than a handwritten allocation fixture. These checks do not establish target behavior, which the document correctly leaves as an implementation gate.

The two failed invariants are at the JSON handoff: artifact publication crosses two commands without a defined common transaction, and `emit` claims freshness without defining what current state it compares.

## 3. Risks and next action

The preallocation handler-body cycle, object-section transport, Arduino compilation-database behavior, AVR vector ABI, and ESP32 registration behavior remain explicit implementation gates. No target or hardware result is claimed here.

Revise the generator protocol to specify artifact promotion, failure cleanup, and the provenance inputs to `emit`; then re-review the corrected blob against the two counterexamples above.
