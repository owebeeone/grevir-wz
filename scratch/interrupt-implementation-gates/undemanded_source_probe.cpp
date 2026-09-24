#include <grevir/peripherals/timer/interrupt_allocator.hpp>

namespace irq = grevir::interrupt;

constexpr auto first = irq::identity<irq::EventKey<"a", "timer", "period">>();
constexpr auto second = irq::identity<irq::EventKey<"b", "timer", "period">>();

constexpr auto problem = [] {
  grevir::timer::Problem<2, 2> value{};
  value.requests[0] = {first, false};
  value.requests[1] = {second, false};
  for (unsigned index = 0; index < 2; ++index) {
    auto& candidate = value.candidates[index];
    candidate.request = index == 0 ? first : second;
    candidate.configuration = irq::literal("period");
    candidate.timer = index == 0 ? irq::literal("timer_a") : irq::literal("timer_b");
    candidate.owner = index == 0 ? irq::literal("a") : irq::literal("b");
    candidate.source = irq::literal("shared_source");
    candidate.selector = irq::literal("overflow");
    candidate.entry = irq::literal("shared_entry");
    candidate.snapshot_policy = irq::literal("snapshot");
    candidate.acknowledge_policy = irq::literal("ack");
    candidate.period_event = true;
  }
  return value;
}();

constexpr irq::DemandSummary<2> one_demand{{{second}}, 1};
constexpr auto one = grevir::timer::solve(problem, one_demand);
static_assert(one.error == grevir::timer::Error::none);
static_assert(one.interrupts.count == 1);
static_assert(one.interrupts.bindings[0].event == second);

constexpr irq::DemandSummary<2> both_demands{{{first, second}}, 2};
constexpr auto both = grevir::timer::solve(problem, both_demands);
static_assert(both.error == grevir::timer::Error::conflict);
