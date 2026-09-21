#include "atmega328p_program.hpp"
#include "atmega328p_fixture.hpp"
#include <cstdlib>
#include <iostream>
using namespace timer_prototype;
namespace m = timer_prototype::atmega328p;
using atmega328p_mock::Bindings;
using atmega328p_mock::Memory;
using atmega328p_mock::word;

template <unsigned PinId, typename Rate, typename Top, unsigned Steps = 128>
using RequestFor = PwmRequest<"pwm", Frequency<Rate,Exact>, DutyStepAtMost<1,Steps>,
  For<Target::avr,Pin<PinId>,avr::FastPwm,Top>>;
using Zero = Instance<"zero",RequestFor<m::PD6,Hertz<15625,16>,avr::BuiltInTop,256>,1>;
using ZeroB = Instance<"zero_b",RequestFor<m::PD5,Hertz<15625,16>,avr::BuiltInTop,256>,1>;
using One = Instance<"one",RequestFor<m::PB1,Hertz<1000>,avr::TopFromIcr>,2>;
using OneB = Instance<"one_b",RequestFor<m::PB2,Hertz<1000>,avr::TopFromIcr>,2>;
using Two = Instance<"two",RequestFor<m::PD3,Hertz<2000>,avr::TopFromOcra>>;
using App = m::Program<Bindings,16'000'000,Zero,One,Two,OneB,ZeroB>;
using Reordered = m::Program<Bindings,16'000'000,ZeroB,OneB,Two,One,Zero>;
static_assert(App::plan.ok());
static_assert(App::plan.requests == Reordered::plan.requests);
static_assert(App::plan.candidates == Reordered::plan.candidates);
static_assert(App::Pwm<"one">::actual_frequency == Ratio{1000,1});
static_assert(App::Pwm<"two">::duty_step == Ratio{1,250});
using Reserved = m::Allocation<Bindings,16'000'000,m::Reservations<2>,One>;
static_assert(Reserved::plan.diagnostic.status == Status::reserved);
using Collision = m::Program<Bindings,16'000'000,One,Instance<"other",One::request>>;
static_assert(Collision::plan.diagnostic.status == Status::conflict);
using BadA = m::Program<Bindings,16'000'000,Instance<"bad",RequestFor<m::PB3,Hertz<2000>,avr::TopFromOcra>>>;
static_assert(BadA::plan.diagnostic.status == Status::no_candidate);

void check(bool result) { if (!result) { std::abort(); } }

using Full = m::Program<Bindings,16'000'000,Instance<"full",
  RequestFor<m::PB1,Hertz<15625,64>,avr::TopFromIcr,65536>>>;
static_assert(Full::plan.ok() && Full::Pwm<"full">::duty_step == Ratio{1,65536});
using Alternate = m::Program<Bindings,16'000'000,
  Instance<"zero",RequestFor<m::PD5,Hertz<2000>,avr::TopFromOcra,125>>,
  Instance<"two",RequestFor<m::PB3,Hertz<15625,16>,avr::BuiltInTop,256>>>;
static_assert(Alternate::plan.ok());
using Nine = m::Program<Bindings,16'000'000,Instance<"nine",
  RequestFor<m::PB1,Hertz<31250>,avr::BuiltInTop,512>>>;
using Ten = m::Program<Bindings,16'000'000,Instance<"ten",
  RequestFor<m::PB1,Hertz<15625>,avr::BuiltInTop,1024>>>;
static_assert(Nine::Pwm<"nine">::duty_step == Ratio{1,512});
static_assert(Ten::Pwm<"ten">::duty_step == Ratio{1,1024});
using PhaseOnly = m::Program<Bindings,16'000'000,Instance<"phase",PwmRequest<"pwm",
  Frequency<Hertz<1000>,Exact>,DutyStepAtMost<1,128>,Pin<m::PB1>,avr::PhaseCorrectPwm>>>;
static_assert(PhaseOnly::plan.diagnostic.status == Status::no_candidate);

