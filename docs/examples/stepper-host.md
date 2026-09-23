# Stepper with injected host pins

Requires Grevir Stepper, Peripherals and their transitive Grevir dependencies. This exact installed consumer compiles and runs on the native host. It checks pin setup, strict period expiry and two steps. It is a host mock example, not an AVR firmware result.

```cpp
#include <GrevirStepper.h>
#include <GrevirPeripherals.h>
#include <array>
#include <cstdint>

namespace {
struct Backend {
  inline static std::array<bool, 16> levels{};
  inline static unsigned setups = 0;
  static void pinMode(unsigned, ardo::gpio::OutputPinMode) {
    ++setups;
  }
  static void digitalWrite(unsigned pin, bool level) {
    levels.at(pin) = level;
  }
};
struct Clock {
  using TimeType = setl::Time<std::uint32_t>;
  inline static std::uint32_t ticks = 0;
  static TimeType now() {
    return TimeType(ticks);
  }
};
using Module = step::StepperModule<Clock, 20, 0, setl::TimeUnit::MILLIS,
  ardo::OutputPin<Backend, 5>, ardo::OutputPin<Backend, 6>,
  ardo::OutputPin<Backend, 7>, ardo::OutputPin<Backend, 8>>;
using App = ardo::Application<Module>;
}

int main() {
  App::runSetup();
  if (Backend::setups != 4) {
    return 1;
  }
  Module::instance.stepper.setTargetPosition(2);
  App::runLoop();
  if (Module::instance.stepper.getCurrentPosition() != 0) {
    return 2;
  }
  Clock::ticks = 21;
  App::runLoop();
  if (Module::instance.stepper.getCurrentPosition() != 1) {
    return 3;
  }
  Clock::ticks = 42;
  App::runLoop();
  if (Module::instance.stepper.getCurrentPosition() != 2) {
    return 4;
  }
  if (Backend::levels[5] || Backend::levels[6] || !Backend::levels[7] || !Backend::levels[8]) {
    return 5;
  }
  return 0;
}
```

The packaged source is `grevir-stepper/tests/installed-consumer/main.cpp`. See [support](../supported.md) for evidence levels.
