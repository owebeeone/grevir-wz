#include "avr_bindings.hpp"
#include "probe_result.hpp"
#include <avr/interrupt.h>
#include <avr/io.h>

// Timer0 normal mode, clk/1, overflow interrupt. Host measures pending,
// vector, ISR body and RETI in simavr cycles (not silicon).

extern "C" {
volatile ProbeResult probe_result;
}

ISR(TIMER0_OVF_vect) {
  const auto count = probe_result.v[0];
  probe_result.v[0] = static_cast<std::uint16_t>(count + 1);
  if (count == 0) {
    probe_result.v[1] = TCNT0;
    probe_result.ready = 1;
  }
}

int main() {
  using R = probe::Bindings::Timer0Def::Registers;
  namespace device = probe::device;

  probe_result.magic = kProbeMagic;
  probe_result.ready = 0;
  probe_result.v[0] = 0;

  R::ReadModifyWrite(
    device::BitsWGM0_210{device::EnumWGM0::normal},
    device::BitsCS01_16{device::EnumCS1::clk1});
  R::ReadModifyWrite(device::BitsTCNT0{0});
  R::ReadModifyWrite(device::BitsTOV0{true});
  R::ReadModifyWrite(device::BitsTOIE0{true});
  sei();
  for (;;) {
  }
}
