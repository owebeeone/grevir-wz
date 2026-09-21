#pragma once

#include <avr/interrupt.h>
#include <grevir/avr/devices/atmega328p/timers.hpp>
#include <grevir/base/compat/cstdint.hpp>

namespace probe {

struct IrqBarrier {
  IrqBarrier() : saved(SREG) {
    cli();
  }
  ~IrqBarrier() {
    SREG = saved;
  }
  const std::uint8_t saved;
};

namespace device = ardo::sys::avr::arch_atmega328p;
namespace base = ardo::sys::avr::base;
using Bindings = device::TimerBindings<base::VolatileAccess, IrqBarrier>;

} // namespace probe
