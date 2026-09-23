# Pulse encoding and GPIO I/O

Grevir Pulse Codec turns a fixed-width value into timed levels and decodes
sampled levels. It owns no GPIO or clock. Grevir Pulse IO supplies Core modules
that bind the codec to injected pin and clock types. Choose the codec alone for
a custom transport, or Pulse IO when a GPIO polling loop suits the application.

An 8-bit transmitter and receiver can be declared as follows:

```cpp
#include <GrevirPulseIO.h>

using Bits = setl::PweBitCollector<8>;
using Receive = ardo::PweReceiverModule<
  ardo::InputPin<BoardGPIO, 4>, Bits, BoardClock,
  std::uint32_t, 10, 200, 100>;
using Transmit = ardo::PweTransmitterModule<
  ardo::OutputPin<BoardGPIO, 5>, Bits, BoardClock,
  std::uint32_t, 10, 200, 100>;
using App = ardo::Application<Receive, Transmit>;
```

The backend must supply the pin operations; `BoardClock::now()` must use the
same units as the waveform periods. Call `App::runSetup()` once. Call
`App::runLoop()` frequently enough to observe the edges and meet the encoder's
deadlines. `Transmit::instance.send(value)` returns false while a prior frame
is active. `Receive::instance.readValue(value)` consumes a complete frame;
`isDataReady()` reports whether one is waiting. Giving both modules the same
GPIO pin is a compile-time conflict.

The [complete Uno example](../examples/pulse-io-avr.md) uses Arduino `micros()`,
D5 output and D4 input. Its selected 8-bit path compiled for AVR and passed a
simavr GPIO loopback. Real pin timing and electrical behavior remain unvalidated.
See [Pulse Codec](../api/pulse-codec.md) and [Pulse IO](../api/pulse-io.md) for
storage and timing contracts.
