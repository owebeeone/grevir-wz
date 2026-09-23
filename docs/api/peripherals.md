# Peripherals API

**Package:** Grevir Peripherals. **Header:** `<GrevirPeripherals.h>` or
`<grevir/peripherals/...hpp>`. **CMake target:** `grevir::peripherals`.
**Dependencies:** Base, Time and Core.

Portable GPIO wrappers include `ardo::InputPin<Backend, N>`,
`ardo::OutputPin<Backend, N>` and open-drain variants. `ardo::ExternalPin<N>`
claims a pin configured by an external library. Backends supply only the pin
operations used by the selected wrapper. The wrappers carry Core resource
claims; assigning the same pin to independent owners fails compilation.
See [Pins and clocks](../concepts/pins-and-clocks.md).

Polling types include `ardo::TimePoller<State, Clock>`,
`ardo::Sequence<Tick, values...>`,
`ardo::CyclicTimeSequencePoller<Sequence, Clock>` and
`ardo::TimerSequencePoller<Sequence, Clock, State>`. Expiry requires elapsed
time **greater than** the period. A poll advances one interval while retaining
the prior schedule, so repeated calls can catch up. `init()` restarts the time
origin without resetting sequence state; `reset()` resets both. Unsigned clock
wraparound works only when observations remain within the intended interval
range.

The package also supplies debounce and button-event helpers, a backend-bound
PWM wrapper, typed storage regions and portable timer requirements. AVR PWM
allocation and concrete timer setup are described in [PWM](../guides/pwm.md).
Hardware effects depend on the injected backend; native mocks do not prove
electrical timing.