void period_oracle() {
  unsigned checks = 0;
  for (auto rate : {Ratio{1,1}, Ratio{1000,1}, Ratio{15625,16}, Ratio{31250,1}}) {
    for (unsigned ppm : {0u,10'000u,1'000'000u}) {
      const auto window = FrequencyWindow::around(rate,ppm);
      for (std::uint32_t divider : {1u,8u,32u,64u,128u,256u,1024u}) {
        for (std::uint32_t maximum : {256u,65536u}) {
          std::uint32_t expected = 0;
          for (std::uint32_t cycles = 4; cycles <= maximum; ++cycles) {
            // Independent brute force cross products; this fixture's largest
            // rate/bound/period products fit uint64_t.
            const auto actual = std::uint64_t{16'000'000} * 1'000'000 * window.lower.denominator;
            const auto lower = window.lower.numerator * divider * cycles;
            if (lower <= actual) { expected = cycles; }
          }
          check(m::longest_period(window,16'000'000,divider,maximum) == expected);
          ++checks;
        }
      }
    }
  }
  check(checks == 168);
  constexpr auto maximum = std::numeric_limits<std::uint64_t>::max();
  static_assert(fraction_at_most(maximum,maximum-1,maximum-1,maximum-2));
  static_assert(!fraction_at_most(maximum-1,maximum-2,maximum,maximum-1));
}

int main() {
  atmega328p_mock::Fixture fixture;
  App::setup();
  check(Memory::bytes[0x44] == 3 && Memory::bytes[0x45] == 3);
  check(Memory::bytes[0x80] == 2 && Memory::bytes[0x81] == 0x19);
  check(word(0x86) == 15999);
  check(Memory::bytes[0xb0] == 3 && Memory::bytes[0xb1] == 0x0b);
  check(Memory::bytes[0xb3] == 249);
  check(Memory::bytes[0x2a] == 0x68 && Memory::bytes[0x24] == 6);
  check(Memory::bytes[0x2b] == 0 && Memory::bytes[0x25] == 0);
  check(App::Pwm<"zero">::write(1,2));
  check(Memory::bytes[0x47] == 127);
  check(App::Pwm<"one">::write(1,4));
  check(word(0x88) == 3999);
  check(App::Pwm<"one_b">::write(3,4));
  check(word(0x8a) == 11999 && word(0x88) == 3999);
  check(App::Pwm<"two">::write(1,2));
  check(Memory::bytes[0xb4] == 124 && Memory::bytes[0xb3] == 249);
  check(App::Pwm<"two">::write(1,250));
  check(Memory::bytes[0xb4] == 0 && (Memory::bytes[0xb0] & 0x30) == 0x20);
  check(App::Pwm<"two">::write(0,1));
  check((Memory::bytes[0xb0] & 0x30) == 0 && (Memory::bytes[0x2b] & 8) == 0);
  check(App::Pwm<"two">::write(1,1));
  check((Memory::bytes[0xb0] & 0x30) == 0 && (Memory::bytes[0x2b] & 8) == 8);
  const auto before = Memory::bytes;
  Memory::events.clear();
  check(!App::Pwm<"one">::write(1,0));
  check(!App::Pwm<"one">::write(2,1));
  check(Memory::events.empty() && before == Memory::bytes);
  Memory::reset();
  Full::setup();
  check(word(0x86) == 65535);
  check(Full::Pwm<"full">::writeTicks(65535));
  check(word(0x88) == 65534);
  check(Full::Pwm<"full">::write(65534,65535));
  check(word(0x88) == 65533);
  check(Full::Pwm<"full">::writeTicks(65536));
  check((Memory::bytes[0x80] & 0xc0) == 0 && (Memory::bytes[0x25] & 2) == 2);
  Memory::events.clear();
  check(!Full::Pwm<"full">::writeTicks(65537));
  check(Memory::events.empty());
  Memory::reset();
  Alternate::setup();
  check(Memory::bytes[0x47] == 124 && Memory::bytes[0x45] == 0x0b);
  check(Memory::bytes[0xb0] == 3 && Memory::bytes[0xb1] == 4);
  check(Alternate::Pwm<"zero">::writeTicks(1));
  check(Memory::bytes[0x48] == 0 && (Memory::bytes[0x44] & 0x30) == 0x20);
  check(Alternate::Pwm<"two">::write(1,2));
  check(Memory::bytes[0xb3] == 127 && (Memory::bytes[0xb0] & 0xc0) == 0x80);
  check(Memory::bytes[0x24] == 8 && Memory::bytes[0x2a] == 0x20);
  period_oracle();
  std::cout << "All three ATmega328P timers: generated allocation, register setup and duty writes passed\n";
}
