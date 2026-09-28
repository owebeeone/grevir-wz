# Deferred context identity limit review outcome

Status: **GO; accepted** at the settled root
`a2e0badd2aa4e3d7592ee50d27fc6913602fe547`, Core
`f389c902d9a73e9a0579dce0dc08e34cf5af2f7b`, Arduino ESP32
`6b5a1776d2c2eba1fa0e00729c5f9e3ac5a79e5f`, AVR
`322e8b9fc520d7ba60b46104b14f86e2eaf79c9c`, and Test Support
`b13fea29e82d813f2ae2f6719b0adb2fb6a8cb45` tuple.
[Code](GrevirDeferredContextIdentityLimit-ReviewCode.md),
[State](GrevirDeferredContextIdentityLimit-ReviewState.md), and
[Surface](GrevirDeferredContextIdentityLimit-ReviewSurface.md) independently
reported GO with no open P0–P2 finding. This follow-up resolves the P2 in the
[prior STOP outcome](GrevirEsp32DeferredDispatchCheckpoint-ReviewOutcome.md)
by making 255 encoded bytes the explicit accepted policy identity limit.

The full 255-byte mock case passed probe generation, strict compilation,
link, and dispatch. The 256- and 257-byte cases failed at template
instantiation with `GREVIR_EVENT_POLICY_ID_TOO_LONG` under Apple Clang and
MSVC. The original ambiguous-pair cases still produced distinct plans and
fingerprints; compiling one generated binding against the other live policy
failed with `GREVIR_IRQ_STALE_EVENT_CONTEXT_POLICY`. Native CTest passed
192/192, and staged Uno and classic ESP32 Arduino-ESP32 3.3.11 builds passed
on Raspberry Pi. The code reviewers inspected these results as supplied
evidence and did not rerun the builds.

Acceptance covers the described compiler/generator integration and classic
ESP32 deferred route. Physical ESP32 interrupt delivery and cross-core
timing remain unvalidated. The unrelated root archive
`grevir-source-docs.aiar.sh.txt` was left untouched at the operator's request.
