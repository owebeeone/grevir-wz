#pragma once

#include <grevir/core/allocated_application.hpp>
#include <grevir/interrupt/binding.hpp>
#include <grevir/interrupt/install.hpp>
#include <grevir/peripherals/timer/interrupt_allocator.hpp>
#include <grevir/test/host_start_policy.hpp>
#include <grevir/test/interrupt_controller.hpp>

namespace example {
namespace irq = grevir::interrupt;

using TimerKey = irq::EventKey<"motor", "timer", "period">;
struct PeriodElapsed { using Key = TimerKey; };
struct TimerRequest { using InterruptEvents = setl::TypeArgs<PeriodElapsed>; };
template <class>
struct MotorModule : ardo::ModuleBase<ardo::Parameters<>> {};
using Motor = grevir::RequestedModule<setl::TypeArgs<TimerRequest>, MotorModule>;

inline grevir::test::InterruptController controller{};
inline unsigned ticks = 0;

struct Board {
  template <class Spec>
  using StartPolicy = grevir::test::HostStartPolicy<Spec>;
  inline static constexpr auto backend = irq::literal("mock");
  inline static constexpr auto target = irq::literal("mock_mcu");
  inline static constexpr auto board = irq::literal("mock_board");
  inline static constexpr auto compiler = irq::literal("native_cxx23");
  inline static constexpr auto application = irq::literal("interrupt_mock_example");

  template <class Spec>
  struct Allocate {
    inline static constexpr auto problem = [] {
      grevir::timer::Problem<1, 1> value{};
      value.requests[0] = {irq::EventCatalog<Spec>::keys[0], false};
      auto& candidate = value.candidates[0];
      candidate.request = value.requests[0].event;
      candidate.configuration = irq::literal("periodic");
      candidate.timer = irq::literal("mock_timer");
      candidate.owner = irq::literal("motor");
      candidate.source = irq::literal("mock_timer");
      candidate.selector = irq::literal("overflow");
      candidate.entry = irq::literal("mock_entry");
      candidate.snapshot_policy = irq::literal("mock_status");
      candidate.acknowledge_policy = irq::literal("mock_ack");
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

  static void mask_owned() noexcept { controller.mask(); }
  template <class Spec>
  static bool configure() noexcept {
    static_assert(Allocate<Spec>::solution.configurations[0]
                  == irq::literal("periodic"));
    return true;
  }
  static void setup_modules() noexcept {}
  template <class Source>
  static bool install(void (*callback)() noexcept) noexcept {
    static_assert(Source::value.view() == irq::literal("mock_timer"));
    return controller.install(callback);
  }
  static bool settle_pending() noexcept { return true; }
  template <class Spec>
  static void enable_owned() noexcept {
    if constexpr (irq::BindingPlan<Spec>::selected.count != 0) {
      controller.enable();
    }
  }
  static bool cleanup() noexcept {
    controller.reset();
    return true;
  }
};
} // namespace example

using GrevirApplication = grevir::ApplicationSpec<example::Board, example::Motor>;

#include <grevir/interrupt/handler.hpp>

template <>
inline void grevir::on_interrupt<example::PeriodElapsed>() noexcept {
  ++example::ticks;
}
