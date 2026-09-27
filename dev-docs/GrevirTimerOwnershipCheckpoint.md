# Timer ownership implementation checkpoint

Status: **remediation round 2 ready for final re-review**. The controlling design is
[Module-owned timer configuration](GrevirTimerModuleDesign.md). This checkpoint
records the supported implementation slice, not completion of that design.

The portable `timer::Own` declaration groups named uses and common or
resident-target constraints under one stable module-instance identity. The
common allocator accepts use-neutral candidate envelopes and selects one whole
physical timer per instance; mock event-only and joint PWM/event candidates
exercise this shape. AVR translates its typed PWM candidates into that same
allocator and rejects unsupported event uses explicitly. The module receives
only its own selected binding view. Selected timer and pin claims attach to
the bound module; dependency-aware setup applies its selected backend payload
after prerequisite modules and before its callback. The ATmega328P backend
derives counter width from `BitsTCNT` and pin identity from extracted port
register and bit metadata. Uno/Nano board pin claims normalize to the same
device identity at the Arduino AVR adapter boundary. An exact device-timer
requirement intersects with a family counter-width requirement: Timer1 plus at
least 16 bits succeeds; Timer0 plus at least 16 bits emits a dedicated
compile-time width diagnostic with owner/target/timer/width template arguments.
Nonresident target options remain inert.

The only realized ATmega328P use category in this checkpoint is fixed-frequency PWM.
The ATmega328P candidate builder still models the two compare outputs of
Timer0/1/2 and the PWM modes it already supported. It does not yet build a
joint PWM-and-period-event candidate, connect to the interrupt plan, support
capture/count/one-shot uses, or provide an ESP32 timer backend. Timer setup
still uses the existing typed register path rather than a new general
`setl::ApplierValues` program. Those are open design/implementation items,
not properties established by the passing host tests. No silicon validation
was run.

Review question: does this corrected slice establish module ownership and a
use-neutral allocation boundary without silently accepting unsupported uses,
losing dependency ordering, or permitting board/device pin aliases? A reviewer
should treat any structural obstruction as blocking, even if the host tests
pass. Both peer-blind review rounds and their remediation dispositions are
filed beside this checkpoint.
