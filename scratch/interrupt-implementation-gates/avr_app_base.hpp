#pragma once

#include <grevir/core/allocated_application.hpp>
#include <grevir/interrupt/binding.hpp>
#include <grevir/interrupt/install.hpp>
#include <grevir/interrupt/single_thread_start_policy.hpp>
#include <grevir/peripherals/timer/interrupt_allocator.hpp>
#include <grevir/avr/devices/atmega328p/interrupt_timer1.hpp>

namespace avr_app {
namespace irq = grevir::interrupt;

using TimerKey = irq::EventKey<"motor", "timer", "period">;
struct PeriodElapsed { using Key = TimerKey; };
struct TimerRequest { using InterruptEvents = setl::TypeArgs<PeriodElapsed>; };
template <class>
struct MotorModule : ardo::ModuleBase<ardo::Parameters<>> {};
using Motor = grevir::RequestedModule<setl::TypeArgs<TimerRequest>, MotorModule>;

extern "C" volatile unsigned char grevir_irq_test_ticks;

struct Board {
  template <class Spec>
  using StartPolicy = irq::SingleThreadStartPolicy<Spec>;
  inline static constexpr auto backend = irq::literal("avr");
  inline static constexpr auto target = irq::literal("atmega328p");
  inline static constexpr auto board = irq::literal("uno");
  inline static constexpr auto compiler = irq::literal("avr_gcc14");
  inline static constexpr auto application = irq::literal("avr_timer1_example");

  template <class Spec>
  struct Allocate {
    inline static constexpr auto problem = [] {
      grevir::timer::Problem<1, 2> value{};
#if defined(GREVIR_TEST_AVR_PWM_REQUIRED)
      value.requests[0] = {irq::EventCatalog<Spec>::keys[0], true};
#else
      value.requests[0] = {irq::EventCatalog<Spec>::keys[0], false};
#endif
      auto& normal = value.candidates[0];
      normal.request = value.requests[0].event;
      normal.configuration = irq::literal("normal_period");
      normal.timer = irq::literal("timer1");
      normal.owner = irq::literal("motor");
      normal.source = irq::literal("timer1_ovf");
      normal.selector = irq::literal("overflow");
      normal.entry = irq::literal("timer1_ovf_vect");
      normal.snapshot_policy = irq::literal("timer1_status");
      normal.acknowledge_policy = irq::literal("timer1_auto_ack");
      normal.period_event = true;
      auto& combined = value.candidates[1];
      combined = normal;
      combined.configuration = irq::literal("fast_pwm8_period");
      combined.pwm = true;
      combined.preference = 1;
      return value;
    }();
    inline static constexpr auto solution =
      grevir::timer::solve(problem, irq::DemandSet<Spec>::value);
    inline static constexpr auto plan = solution.interrupts;
    template <class Demand, class Selected>
    static consteval bool validates(const Demand& demands, const Selected& selected) {
      return grevir::timer::validates<Spec>(problem, demands, selected);
    }
  };

  static void mask_owned() noexcept {
    grevir::avr::atmega328p::Timer1Overflow::mask();
    TCCR1B &= static_cast<unsigned char>(~(_BV(CS12) | _BV(CS11) | _BV(CS10)));
  }

  template <class Spec>
  static bool configure() noexcept {
    constexpr auto config = Allocate<Spec>::solution.configurations[0];
    static_assert(config == irq::literal("normal_period")
                  || config == irq::literal("fast_pwm8_period"));
    TCCR1A = config == irq::literal("fast_pwm8_period") ? _BV(WGM10) : 0;
    TCCR1B = config == irq::literal("fast_pwm8_period") ? _BV(WGM12) : 0;
    TCNT1 = config == irq::literal("fast_pwm8_period") ? 0xfeu : 0xfffeu;
    return true;
  }

  static void setup_modules() noexcept {}

  template <class Source>
  static bool install() noexcept {
    static_assert(Source::value.view() == irq::literal("timer1_ovf"));
    return true;
  }

  // Preserve a pre-existing overflow; enabling TOIE1 delivers it.
  static bool settle_pending() noexcept { return true; }

  template <class Spec>
  static void enable_owned() noexcept {
    TCCR1B |= _BV(CS10);
    if constexpr (irq::BindingPlan<Spec>::selected.count != 0) {
      grevir::avr::atmega328p::Timer1Overflow::enable_preserving_pending();
      sei();
    }
  }

  static bool cleanup() noexcept {
    mask_owned();
    return true;
  }
};
} // namespace avr_app

using GrevirApplication = grevir::ApplicationSpec<avr_app::Board, avr_app::Motor>;

#include <grevir/interrupt/handler.hpp>
