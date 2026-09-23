# Pulse IO API

**Package:** Grevir Pulse IO. **Header:** `<GrevirPulseIO.h>`.
**CMake target:** `grevir::pulse_io`. **Dependencies:** Base, Time, Core,
Peripherals and Pulse Codec.

`ardo::PweTransmitterModule<Pin, Bits, Clock, Tick, ...>` and
`ardo::PweReceiverModule<Pin, Bits, Clock, Tick, ...>` connect the codec to
injected output/input pins and a typed clock. The pin types contribute Core
resource claims. `runSetup()` configures pins and the transmitter idle level;
`runLoop()` services the encoder/decoder. Call it often enough to observe and
apply every pulse edge.

`Transmit::instance.send(value)` returns false while a prior frame is active.
`Receive::instance.isDataReady()` reports a complete value, and
`readValue(value)` consumes it. The clock and waveform template periods must
use compatible units. Inversion and bit-polarity template choices are
independent. The [guide](../guides/pulse-io.md) and [complete AVR
example](../examples/pulse-io-avr.md) show use. The selected D5-to-D4 path
passes in simavr; physical pulse timing has not been measured.
