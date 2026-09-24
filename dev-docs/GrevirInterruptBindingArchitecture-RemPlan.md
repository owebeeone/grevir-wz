# Interrupt binding architecture — remediation plan

The reviewed object is `GrevirInterruptBindingArchitecture.md` at Git blob
`670412dd24e0d35854509ab9883d83d371e1006e`. The two peer-blind reports
are [Consistency](GrevirInterruptBindingArchitecture-ReviewConsistency.md) and
[Safety](GrevirInterruptBindingArchitecture-ReviewSafety.md). Both independently
found the incomplete JSON/C++ publication boundary. This is the first
remediation round for the draft design; no implementation or target validation
is claimed.

| Finding | Disposition | Closure evidence required |
| --- | --- | --- |
| Consistency P2-1; Safety P2-2 | One root cause. The build adapter invalidates a ready marker before `plan`, stages JSON and C++ together in a private generation directory, and publishes the marker only after all three files exist. A failed command leaves no ready set. CMake and Arduino depend on/verify the marker and never fall back to an older generation. Include emitter identity in the set ID. | Reviewers retrace plan failure, emit failure, and interruption after each file write with older valid output present. The future implementation injects these failures and proves no mixed set is compiled/exported. |
| Consistency P2-2 | Narrow `emit`'s guarantee to schema/canonical/internal plan validation. The build adapter owns freshness by running `plan` for the current inputs and verifying the expected target and provenance before `emit`; strict compilation compares the current C++ plan with emitted constants. Standalone `emit` is an internal command, not a freshness oracle. | Reviewer retraces a changed application/board/toolchain against the documented gates. Future tests vary each input and require generation or strict compilation to reject stale output. |
| Safety P2-1 | Specify a terminal failed-start state: all sources masked, already acquired registrations released where possible, unreleased handles retained and reported, no retry in that firmware session. Concurrent callers receive the settled failure. | Reviewer retraces failure on the second registration and concurrent/repeated `start()`. Future mock fault injection checks handle ownership and caller results; ESP32 target check remains named separately. |
| Safety P2-3 | Make startup pending-event treatment explicit per offered event in backend metadata: preserve/deliver with safe snapshot/ack semantics, or deliberate discard only with a stated quiescent precondition. No unconditional clear. | Reviewer retraces a pending event raised after module initialization and before enable. Future mock test injects it for each policy; target policy verification is separate. |

One amendment to the architecture document resolves these findings. The same
reviewers will re-check their original counterexamples on the revised blob.
