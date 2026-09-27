# Deferred event dispatch checkpoint — review outcome

Status: **NO-GO after review round 2**. The reviewed implementation is the
settled tuple root `3ea2c2d31ab50b099247687274c0571e300352ca`, Core
`0944c59b0c7a1ca53656f4a71fd62f1cce8969f5`, Test Support
`d8fdd2e86c07ff62d08e282fc7fdb9990cdf9fec`, Peripherals
`4612cecdc99c22f326a0c1498846fea34c0c596c`, and AVR
`5f4feffa15110aac9bf27f0bc17c2ce668b003d6`. The reports are
[Code](GrevirEventDispatchCheckpoint-ReviewCode-2.md),
[State](GrevirEventDispatchCheckpoint-ReviewState-2.md), and
[Surface](GrevirEventDispatchCheckpoint-ReviewSurface-2.md). They were
independent, peer-blind and read-only, with the declarative
[Dos](GrevirDeclarativeIntegrationDos.md) and
[DontDos](GrevirDeclarativeIntegrationDontDos.md) as controlling rules.

The original CODE-01 plan-fidelity and STATE-1 nested-dispatch blockers are
closed on this tuple. The original SURFACE-01 overflow documentation gap is
closed. Round 2 found two new P2 defects:

1. **Strict-build demand omission (Code P2-1):** a newly visible catalogued
   handler can be absent from the generated demand set and receive no entry.
   Strict compilation currently checks only previously bound handlers.
   This is a distinct structural root cause in the hardware-event activation
   path, inherited from the initial checkpoint and within its scope.
2. **Capacity narrowing (State P2-1):** on the supported AVR ABI, a declared
   capacity such as `65537UL` can narrow to one before the plan and runtime
   range checks. The canonical plan then validates and describes the wrong
   capacity.

The Surface axis reported GO with three P3 documentation corrections: align
ESP32 direct-interrupt support wording, show qualified publication outcomes,
and make the Uno example's overrun diagnostic visible. These do not alter the
NO-GO classification.

Validation completed on the reviewed source: 189 native CTests and the
generated mock example passed on macOS; the probe/plan/strict round trip
passed locally and on Raspberry Pi, including the two-event mismatch and
inactive-context cases; the generated mock example passed on Raspberry Pi
and Win11/MSVC; the Uno example compiled under AVR GCC 14.2 and its Timer1
handler fired once in simavr. These checks do not refute the two new source
counterexamples. The older `check_proofs.py` script still fails because its
PWM fixture names the removed `atmega::PB1`; it was not used as evidence for
this checkpoint. Silicon validation remains held.

Under the [review-loop skill](../../../.claude/skills/review-loop/SKILL.md),
the distinct architectural root causes have reached the lane's remediation
cap. No further patch is made under this review object. The next decision is
to redesign the strict demand-agreement boundary and open a new checkpoint,
or explicitly accept this limitation. If the strict boundary is redesigned,
the capacity validation and P3 documentation corrections can be handled in
that new scope with exact negative probes. This document records the stop;
it does not mark the event checkpoint accepted, tagged or pushed.
