# Timer ownership implementation checkpoint

Status: **draft for implementation review**. The controlling design is
[Module-owned timer configuration](GrevirTimerModuleDesign.md). This checkpoint
records the supported implementation slice, not completion of that design.

The portable `timer::Own` declaration now groups PWM uses and common or
resident-target constraints under one stable module-instance identity. The
allocator selects one whole physical timer per instance, checks all internal
PWM uses against one configuration, and cannot join uses from two differently
named owners. Selected timer and pin claims attach to the bound module;
module parameter setup applies the selected backend payload before module
callbacks. The ATmega328P backend derives counter width from `BitsTCNT` and
pin identity from the extracted port register and bit metadata. An exact
device-timer requirement intersects with a family counter-width requirement:
Timer1 plus at least 16 bits succeeds; Timer0 plus at least 16 bits fails
during application compilation. Nonresident target options remain inert.

The only supported use category in this checkpoint is fixed-frequency PWM.
The ATmega328P candidate builder still models the two compare outputs of
Timer0/1/2 and the PWM modes it already supported. It does not yet build a
joint PWM-and-period-event candidate, connect to the interrupt plan, support
capture/count/one-shot uses, or provide an ESP32 timer backend. Timer setup
still uses the existing typed register path rather than a new general
`setl::ApplierValues` program. Those are open design/implementation items,
not properties established by the passing host tests. No silicon validation
was run.

Review question: does this slice establish the intended module ownership and
configuration boundary without making the future non-PWM modes or target
backends depend on a PWM-shaped abstraction? A reviewer should treat any
structural obstruction as blocking, even if the fixed-PWM tests pass.
