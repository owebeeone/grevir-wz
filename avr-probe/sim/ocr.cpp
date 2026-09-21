#include "avr_bindings.hpp"
#include "probe_result.hpp"

// Timer1 Fast PWM, TOP = ICR1. OCR1A is double-buffered and updates at BOTTOM.
// Write a new OCR mid-period (after the old match, before the new match) and
// check that OCF1A does not fire at the new value before BOTTOM.

extern "C" {
volatile ProbeResult probe_result;
}

int main() {
  using R = probe::Bindings::Timer1Def::Registers;
  namespace device = probe::device;

  probe_result.magic = kProbeMagic;
  probe_result.ready = 0;

  R::ReadModifyWrite(
    device::BitsWGM1_3210{device::EnumWGM1::fast_pwm_icr},
    device::BitsCS11_16{device::EnumCS1::no_clk},
    device::BitsCOM1A{device::EnumCOMn::clear});
  R::ReadModifyWrite(device::BitsICR1{200});
  R::ReadModifyWrite(device::BitsOCR1A{40});
  R::ReadModifyWrite(device::BitsCS11_16{device::EnumCS1::clk8});

  R::ReadModifyWrite(device::BitsTOV1{true});
  while (!R::Read<device::BitsTOV1>()) {
  }
  R::ReadModifyWrite(device::BitsTOV1{true});
  R::ReadModifyWrite(device::BitsOCF1A{true});

  for (;;) {
    const auto tcnt = R::Read<device::BitsTCNT1>();
    if (tcnt >= 70 && tcnt <= 100) {
      probe_result.v[0] = tcnt;
      break;
    }
  }

  R::ReadModifyWrite(device::BitsOCF1A{true});
  R::ReadModifyWrite(device::BitsOCR1A{120});
  probe_result.v[1] = R::Read<device::BitsTCNT1>();
  probe_result.v[2] = R::Read<device::BitsOCR1A>();

  R::ReadModifyWrite(device::BitsTOV1{true});
  while (!R::Read<device::BitsTOV1>()) {
  }

  const bool matched = R::Read<device::BitsOCF1A>();
  probe_result.v[3] = matched ? 1 : 0;
  probe_result.flags = matched ? 0 : 1;
  probe_result.ready = 1;
  for (;;) {
  }
}
