# Declare and allocate a timer for PWM

The portable timer declaration gives one module instance the whole timer.
An application supplies a backend capability inventory; Grevir selects a valid
binding and gives it to the module. The installed implementation covers
fixed-frequency synchronous fast PWM on ATmega328P Timer0, Timer1 and Timer2.
See [support](../supported.md) before selecting another target or waveform.

This request asks for 1 kHz with a duty step no larger than 1/256. It adds an
AVR-specific PB1 pin and ICR TOP constraint and requires at least a 16-bit
counter. Its ATmega328P section pins Timer1. Its ESP32 section documents an
alternative pin and clock source, but is ignored when the resident backend is
AVR; it is not an ESP32 driver:

```cpp
#include <grevir/avr/devices/atmega328p/pwm_backend.hpp>
#include <grevir/peripherals/timer/own.hpp>

namespace p = grevir::pwm;
namespace avr_pwm = p::atmega328p;

using MotorTimer = grevir::timer::Own<p::PwmRequest<"pwm",
  p::Frequency<p::Hertz<1000>, p::Exact>,
  p::DutyStepAtMost<1, 256>,
  p::For<p::Target::avr,
    p::Pin<avr_pwm::physical_pin<ardo::sys::avr::arch_atmega328p::rrPORTB,
      ardo::sys::avr::arch_atmega328p::ccPORTB1>()>, p::avr::TopFromIcr>,
  p::For<p::Target::esp32, p::Pin<18>, p::esp32::ApbClock>>,
  grevir::timer::For<p::Target::avr, grevir::timer::CounterBitsAtLeast<16>>,
  grevir::timer::For<p::Target::atmega328p,
    grevir::timer::RequireTimer<grevir::timer::atmega328p::Timer1>>>;
using MotorPwm = grevir::timer::Instance<"motor", MotorTimer>;
```

The common requirement and the resident target's requirements are intersected.
An unsupported active mandatory requirement fails compilation. Request identity
uses the explicit string (`"motor"` here); allocation does not change when
independent module declarations are reordered. Existing resource claims and
backend reservations participate in the search. Each selected timer and its pins
have one module owner; its internal uses receive typed bindings.

The [complete host example](../examples/pwm-host.md) supplies a memory-backed
`Device` and runs this request end to end. For a module, bind the selected
endpoint through `RequestedModule` and
`AllocatedApplication`:

```cpp
template <typename Allocation>
struct Motor : ardo::ModuleBase<
    ardo::Parameters<typename Allocation::template Pwm<"motor">>> {
  using Output = typename Allocation::template Pwm<"motor">;
  static void runSetup() {
    Output::write(1, 4); // quarter duty
  }
};

using MotorModule = grevir::RequestedModule<setl::TypeArgs<MotorPwm>, Motor>;
using App = grevir::AllocatedApplication<
  avr_pwm::Backend<Device, 16'000'000>, MotorModule>;
```

`Device` is an explicit ATmega328P timer binding over the application's register
access and interrupt-barrier policies. It must be supplied by the target
integration; the declaration above intentionally does not invent one. At setup,
the selected timers are initialized once before ordinary module setup. `runLoop()`
does not reconfigure timers. For this request, Timer1 with ICR TOP is a valid
binding, with TOP 15999 and a quarter-duty compare value of 3999. The selected
output accepts a numerator and denominator rather than requiring floating-point
arithmetic on the MCU.

Arduino's `millis()` reservation excludes Timer0 from an
`ArduinoAvrApplication`; pins 5 and 6 therefore cannot be allocated there.
Pin identities in the AVR PWM backend derive from the device's extracted port
register and bit definitions, not Arduino digital pin numbers. Board pin-alias
normalization and non-AVR timer backends remain future
work. For request semantics and limitations, see [Core](../api/core.md),
[Peripherals](../api/peripherals.md) and [AVR](../api/avr.md).
