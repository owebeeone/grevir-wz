# Pulse IO on Uno

Requires Grevir Base, Time, Core, Peripherals, Registers, Pulse Codec, Pulse IO, Arduino, Arduino AVR and AVR. This exact sketch compiled for Uno. A separate simavr probe connected D5 output to D4 input and decoded a frame. Connect the pins only when physical validation is intentionally scheduled; no board run is claimed here. `MicrosClock` and waveform periods are in microseconds.

```cpp
#include <GrevirArduinoAVR.h>
#include <GrevirPulseIO.h>

struct MicrosClock {
  using TimeType = setl::Time<unsigned long, setl::TimeUnit::MICROS>;

  static TimeType now() {
    return TimeType(ardo::CoreIF::micros());
  }
};

using Bits = setl::PweBitCollector<8>;
using Receive = ardo::PweReceiverModule<
  ardo::arduino::InputPin<4, ardo::gpio::InputPinMode::Untied>,
  Bits, MicrosClock, unsigned long, 10, 200, 100>;
using Transmit = ardo::PweTransmitterModule<
  ardo::arduino::OutputPin<5>, Bits, MicrosClock, unsigned long, 10, 200, 100>;
using App = ardo::ArduinoAvrApplication<Receive, Transmit>;

volatile unsigned char last_received = 0;
unsigned char next_value = 0;

void setup() {
  App::runSetup();
}

void loop() {
  App::runLoop();
  if (Transmit::instance.send(next_value)) {
    ++next_value;
  }
  Bits::value_type value{};
  if (Receive::instance.readValue(value)) {
    last_received = value;
  }
}
```

The packaged sketch is `grevir-pulse-io/examples/AvrLoopback/AvrLoopback.ino`. See [target support](../supported.md) for the distinction between compilation, simulation and physical validation.
