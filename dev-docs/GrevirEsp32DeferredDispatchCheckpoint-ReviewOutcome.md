# Classic ESP32 deferred-dispatch review outcome

Status: **STOP; not accepted** at the settled root `17f0f05f7e99bb831d5f052e85a9fbbcbfb2c70a`,
Core `a711cb78a4276222e135cfd806959aaf2e391bd1`, Arduino ESP32
`6b5a1776d2c2eba1fa0e00729c5f9e3ac5a79e5f`, AVR
`322e8b9fc520d7ba60b46104b14f86e2eaf79c9c`, and Test Support
`b13fea29e82d813f2ae2f6719b0adb2fb6a8cb45` tuple. The baseline is
recorded by the committed `esp32-deferred-dispatch-base-2026-09-28` GWZ snapshot
and root `e242a92`.

The third read-only review round returned [Code STOP](GrevirEsp32DeferredDispatchCheckpoint-ReviewCode-3.md),
[State GO](GrevirEsp32DeferredDispatchCheckpoint-ReviewState-3.md), and
[Surface GO](GrevirEsp32DeferredDispatchCheckpoint-ReviewSurface-3.md). The
first remediation made the main-loop task policy mandatory and stopped active
queue rebinding. The second made joined policy identities unambiguous. The
Code reviewer verified those original counterexamples closed, then found a new
P2 architectural boundary: two valid 123-character identity components now
produce a 257-character composite, while the generated probe field supports
at most 255. This case was representable before the second remediation. The
review-loop rule stops the lane after a third new architectural root cause
following two remediation rounds; the implementation remains committed but
is not marked accepted.

Validation on this tuple: 192/192 native CTests; clean mock generator build
and test; distinct generated plan identities and fingerprints for the original
collision pair; strict stale-policy rejection when compiling one pair's
generated binding against the other; staged Uno and classic ESP32 Dev Module
Arduino-ESP32 3.3.11 compile/link on Raspberry Pi. No physical silicon
validation was performed.

The next decision is whether to redesign the policy/probe representation to
preserve all accepted identity lengths or explicitly accept a narrower,
documented contract for this checkpoint. That decision must reconcile the
C++ validity check, probe schema, generator, and strict output, with boundary
tests at lengths 255, 256, and 257. No further remediation was attempted in
this review lane.
