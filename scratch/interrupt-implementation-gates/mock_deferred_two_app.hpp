#pragma once

#include <grevir/core/allocated_application.hpp>
#include <grevir/interrupt/binding.hpp>
#include <grevir/interrupt/install.hpp>
#include <grevir/peripherals/timer/interrupt_allocator.hpp>
#include <grevir/test/event_lock.hpp>
#include <grevir/test/host_start_policy.hpp>
#include <grevir/test/interrupt_controller.hpp>

namespace mock_deferred_two {
namespace irq = grevir::interrupt;

struct A { using Key = irq::EventKey<"a", "timer", "period">; };
struct B { using Key = irq::EventKey<"b", "timer", "period">; };
struct RequestA { using InterruptEvents = setl::TypeArgs<A>; };
struct RequestB { using InterruptEvents = setl::TypeArgs<B>; };
template <class>
struct ModuleA : ardo::ModuleBase<ardo::Parameters<>> {};
template <class>
struct ModuleB : ardo::ModuleBase<ardo::Parameters<>> {};
using OwnerA = grevir::RequestedModule<setl::TypeArgs<RequestA>, ModuleA>;
using OwnerB = grevir::RequestedModule<setl::TypeArgs<RequestB>, ModuleB>;

struct AlternateEventLock : grevir::test::EventLock {
  inline static constexpr std::string_view identity{"host_mutex_v2", 13};
};

inline grevir::test::InterruptController controller_a{};
inline grevir::test::InterruptController controller_b{};
inline unsigned calls_a = 0;
inline unsigned calls_b = 0;

struct Board {
#if defined(GREVIR_TEST_STALE_POLICY)
  using EventLock = AlternateEventLock;
#else
  using EventLock = grevir::test::EventLock;
#endif
#if defined(GREVIR_TEST_CAPACITY_VALUE)
  inline static constexpr auto event_queue_capacity = GREVIR_TEST_CAPACITY_VALUE;
#elif defined(GREVIR_TEST_STALE_CAPACITY)
  inline static constexpr unsigned event_queue_capacity = 1;
#else
  inline static constexpr unsigned event_queue_capacity = 2;
#endif
  template <class Spec>
  using StartPolicy = grevir::test::HostStartPolicy<Spec>;
  inline static constexpr auto backend = irq::literal("mock");
  inline static constexpr auto target = irq::literal("mock_mcu");
  inline static constexpr auto board = irq::literal("mock_board");
  inline static constexpr auto compiler = irq::literal("scratch_compiler");
  inline static constexpr auto application = irq::literal("two_deferred_events");

  template <class Spec>
  struct Allocate {
    inline static constexpr auto problem = [] {
      grevir::timer::Problem<2, 2> value{};
      value.requests[0] = {irq::identity<A::Key>(), false};
      value.requests[1] = {irq::identity<B::Key>(), false};
      auto& first = value.candidates[0];
      first.request = value.requests[0].event;
      first.configuration = irq::literal("periodic");
      first.timer = irq::literal("timer_a");
      first.owner = irq::literal("a");
      first.source = irq::literal("source_a");
      first.selector = irq::literal("overflow");
      first.entry = irq::literal("entry_a");
      first.snapshot_policy = irq::literal("status");
      first.acknowledge_policy = irq::literal("ack");
      first.period_event = true;
      auto& second = value.candidates[1];
      second.request = value.requests[1].event;
      second.configuration = irq::literal("periodic");
      second.timer = irq::literal("timer_b");
      second.owner = irq::literal("b");
      second.source = irq::literal("source_b");
      second.selector = irq::literal("overflow");
      second.entry = irq::literal("entry_b");
      second.snapshot_policy = irq::literal("status");
      second.acknowledge_policy = irq::literal("ack");
      second.period_event = true;
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
    controller_a.mask();
    controller_b.mask();
  }
  template <class Spec>
  static bool configure() noexcept { return true; }
  static void setup_modules() noexcept {}
  template <class Source>
  static bool install(void (*callback)() noexcept) noexcept {
    if constexpr (Source::value.view() == irq::literal("source_a")) {
      return controller_a.install(callback);
    } else {
      static_assert(Source::value.view() == irq::literal("source_b"));
      return controller_b.install(callback);
    }
  }
  static bool settle_pending() noexcept { return true; }
  template <class Spec>
  static void enable_owned() noexcept {
    controller_a.enable();
    controller_b.enable();
  }
  static bool cleanup() noexcept {
    controller_a.reset();
    controller_b.reset();
    return true;
  }
};
} // namespace mock_deferred_two

using GrevirApplication = grevir::ApplicationSpec<mock_deferred_two::Board,
  mock_deferred_two::OwnerA, mock_deferred_two::OwnerB>;

#include <grevir/interrupt/handler.hpp>

#if defined(GREVIR_TEST_STREAM_B)
template <>
struct grevir::event::RouteFor<mock_deferred_two::B> {
  using Context = grevir::event::MainLoop;
  using Delivery = grevir::event::Stream;
};
#endif

template <>
inline void grevir::on_event<mock_deferred_two::A>() noexcept {
  ++mock_deferred_two::calls_a;
}

#if !defined(GREVIR_IRQ_PROBE) || !defined(GREVIR_TEST_PROBE_ONE_EVENT)
template <>
inline void grevir::on_event<mock_deferred_two::B>() noexcept {
  ++mock_deferred_two::calls_b;
}
#endif
