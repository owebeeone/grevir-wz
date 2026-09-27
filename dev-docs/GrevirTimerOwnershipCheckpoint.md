# Timer ownership implementation checkpoint

Status: **accepted at root `e364345c0e0465007ba607bc2c6a4d8c4c078ef6`, Core
`852a38aedf9bc7bc7569e971b91d4a8d08821b11`, Peripherals
`0cf6a7d7d3358dcb3df9474c8e190d5217029e69`, AVR
`9f6bc02cc232d8a60a7b16a1ecdf16bfc8c42e0a` after Code and State
review-5 reported GO; this accepts the fixed-PWM module-owned timer slice
only**. The controlling design is
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

The final host build and all 187 CTests passed. Compiler probes included 17
expected portable-PWM rejections and the Uno/Nano board alias conflicts; the
installed two-output CMake consumer built and exited successfully. A host
compile with conflicting AVR-style macro definitions also passed. These checks
do not establish target-compiler or silicon behavior for this checkpoint.

The peer-blind [Code](GrevirTimerOwnershipCheckpoint-ReviewCode-5.md) and
[State](GrevirTimerOwnershipCheckpoint-ReviewState-5.md) final re-verdicts are
GO on the same tuple. The prior reports, State classification erratum, and
remediation dispositions remain filed beside this checkpoint. The public
[PWM guide](../docs/guides/pwm.md) and [two-output example](../docs/examples/pwm-host.md)
describe the accepted usage and support limits.
