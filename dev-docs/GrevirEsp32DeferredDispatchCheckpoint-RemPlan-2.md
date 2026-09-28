# ESP32 deferred-dispatch review remediation, round 2

Round 2 reviewed root `81d0a1d`, Core `9eb02e8`, Arduino ESP32 `6b5a177`,
AVR `322e8b9`, and Test Support `b13fea2`. Code found one new blocking
identity-encoding defect; State and Surface reported GO.

| Finding | Disposition | Closure evidence |
| --- | --- | --- |
| Code P2, ambiguous `lock + "_" + context` | Replace the joined policy with an injective length-prefixed encoding of both validated components. Preserve the single existing serialized policy field and strict comparison. | Compile-time counterexample for `alpha_beta`/`gamma` versus `alpha`/`beta_gamma`; generated plan/fingerprint difference and stale strict-header rejection; mock, Uno and ESP32 staged builds; native suite. |

This is the second and final planned remediation round for this checkpoint. A
new architectural root cause in the next review would stop acceptance and
require a redesign decision under the review-loop rule.
