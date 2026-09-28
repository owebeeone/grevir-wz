# Deferred context identity length contract

Status: **accepted at root `a2e0badd2aa4e3d7592ee50d27fc6913602fe547`
and Core `f389c902d9a73e9a0579dce0dc08e34cf5af2f7b` after
[Code](GrevirDeferredContextIdentityLimit-ReviewCode.md),
[State](GrevirDeferredContextIdentityLimit-ReviewState.md), and
[Surface](GrevirDeferredContextIdentityLimit-ReviewSurface.md) reviews reported
GO; this accepts the 255-byte identity contract only**. This records the operator's decision after the
[ESP32 deferred-dispatch review stop](GrevirEsp32DeferredDispatchCheckpoint-ReviewOutcome.md).

The generated interrupt probe encodes its deferred context policy identity as
one text field with an unsigned one-byte length. The identity is an injective
length-prefixed encoding of the event lock and main-loop context identities.
The complete encoded identity, not either input alone, is limited to 255 bytes.
A longer identity is rejected during C++ template instantiation with
`GREVIR_EVENT_POLICY_ID_TOO_LONG`, before probe serialization. No generator
schema or runtime queue format changes.

This deliberately narrows the accepted board policy identity contract. There
are no current API users requiring long identities. It resolves the concrete
257-byte counterexample from Code Review 3 without weakening the original
collision and stale-generated-binding checks.

Closure evidence required: 255-byte identity probe, generator, strict compile,
link and dispatch success; 256- and 257-byte compile-time rejection with the
named diagnostic; the original two-policy collision remains distinct and a
stale binding is rejected; native and selected target builds pass. Review must
also inspect the shared C++/probe boundary for any accepted value that the
probe cannot represent. Hardware validation remains deferred.
