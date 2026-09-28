# ESP32 deferred-dispatch review remediation

Round 1 reviewed root `12f6871`, Core `9179078`, Test Support `b13fea2`,
AVR `322e8b9`, and Arduino ESP32 `d4f724d`.

| Finding | Disposition | Closure evidence |
| --- | --- | --- |
| Code P2, optional `MainLoopContext` | Require an explicit context policy in every selected deferred plan. Mock and AVR use an explicit unrestricted policy; ESP32 uses the loop-task policy. Compose the lock and context identities into the existing canonical policy field so a changed context invalidates generated bindings. Remove the optional queue branches. | Negative missing-policy compile fails; host foreign-task dispatch test passes; mock, Uno and ESP32 staged builds pass, including generated strict checks. |
| State P3, backend-wide owner/rebind | Address the shared owner by making ESP32 context state per application and rejecting `prepare` while a queue is active. Treat a stopped queue as eligible for a new owner. | Host policy test attempts foreign-task reprepare while active and verifies the original owner still dispatches; ESP32 target build passes. |
| Surface P3, misleading `ticks` name | Rename the example field to `delivered_callbacks` and explain that Elide can coalesce firings. | Public example and guide use the same meaning; ESP32 staged build passes. |

This is one merged remediation patch. No silicon validation, general timer-owner
repair, named task context, software event, or sleeping dispatcher is included.
