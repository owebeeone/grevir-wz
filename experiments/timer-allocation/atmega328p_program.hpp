#pragma once
#include "atmega328p_candidates.hpp"

namespace timer_prototype::atmega328p {

template <unsigned... Ids> struct Reservations {
  inline static constexpr std::array<unsigned, sizeof...(Ids)> values{Ids...};
};

// Experimental end-to-end binding. All allocation/generation is constant
// evaluated; only fixed register operations and bounded duty arithmetic run.
template <typename Bindings, std::uint32_t Clock, typename Reserved, typename... Instances>
struct Allocation {
  inline static constexpr auto input = requests<Target::atmega328p, Instances...>();
  inline static constexpr auto choices = [] {
    constexpr auto count = generate<Bindings,Clock>(input).size();
    const auto generated = generate<Bindings,Clock>(input);
    std::array<Choice,count> result{};
    std::copy(generated.begin(), generated.end(), result.begin());
    return result;
  }();
  inline static constexpr auto problem = [] {
    std::array<Candidate,choices.size()> candidates{};
    for (std::size_t i = 0; i < choices.size(); ++i) { candidates[i] = choices[i].candidate; }
    return Problem{input, candidates, resources, Reserved::values};
  }();
  inline static constexpr auto plan = compile(problem);

  template <std::size_t Index>
  struct Selected {
    inline static constexpr auto hardware = choices[Index].hardware;
    using TimerDef = std::tuple_element_t<hardware.timer,
      std::tuple<typename Bindings::Timer0Def, typename Bindings::Timer1Def, typename Bindings::Timer2Def>>;
    using Registers = typename TimerDef::Registers;
    struct Config {
      static constexpr b::TimerTop timer_top = hardware.source;
      static constexpr std::uint32_t top_count = hardware.top;
      static constexpr std::uint32_t capacity = b::nfp::TimerCountField<typename TimerDef::BitsTCNT>::capacity;
    };
    template <b::OcrEnum Channel>
    using Output = b::TimerOutputPin<b::TimerOutputPinSettings<Channel,true>, Selected>;

    template <b::OcrEnum Channel>
    static bool write(std::uint16_t numerator, std::uint16_t denominator) {
      if (denominator == 0 || numerator > denominator) { return false; }
      constexpr std::uint32_t cycles = std::uint32_t{hardware.top} + 1;
      // <=65536*65535 fits uint32_t, including under the AVR 16-bit int ABI.
      // Round down to a realizable number of high ticks. No FP or 64-bit work.
      const auto ticks = (cycles * std::uint32_t{numerator}) / denominator;
      return writeTicks<Channel>(ticks);
    }
    template <b::OcrEnum Channel>
    static bool writeTicks(std::uint32_t ticks) {
      using Pin = Output<Channel>;
      constexpr std::uint32_t cycles = std::uint32_t{hardware.top} + 1;
      if (ticks > cycles) { return false; }
      if (ticks == 0 || ticks == cycles) {
        Pin::pwmWrite(ticks == 0 ? 0u : hardware.top, hardware.top);
      } else {
        // Fast PWM OCR=0 represents a one-tick pulse, not a zero-duty endpoint.
        Pin::pwmWriteAbsoluteValue(static_cast<typename Pin::OCR::type>(ticks - 1));
        Pin::setupTimerOutputMode();
      }
      return true;
    }

    template <b::OcrEnum Channel>
    static void initialize_output() {
      write<Channel>(0,1);
      Output<Channel>::setupGpio();
    }
    static void setup() {
      using CS = typename TimerDef::BitsCS;
      using WGM = typename TimerDef::BitsWGM_16;
      using Count = typename TimerDef::BitsTCNT;
      using COMA = typename TimerDef::template OcrType<b::OcrEnum::OcrA>::COM8;
      using COMB = typename TimerDef::template OcrType<b::OcrEnum::OcrB>::COM8;
      // Establish stopped normal mode before writing buffered TOP registers.
      Registers::ReadModifyWrite(CS{static_cast<typename CS::type>(0)},
        WGM{static_cast<typename WGM::type>(0)});
      Registers::ReadModifyWrite(COMA{COMA::type::disconnect}, COMB{COMB::type::disconnect});
      Registers::ReadModifyWrite(Count{0});
      if constexpr (hardware.source != b::TimerTop::built_in) {
        using TOP = typename TimerDef::template TimerDefTopRegister<hardware.source>;
        Registers::ReadModifyWrite(TOP{static_cast<typename TOP::type>(hardware.top)});
      }
      for_each_output(std::make_index_sequence<choices[Index].candidate.count>{});
      Registers::ReadModifyWrite(WGM{static_cast<typename WGM::type>(hardware.wgm)});
      Registers::ReadModifyWrite(CS{static_cast<typename CS::type>(hardware.cs)});
    }
    template <std::size_t... E>
    static void for_each_output(std::index_sequence<E...>) {
      (initialize_output<choices[Index].candidate.endpoints[E].channel % 10 == 1
        ? b::OcrEnum::OcrA : b::OcrEnum::OcrB>(), ...);
    }
  };

  template <std::size_t Index>
  static void setup_selected() {
    constexpr bool used = [] {
      for (auto key : plan.candidates) {
        if (key == choices[Index].candidate.key) { return true; }
      }
      return false;
    }();
    if constexpr (used) { Selected<Index>::setup(); }
  }
  template <std::size_t... I>
  static void setup_all(std::index_sequence<I...>) { (setup_selected<I>(), ...); }
  // Caller owns the timers exclusively, enables their peripheral clocks and
  // leaves Timer2 synchronous (AS2=0), with timer interrupts disabled. Startup
  // is sequential; this does not promise glitch-free live reconfiguration.
  static void setup() {
    require_success<plan.diagnostic.status>();
    setup_all(std::make_index_sequence<choices.size()>{});
  }

  template <Text InstanceName, Text LocalName = "pwm">
  struct Pwm {
    inline static constexpr Key key{InstanceName.view(),LocalName.view()};
    inline static constexpr auto index = [] {
      for (std::size_t i = 0; i < plan.requests.size(); ++i) {
        if (plan.requests[i] == key) {
          for (std::size_t c = 0; c < choices.size(); ++c) {
            if (choices[c].candidate.key == plan.candidates[i]) { return c; }
          }
        }
      }
      return choices.size();
    }();
    static_assert(plan.ok() && index < choices.size(), "GREVIR_TIMER_BINDING_UNAVAILABLE");
    inline static constexpr auto channel = [] {
      for (const auto& endpoint : choices[index].candidate.endpoints) {
        if (endpoint.request == key) {
          return endpoint.channel % 10 == 1 ? b::OcrEnum::OcrA : b::OcrEnum::OcrB;
        }
      }
      return b::OcrEnum::OcrA;
    }();
    inline static constexpr Ratio actual_frequency = choices[index].candidate.frequency;
    inline static constexpr Ratio duty_step{1,std::uint32_t{choices[index].hardware.top} + 1};
    static bool writeTicks(std::uint32_t high_ticks) {
      return Selected<index>::template writeTicks<channel>(high_ticks);
    }
    static bool write(std::uint16_t numerator, std::uint16_t denominator) {
      return Selected<index>::template write<channel>(numerator,denominator);
    }
  };
};

template <typename Bindings, std::uint32_t Clock, typename... Instances>
using Program = Allocation<Bindings,Clock,Reservations<>,Instances...>;

} // namespace timer_prototype::atmega328p
