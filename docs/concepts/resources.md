# Resource claims and conflicts

Modules and their parameters can claim physical or logical resources. Grevir
checks their combined claim set while forming an `ardo::Application`. Two
exclusive claims to the same resource fail compilation, including repeated
claims inside one module. Different overlapping ranges also conflict; adjacent
half-open ranges do not. Claiming a whole resource conflicts with any subrange.

GPIO wrappers supply claims for their pin identities. The same GPIO cannot be
assigned to two independent owners in one application. Compatible shared-use
claims can coexist when they name the same resource, ID and configuration;
different configurations conflict. An exclusive claim conflicts with a shared
one. Dependencies are included in the check, but a repeated dependency type is
one module in the closure rather than a second owner.

The Core compiler diagnoses these conflicts with `Application has resource
conflict.`. A compile-time rejection is part of the API contract, not a runtime
error to catch. See [Modules](modules.md) for lifecycle and
[PWM](../guides/pwm.md) for allocation alongside existing claims.

Pin identities are currently numeric within the selected backend; different
GPIO controllers are not automatically distinguished by a shared numeric pin
value. ATmega328P PWM backend pad identities are physical pads, while Arduino
sketch pin numbers are board-facing aliases. Do not assume those two numbering
schemes are interchangeable in a custom binding.
