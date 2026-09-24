# Interrupt implementation remediation plan

This pre-commit round reviews the uncommitted snapshot digest
`8d2cde04096b5ec2dc2d91933c284d6748330ecf4c500f049969aa02f510f9c2`.
The [code](GrevirInterruptImplementation-ReviewCode.md),
[state](GrevirInterruptImplementation-ReviewState.md), and
[surface](GrevirInterruptImplementation-ReviewSurface.md) reports are the
peer-blind inputs. This plan is one merged correction, not an acceptance
verdict.

| Finding | Disposition | Closure evidence |
| --- | --- | --- |
| Code P1, State P1 | Move named-target configuration and enablement into Board lifecycle; make the selected configuration choose hardware setup; use `Application<Spec>::start()` in both sketches. | Named AVR/ESP32 target builds, simavr, negative configuration check, host start semantics. |
| Code P2 inventory | Expose each Board's typed candidate problem and require a compile-time proof that its selected plan is the solver result for catalog-derived requests. | Positive and foreign owner/configuration/selector compile probes. |
| Code P2 shared source | Explicitly defer multi-event source sharing in this first event slice and reject it in C++ and JSON before emission. Preserve the extensible record shape. | Shared-source negative probes fail at validation on all host formats. |
| Code P2 undemanded conflict | Compare source/entry only for earlier demanded bindings. | Two-request negative/positive allocator probes. |
| State P2 readiness | Add mandatory consumption-time ready/output validation to CMake and Arduino staging, checking attempt, identities, protocol/emitter and hashes. | Corrupt each output and marker independently; validation rejects before final compilation. |
| State P3 cleanup | Separate registration failure from release failure in classic ESP32 adapter. | Target compile plus injected host model cases where feasible. |
| Surface P2 mock docs | Publish a complete `/docs` mock example and run it from documented commands. | CMake mock run on macOS, Pi and Win11. |
| Surface P3 help/path | Expose `arduino-build` in top-level help; resolve Arduino CLI from PATH in docs. | Help inspection and direct copyable command on Pi. |

The AVR hardware hold remains. Classic ESP32 silicon routing remains untested;
target compile/link cannot close that evidence gap. The final review of this
changed contract must be independent of the initial reviewer findings.
