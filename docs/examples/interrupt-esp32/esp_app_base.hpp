#pragma once

#include <grevir/core/allocated_application.hpp>
#include <grevir/interrupt/binding.hpp>
#include <grevir/interrupt/install.hpp>
#include <grevir/peripherals/timer/interrupt_allocator.hpp>
#include <grevir/arduino_esp32/timer_group0_timer0.hpp>

namespace esp_app {
namespace irq = grevir::interrupt;

using TimerKey = irq::EventKey<"motor", "timer", "period">;
struct PeriodElapsed { using Key = TimerKey; };
struct TimerRequest { using InterruptEvents = setl::TypeArgs<PeriodElapsed>; };
template <class>
struct MotorModule : ardo::ModuleBase<ardo::Parameters<>> {};
using Motor = grevir::RequestedModule<setl::TypeArgs<TimerRequest>, MotorModule>;

inline volatile unsigned ticks = 0;

struct Board {
  template <class Spec>
  using StartPolicy = grevir::arduino_esp32::TimerStartPolicy<Spec>;
  inline static constexpr auto backend = irq::literal("esp32");
  inline static constexpr auto target = irq::literal("esp32_classic");
  inline static constexpr auto board = irq::literal("esp32_dev_module");
  inline static constexpr auto compiler = irq::literal("esp_x32_2601");
  inline static constexpr auto application = irq::literal("esp_timer_example");

  template <class Spec>
  struct Allocate {
    inline static constexpr auto problem = [] {
      grevir::timer::Problem<1, 1> value{};
      value.requests[0] = {irq::EventCatalog<Spec>::keys[0], false};
      auto& candidate = value.candidates[0];
      candidate.request = value.requests[0].event;
      candidate.configuration = irq::literal("period_1ms_div80");
      candidate.timer = irq::literal("tg0_t0");
      candidate.owner = irq::literal("motor");
      candidate.source = irq::literal("tg0_t0_alarm");
      candidate.selector = irq::literal("alarm");
      candidate.entry = irq::literal("tg0_t0_intr_alloc");
      candidate.snapshot_policy = irq::literal("tg0_t0_status");
      candidate.acknowledge_policy = irq::literal("tg0_t0_clear_rearm");
      candidate.period_event = true;
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
    grevir::arduino_esp32::TimerGroup0Timer0::mask();
  }

  template <class Spec>
  static bool configure() noexcept {
    constexpr auto config = Allocate<Spec>::solution.configurations[0];
    static_assert(config == irq::literal("period_1ms_div80"));
    return grevir::arduino_esp32::TimerGroup0Timer0::configure_periodic(80, 1000);
  }

  static void setup_modules() noexcept {}

  template <class Source>
  static bool install(void (*callback)() noexcept) noexcept {
    static_assert(Source::value.view() == irq::literal("tg0_t0_alarm"));
    return grevir::arduino_esp32::TimerGroup0Timer0::install(callback);
  }

  static bool settle_pending() noexcept { return true; }

  template <class Spec>
  static void enable_owned() noexcept {
    if constexpr (irq::BindingPlan<Spec>::selected.count != 0) {
      grevir::arduino_esp32::TimerGroup0Timer0::enable_preserving_pending();
    } else {
      grevir::arduino_esp32::TimerGroup0Timer0::start_without_interrupt();
    }
  }

  static bool cleanup() noexcept {
    return grevir::arduino_esp32::TimerGroup0Timer0::cleanup();
  }
};
} // namespace esp_app

using GrevirApplication = grevir::ApplicationSpec<esp_app::Board, esp_app::Motor>;

#include <grevir/interrupt/handler.hpp>
