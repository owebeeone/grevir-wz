#pragma once
#include "allocator.hpp"
#include <grevir/avr/devices/atmega328p/timers.hpp>
#include <vector>

namespace timer_prototype::atmega328p {
namespace b = ardo::sys::avr::base;
namespace d = ardo::sys::avr::arch_atmega328p;
// Physical pad identities, independent of an Arduino board numbering scheme.
inline constexpr unsigned PB1 = 101, PB2 = 102, PB3 = 103, PD3 = 203, PD5 = 205, PD6 = 206;
inline constexpr std::array resources{
  Resource{1,0,Kind::timer}, Resource{2,0,Kind::timer}, Resource{3,0,Kind::timer},
  Resource{11,1,Kind::channel}, Resource{12,1,Kind::channel},
  Resource{21,2,Kind::channel}, Resource{22,2,Kind::channel},
  Resource{31,3,Kind::channel}, Resource{32,3,Kind::channel},
  Resource{PB1,0,Kind::pin}, Resource{PB2,0,Kind::pin}, Resource{PB3,0,Kind::pin},
  Resource{PD3,0,Kind::pin}, Resource{PD5,0,Kind::pin}, Resource{PD6,0,Kind::pin}
};

struct Hardware {
  unsigned timer = 0; // Zero-based device timer number.
  unsigned cs = 0;
  unsigned wgm = 0;
  std::uint16_t top = 0;
  b::TimerTop source = b::TimerTop::none;
  constexpr bool operator==(const Hardware&) const = default;
};
struct Choice { Candidate candidate; Hardware hardware; };

template <typename T> struct Types;
template <typename... T> struct Types<b::WaveformGeneratorModes<T...>> {
  template <typename F> static constexpr void each(F f) { (f.template operator()<T>(), ...); }
};
template <typename... T> struct Types<b::DividerMappings<T...>> {
  template <typename F> static constexpr void each(F f) { (f.template operator()<T>(), ...); }
};

template <typename Bindings, typename Pin>
constexpr unsigned physical_pin() {
  if constexpr (std::is_same_v<Pin, typename Bindings::Gpio::ppPB1>) { return PB1; }
  else if constexpr (std::is_same_v<Pin, typename Bindings::Gpio::ppPB2>) { return PB2; }
  else if constexpr (std::is_same_v<Pin, typename Bindings::Gpio::ppPB3>) { return PB3; }
  else if constexpr (std::is_same_v<Pin, typename Bindings::Gpio::ppPD3>) { return PD3; }
  else if constexpr (std::is_same_v<Pin, typename Bindings::Gpio::ppPD5>) { return PD5; }
  else if constexpr (std::is_same_v<Pin, typename Bindings::Gpio::ppPD6>) { return PD6; }
  else { return 0; }
}

constexpr Source source(b::TimerTop top) {
  if (top == b::TimerTop::icr) { return Source::icr; }
  if (top == b::TimerTop::ocra) { return Source::ocra; }
  return Source::built_in;
}

// Find the greatest cycle count whose frequency is >= the lower bound.
// Binary search takes at most 16 comparisons, including a zero lower bound.
constexpr std::uint32_t longest_period(FrequencyWindow window, std::uint32_t clock,
    std::uint32_t divider, std::uint32_t maximum) {
  std::uint32_t low = 4, high = maximum, best = 0;
  while (low <= high) {
    const auto middle = low + (high - low) / 2;
    const FrequencyBound actual{std::uint64_t{clock} * 1'000'000, divider * middle};
    if (window.lower.at_most(actual)) { best = middle; low = middle + 1; }
    else { high = middle - 1; }
  }
  return best;
}

template <typename Bindings, unsigned Timer, typename Def, typename Clocks,
    std::uint32_t Clock, typename Requests>
constexpr void append(std::vector<Choice>& choices, const Requests& members) {
  FrequencyWindow window;
  for (const auto& r : members) { window = window.intersect(r.config.frequency); }
  if (!window.valid() || window.empty() || members.empty() || members.size() > 2) { return; }
  Types<typename Def::ModeTraits::Modes>::each([&]<typename Mode> {
    if constexpr (Mode::timer_pwm_mode == b::TimerPwmMode::fast) {
      Types<typename Clocks::FreqMapping>::each([&]<typename Divider> {
        constexpr auto capacity = b::nfp::TimerCountField<typename Def::BitsTCNT>::capacity;
        const auto cycles = Mode::timer_top == b::TimerTop::built_in ? Mode::built_in_top + 1
          : longest_period(window, Clock, Divider::divider, capacity + 1);
        if (cycles < 4) { return; }
        const Ratio frequency = Ratio{Clock, Divider::divider * cycles}.normalized();
        if (!window.contains(frequency)) { return; }
        Candidate c;
        c.key = static_cast<unsigned>(choices.size() + 1);
        c.timer = Timer + 1;
        // Prefer the smallest prescaler, then the numeric WGM code. Within a
        // programmable mode the longest acceptable period maximizes resolution.
        c.preference = Divider::divider * 16 + static_cast<unsigned>(Mode::wgm_value);
        c.frequency = frequency;
        c.waveform = Waveform::fast;
        c.source = source(Mode::timer_top);
        for (const auto& r : members) {
          if (r.config.error != ConfigError::none || !r.config.step.valid()
              || !at_most({1,cycles}, r.config.step)
              || (r.config.waveform != Waveform::any && r.config.waveform != c.waveform)
              || (r.config.source != Source::any && r.config.source != c.source)) { return; }
          using A = typename Def::template OcrType<b::OcrEnum::OcrA>::GpioDef;
          using B = typename Def::template OcrType<b::OcrEnum::OcrB>::GpioDef;
          unsigned channel = 0;
          if (r.config.pin == physical_pin<Bindings,A>() && Mode::timer_top != b::TimerTop::ocra) {
            channel = 1;
          } else if (r.config.pin == physical_pin<Bindings,B>()) { channel = 2; }
          if (channel == 0 || (c.count != 0 && c.endpoints[0].pin == r.config.pin)) { return; }
          c.endpoints[c.count++] = {r.key, (Timer + 1) * 10 + channel, r.config.pin, {1,cycles}};
        }
        Hardware h{Timer, static_cast<unsigned>(Divider::cs_value), static_cast<unsigned>(Mode::wgm_value),
          static_cast<std::uint16_t>(cycles - 1), Mode::timer_top};
        c.configuration = c.key;
        for (const auto& prior : choices) {
          if (prior.hardware == h) { c.configuration = prior.candidate.configuration; break; }
        }
        choices.push_back({c,h});
      });
    }
  });
}

template <typename Bindings, std::uint32_t Clock, typename Requests>
constexpr auto generate(Requests requests) {
  static_assert(Clock > 0 && Clock <= 20'000'000, "ATmega328P clock outside MVP range");
  std::sort(requests.begin(), requests.end(), [](const auto& a, const auto& b) { return a.key < b.key; });
  std::vector<Choice> result;
  for (std::size_t i = 0; i < requests.size(); ++i) {
    bool previous = false;
    for (std::size_t j = 0; j < i; ++j) {
      if (requests[i].group != 0 && requests[j].group == requests[i].group) { previous = true; }
    }
    if (previous) { continue; }
    std::vector<Request> members{requests[i]};
    if (requests[i].group != 0) {
      for (std::size_t j = i + 1; j < requests.size(); ++j) {
        if (requests[j].group == requests[i].group) { members.push_back(requests[j]); }
      }
    }
    append<Bindings,0,typename Bindings::Timer0Def,d::TccrEnumTraits<d::EnumCS0>,Clock>(result,members);
    append<Bindings,1,typename Bindings::Timer1Def,d::TccrEnumTraits<d::EnumCS1>,Clock>(result,members);
    append<Bindings,2,typename Bindings::Timer2Def,d::TccrEnumTraits<d::EnumCS2>,Clock>(result,members);
  }
  return result;
}

} // namespace timer_prototype::atmega328p
