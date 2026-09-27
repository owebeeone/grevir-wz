# Main-loop Stream checkpoint — review remediation

The first review examined root `2c2ea1e1b3326caf41f38acdb8f79c02b21b4895`
and Core `6d09870d78b0628da4c6a0007c4a7d45e4ae5473`. Code and State
reported GO. Surface reported NO-GO on one public-contract contradiction.

| Finding | Disposition | Closure evidence |
| --- | --- | --- |
| [Surface P2-1](GrevirEventStreamCheckpoint-ReviewSurface.md) | Fix now | Qualify the queue paragraph by delivery mode. Trace two firings before dispatch at capacities one and two for Elide and Stream; the expected callback count, posting result and overrun state must be unambiguous from the guide alone. Ask the original Surface reviewer for a focused re-verdict on the revised tuple. |
| [Code P3-1](GrevirEventStreamCheckpoint-ReviewCode.md) | Record for later gate integration | The named AVR Stream translation unit compiled with AVR GCC 14.2, but is not part of a tracked recurring AVR gate. A future target-gate update should add it without claiming simavr Stream behavior. This bounded coverage gap does not alter the current result. |
| [Surface P3-1](GrevirEventStreamCheckpoint-ReviewSurface.md) | Record for a later example pass | The guide gives Stream declaration syntax and a staged build path, while linked runnable examples still use Elide. A linked Stream example with repeated-firing and overflow assertions would improve first-day use but is not required to remove the contradictory contract. |
| [State P3-1](GrevirEventStreamCheckpoint-ReviewState.md) | Closed by the same wording fix | It identifies the same unqualified Elide sentence as Surface P2-1, at lower severity. |

The correction is documentation-only. The implementation, generated bindings,
target support, and validation claims remain at the reviewed checkpoint.
