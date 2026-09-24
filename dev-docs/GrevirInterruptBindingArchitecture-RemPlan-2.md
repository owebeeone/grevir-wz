# Interrupt binding architecture — second remediation plan

The reviewed object is `GrevirInterruptBindingArchitecture.md` at Git blob
`033a30d45c04e3697c5d5b3c22b8abd361008e6c`. The round-two peer-blind
reports are [Consistency](GrevirInterruptBindingArchitecture-ReviewConsistency-2.md)
and [Safety](GrevirInterruptBindingArchitecture-ReviewSafety-2.md). Safety
reported GO. Consistency closed all five original findings, then found one new
architectural root cause. This is the second and final architectural
remediation round permitted by the review-loop skill for this object.

| Finding | Disposition | Closure evidence required |
| --- | --- | --- |
| Consistency round 2 P2-1 | Define `StartResult` as a settled setup outcome plus an independent per-call disposition (`initiated`, `waited`, `replayed`). Preserve the originating configuration/registration failure and any cleanup failure for every caller. Repeated invocation never substitutes an error code for the settled outcome. | Reviewer retraces first success and replay, concurrent success, and second-registration failure with concurrent and later callers. Mock and ESP32 implementation gates assert both result fields and retained failure details. |

The correction is limited to the public result contract and the matching
lifecycle/evidence wording. Fresh reviewers will assess the corrected blob
because this is a public API change. A further new architectural root cause
requires stopping this design lane for operator redesign-or-acceptance rather
than drafting another architecture patch.
