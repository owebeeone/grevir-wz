# Pins and clocks

Portable peripheral modules accept pin and clock types instead of importing
Arduino or AVR globals. A GPIO backend supplies the operations used by the
chosen wrapper: `pinMode(pin, mode)`, `digitalRead(pin)` and/or
`digitalWrite(pin, level)`. Input modes are `PullUp` and `Untied`; output modes
include ordinary output and both open-drain variants.

```cpp
#include <GrevirPeripherals.h>

using Button = ardo::InputPin<BoardGPIO, 2>;
using Led = ardo::OutputPin<BoardGPIO, 3>;
```

`BoardGPIO` is supplied by the application or target adapter. In an Arduino
sketch, `ardo::arduino::InputPin<N>` and `OutputPin<N>` use the Arduino adapter;
the [Blink example](../examples/blink.md) shows that path. An external library
that configures its own pin can use `ardo::ExternalPin<N>` for the resource claim.

A clock type supplies a `TimeType` and static `now()` returning that type.
Period values and `now()` must use compatible tick units. For example:

```cpp
struct MicrosClock {
  using TimeType = setl::Time<unsigned long, setl::TimeUnit::MICROS>;
  static TimeType now();
};
```

The application provides the function body through its target service. The
[Pulse IO AVR example](../examples/pulse-io-avr.md) binds `now()` to Arduino
`micros()`. Time pollers use unsigned elapsed subtraction to tolerate clock
wraparound within their documented interval bounds. Their expiry test is
strictly greater than the period; see the [Peripherals API](../api/peripherals.md).
