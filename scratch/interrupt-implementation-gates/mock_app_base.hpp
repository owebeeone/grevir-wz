#pragma once

#include <grevir/test/host_start_policy.hpp>
#include <grevir/core/allocated_application.hpp>
#include <grevir/interrupt/binding.hpp>
#include <grevir/interrupt/install.hpp>
#include <grevir/peripherals/timer/interrupt_allocator.hpp>
#include <grevir/test/interrupt_controller.hpp>

namespace mock_app {
namespace irq = grevir::interrupt;

using TimerKey = irq::EventKey<"motor", "timer", "period">;
struct PeriodElapsed { using Key = TimerKey; };
struct TimerRequest { using InterruptEvents = setl::TypeArgs<PeriodElapsed>; };
template <class>
struct MotorModule : ardo::ModuleBase<ardo::Parameters<>> {};
using Motor = grevir::RequestedModule<setl::TypeArgs<TimerRequest>, MotorModule>;

inline grevir::test::InterruptController controller{};
inline unsigned ticks = 0;
inline bool reraising = false;
inline bool configure_ok = true;
inline bool pending_ok = true;
inline bool cleanup_ok = true;
inline void (*configure_hook)() noexcept = nullptr;

struct Board {
  template <class Spec>
  using StartPolicy = grevir::test::HostStartPolicy<Spec>;
  inline static constexpr auto backend = irq::literal("mock");
  inline static constexpr auto target = irq::literal("mock_mcu");
  inline static constexpr auto board = irq::literal("mock_board");
  inline static constexpr auto compiler = irq::literal("scratch_compiler");
  inline static constexpr auto application = irq::literal("mock_application");

  template <class Spec>
  struct Allocate {
    inline static constexpr auto problem = [] {
      grevir::timer::Problem<1, 2> problem{};
      problem.requests[0] = {irq::identity<TimerKey>(), true};
      auto& pwm = problem.candidates[0];
      pwm.request = irq::identity<TimerKey>();
      pwm.configuration = irq::literal("pwm_only");
      pwm.timer = irq::literal("mock_timer0");
      pwm.owner = irq::literal("motor");
      pwm.pwm = true;
      auto& combined = problem.candidates[1];
      combined.request = irq::identity<TimerKey>();
      #if defined(GREVIR_TEST_DIFFERENT_PLAN)
      combined.configuration = irq::literal("pwm_period_alt");
      #else
      combined.configuration = irq::literal("pwm_period");
      #endif
      combined.timer = irq::literal("mock_timer0");
      combined.owner = irq::literal("motor");
      combined.source = irq::literal("mock_timer0");
      combined.selector = irq::literal("overflow");
      combined.entry = irq::literal("mock_timer0_entry");
      combined.snapshot_policy = irq::literal("mock_status");
      combined.acknowledge_policy = irq::literal("mock_ack");
      combined.preference = 1;
      combined.pwm = true;
      combined.period_event = true;
      return problem;
    }();
    inline static constexpr auto plan =
      grevir::timer::solve(problem, irq::DemandSet<Spec>::value).interrupts;
    template <class Demand, class Selected>
    static consteval bool validates(const Demand& demands, const Selected& selected) {
      return grevir::timer::validates<Spec>(problem, demands, selected);
    }
  };

  template <class Source>
  static bool install(void (*callback)() noexcept) noexcept {
    static_assert(Source::value.view() == irq::literal("mock_timer0"));
    return controller.install(callback);
  }

  static void mask_owned() noexcept { controller.mask(); }
  template <class Spec>
  static bool configure() noexcept {
    if (configure_hook != nullptr) { configure_hook(); }
    return configure_ok;
  }
  static void setup_modules() noexcept {}
  static bool settle_pending() noexcept { return pending_ok; }
  template <class Spec>
  static void enable_owned() noexcept {
    if constexpr (irq::BindingPlan<Spec>::selected.count != 0) {
      controller.enable();
    }
  }
  static bool cleanup() noexcept {
    controller.mask();
    if (!cleanup_ok) { return false; }
    controller.reset();
    return true;
  }
};
} // namespace mock_app

using GrevirApplication = grevir::ApplicationSpec<mock_app::Board, mock_app::Motor>;

#include <grevir/interrupt/handler.hpp>
