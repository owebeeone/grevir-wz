#include "avr_bindings.hpp"
#include "probe_result.hpp"

// Timer1 stopped. 16-bit ICR1/TCNT1 share the TEMP latch: write high then low,
// read low then high. Reversed CPU order disagrees. ICR1 is not a live counter.

extern "C" {
volatile ProbeResult probe_result;
}

int main() {
  using R = probe::Bindings::Timer1Def::Registers;
  namespace device = probe::device;

  probe_result.magic = kProbeMagic;
  R::ReadModifyWrite(device::BitsCS11_16{device::EnumCS1::no_clk});

  R::ReadModifyWrite(device::BitsICR1{0xA5C3});
  probe_result.v[0] = R::Read<device::BitsICR1>();
  *reinterpret_cast<volatile std::uint8_t*>(0x86) = 0x34;
  *reinterpret_cast<volatile std::uint8_t*>(0x87) = 0x12;
  probe_result.v[1] = R::Read<device::BitsICR1>();

  R::ReadModifyWrite(device::BitsTCNT1{0xA5C3});
  probe_result.v[2] = R::Read<device::BitsTCNT1>();
  *reinterpret_cast<volatile std::uint8_t*>(0x84) = 0x34;
  *reinterpret_cast<volatile std::uint8_t*>(0x85) = 0x12;
  probe_result.v[3] = R::Read<device::BitsTCNT1>();

  std::uint8_t flags = 0;
  if (probe_result.v[0] == 0xA5C3) {
    flags |= 1;
  }
  if (probe_result.v[1] != 0x1234) {
    flags |= 2;
  }
  if (probe_result.v[2] == 0xA5C3) {
    flags |= 4;
  }
  if (probe_result.v[3] != 0x1234) {
    flags |= 8;
  }
  probe_result.flags = flags;
  probe_result.ready = 1;
  for (;;) {
  }
}
