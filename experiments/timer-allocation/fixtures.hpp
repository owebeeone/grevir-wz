#pragma once
#include "allocator.hpp"

namespace timer_prototype {
struct MustNotValidate {};
template <Target R> struct Apply<R, MustNotValidate> {
  static constexpr void run(Config&) {
    static_assert(R != R, "POISON_OPTION_WAS_VALIDATED");
  }
};
} // namespace timer_prototype

namespace timer_prototype::fixtures {

// Entirely synthetic routing: these are not AVR or ESP32 register descriptions.
inline constexpr std::array resources{
  Resource{1, 0, Kind::timer}, Resource{2, 0, Kind::timer}, Resource{3, 0, Kind::timer},
  Resource{11, 1, Kind::channel}, Resource{12, 1, Kind::channel},
  Resource{21, 2, Kind::channel}, Resource{22, 2, Kind::channel},
  Resource{31, 3, Kind::channel}, Resource{32, 3, Kind::channel},
  Resource{101, 0, Kind::pin}, Resource{102, 0, Kind::pin}, Resource{103, 0, Kind::pin},
  Resource{201, 0, Kind::domain}
};

template <unsigned PinNumber>
using Basic = PwmRequest<"pwm", Frequency<Hertz<1000>, Exact>, DutyStepAtMost<1, 256>, Pin<PinNumber>>;
using A = Instance<"a", Basic<101>>;
using B = Instance<"b", Basic<102>>;
using C = Instance<"c", Basic<103>>;
inline constexpr Key a{"a", "pwm"};
inline constexpr Key b{"b", "pwm"};
inline constexpr Key c{"c", "pwm"};

constexpr Candidate choice(unsigned key, Key request, unsigned timer, unsigned pin) {
  Candidate result;
  result.key = key;
  result.timer = timer;
  result.configuration = 1;
  result.frequency = {1000, 1};
  result.waveform = Waveform::fast;
  result.source = Source::icr;
  result.endpoints[0] = {request, timer * 10 + 1, pin, {1, 256}};
  result.count = 1;
  return result;
}

inline constexpr auto greedy = Problem{
  requests<Target::avr, A, B>(),
  std::array{choice(1, a, 1, 101), choice(2, a, 2, 101), choice(3, b, 1, 102)},
  resources, std::array<unsigned, 0>{}
};

constexpr auto sharing() {
  auto shared = choice(1, a, 1, 101);
  shared.endpoints[1] = {b, 12, 102, {1, 256}};
  shared.count = 2;
  return Problem{
    requests<Target::avr, Instance<"a", Basic<101>, 7>, Instance<"b", Basic<102>, 7>>(),
    std::array{shared}, resources, std::array<unsigned, 0>{}
  };
}

constexpr auto domains() {
  auto x = choice(1, a, 1, 101);
  x.domain = 201; x.setting = 8;
  auto y = choice(2, b, 2, 102);
  y.domain = 201; y.setting = 64;
  auto z = choice(3, b, 2, 102);
  z.domain = 201; z.setting = 8; z.configuration = 2;
  return Problem{requests<Target::avr, A, B>(), std::array{x, y, z}, resources, std::array<unsigned, 0>{}};
}

// Declaration alone is enough for an inactive section; completing/validating it
// would be an error. Its poison sibling also has invalid numeric metadata.
struct IncompleteOption;
using Portable = PwmRequest<"pwm", Frequency<Hertz<1000>, Exact>, DutyStepAtMost<1, 256>,
  For<Target::avr, Pin<101>, avr::FastPwm, avr::TopFromIcr>,
  For<Target::esp32, Pin<102>, esp32::ApbClock>>;
using PoisonOnEsp = PwmRequest<"pwm", Frequency<Hertz<1000>, Exact>, DutyStepAtMost<1, 256>, Pin<101>,
  For<Target::esp32, IncompleteOption, Frequency<Hertz<0, 0>, WithinPpm<2'000'000>>>>;
using LazyOnEsp = PwmRequest<"pwm", Frequency<Hertz<1000>, Exact>, DutyStepAtMost<1, 256>, Pin<101>,
  For<Target::esp32, MustNotValidate>>;

} // namespace timer_prototype::fixtures
