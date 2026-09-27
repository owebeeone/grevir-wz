# Deferred event dispatch checkpoint — remediation plan

Status: round 1 review NO-GO. Reviewed tuple: root `f1acfc67`, Core
`95b9c438`, Test Support `c6a618d5`, Peripherals `4612cecd`, AVR `032f907f`.
This plan maps the two blocking findings and the bounded surface finding to one
correction patch. It does not expand the checkpoint to Stream, software-only
events, ESP32 deferred execution, named contexts or deadline scheduling.

| Finding | Disposition | Correction and closure evidence |
| --- | --- | --- |
| [CODE-01](GrevirEventDispatchCheckpoint-ReviewCode.md) (P2) | Fix | Derive the active MainLoop queue configuration from the demand set and the board's selected lock policy. Encode capacity and policy identity in the target probe record and canonical JSON; the emitter must assert their equality in strict C++. An unchanged-header probe/strict mismatch must fail at the dedicated stale-context assertion. A two-event mock plan must expose capacity two, and changing it must change the fingerprint. Direct-only and zero-demand plans must remain valid without queue storage. |
| [STATE-1](GrevirEventDispatchCheckpoint-ReviewState.md) (P2) | Fix | Guard entry to each application's dispatcher under the existing queue lock, release the lock for callbacks, and clear the active guard on exit. Nested or competing dispatch returns zero without consuming a record. A mock callback must repost and attempt nested dispatch; maximum callback depth remains one and the repost is processed later. Retain overflow, queue-order and reentrant-post checks. |
| [SURFACE-01](GrevirEventDispatchCheckpoint-ReviewSurface.md) (P3) | Fix with the same patch | Publish the exact overflow-inspection and clearing calls next to the dispatch example and in the Core API reference. State that checking does not clear, explicit clearing leaves queued records intact, and startup resets the diagnostic. Include a runnable mock example. |

The plan extension records **policy identity**, not a duplicate hand-written
resource inventory. `Board::EventLock` chooses the backend critical-section
implementation; the policy type supplies its stable identifier. Only a selected
deferred context has nonzero capacity. The generator lowers validated facts and
does not choose a capacity or lock. This preserves the ownership and plan-fidelity
requirements in the [Do's](GrevirDeclarativeIntegrationDos.md) §§2, 7–9 and
[Don't-dos](GrevirDeclarativeIntegrationDontDos.md) §§1–3, 5–6.

After one patch, run focused host queue tests, target probe/strict mismatch
cases, the generated mock application on macOS, Raspberry Pi and Windows/MSVC,
and the Uno AVR compile plus simavr smoke on weftpi. Silicon remains held.
Then ask the original Code and State reviewers to verify their exact
counterexamples against the revised tuple. The Surface reviewer should verify
the first-day overflow walkthrough. No implementer self-closure is sufficient.
