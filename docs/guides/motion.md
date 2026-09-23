# Quadrature encoder and stepper

Encoder and Stepper are separate portable packages. Both receive pin and clock
types from the application; neither imports Arduino or an MCU backend. Their
selected host consumers run with injected GPIO. Arduino target compilation for
these two packages has not been recorded in v0.1.0.

For a quadrature encoder, the [complete host example](../examples/encoder-host.md)
supplies pins and checks one transition:

```cpp
#include <GrevirEncoder.h>
#include <GrevirPeripherals.h>

using PinA = ardo::InputPin<BoardGPIO, 5>;
using PinB = ardo::InputPin<BoardGPIO, 6>;
using Encoder = quad::QuadEncoder<PinA, PinB>;
using Module = quad::QuadEncoderModule<Encoder>;
using App = ardo::Application<Module>;
```

`App::runSetup()` configures both pins. Poll `App::runLoop()` to sample them.
The first sample latches state and contributes zero. Adjacent Gray-code changes
contribute +1 or -1; a two-bit jump uses the last direction, if known. The
pin polarity is documented in [Encoder](../api/encoder.md).

For a four-pin stepper, the [complete host example](../examples/stepper-host.md)
supplies pins and clock ticks:

```cpp
#include <GrevirStepper.h>
#include <GrevirPeripherals.h>

using Module = step::StepperModule<BoardClock, 20, 50,
  setl::TimeUnit::MILLIS,
  ardo::OutputPin<BoardGPIO, 5>,
  ardo::OutputPin<BoardGPIO, 6>,
  ardo::OutputPin<BoardGPIO, 7>,
  ardo::OutputPin<BoardGPIO, 8>>;
using App = ardo::Application<Module>;
```

`BoardClock` supplies typed time through `now()`. Set a target position on
`Module::instance.stepper`, then poll `App::runLoop()`. A step is taken only
when elapsed time is strictly greater than the scaled step period. A combined
encoder-follower Arduino example remains outside the validated v0.1.0 set.
See [Stepper](../api/stepper.md) for phase and coil-off behavior.
