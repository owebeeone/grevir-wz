#include <grevir/peripherals/timer/interrupt_allocator.hpp>

namespace irq = grevir::interrupt;
namespace timer = grevir::timer;

using A = irq::EventKey<"alpha", "timer", "period">;
using B = irq::EventKey<"beta", "timer", "period">;

consteval auto problem(bool reverse) {
  timer::Problem<2, 3> input{};
  input.requests[0] = {irq::identity<A>(), true};
  input.requests[1] = {irq::identity<B>(), false};
  input.candidates[0] = {irq::identity<A>(), irq::literal("pwm_only"),
    irq::literal("timer0"), irq::literal("alpha"), {}, {}, {}, {}, {},
    0, true, false, false};
  input.candidates[1] = {irq::identity<A>(), irq::literal("pwm_period"),
    irq::literal("timer0"), irq::literal("alpha"), irq::literal("source0"),
    irq::literal("overflow"), irq::literal("entry0"), irq::literal("status0"),
    irq::literal("ack0"), 1, true, true, false};
  input.candidates[2] = {irq::identity<B>(), irq::literal("period"),
    irq::literal("timer1"), irq::literal("beta"), irq::literal("source1"),
    irq::literal("overflow"), irq::literal("entry1"), irq::literal("status1"),
    irq::literal("ack1"), 0, false, true, false};
  if (reverse) {
    const auto first = input.requests[0];
    input.requests[0] = input.requests[1];
    input.requests[1] = first;
    const auto candidate = input.candidates[0];
    input.candidates[0] = input.candidates[2];
    input.candidates[2] = candidate;
  }
  return input;
}

consteval auto demands() {
  irq::DemandSummary<2> value{};
  value.keys[0] = irq::identity<A>();
  value.keys[1] = irq::identity<B>();
  value.count = 2;
  return value;
}

constexpr auto plan = timer::solve(problem(false), demands());
constexpr auto reordered = timer::solve(problem(true), demands());
static_assert(plan.error == timer::Error::none);
static_assert(plan.interrupts.count == 2);
static_assert(plan.interrupts.bindings[0] == reordered.interrupts.bindings[0]);
static_assert(plan.interrupts.bindings[1] == reordered.interrupts.bindings[1]);
static_assert(plan.configurations[0] == irq::literal("pwm_period"));

constexpr auto no_budget = timer::solve(problem(false), demands(), 0);
static_assert(no_budget.error == timer::Error::exhausted);

consteval auto conflict_problem() {
  auto input = problem(false);
  input.candidates[2].timer = irq::literal("timer0");
  return input;
}
static_assert(timer::solve(conflict_problem(), demands()).error == timer::Error::conflict);

consteval auto reserved_problem() {
  auto input = problem(false);
  input.candidates[1].reserved = true;
  return input;
}
static_assert(timer::solve(reserved_problem(), demands()).error == timer::Error::no_candidate);

consteval auto separated_duplicate_problem() {
  timer::Problem<1, 3> input{};
  input.requests[0] = {irq::identity<A>(), true};
  const auto original = problem(false);
  input.candidates[0] = original.candidates[0];
  input.candidates[1] = original.candidates[1];
  input.candidates[2] = original.candidates[0];
  input.candidates[2].preference = 2;
  return input;
}
static_assert(timer::solve(separated_duplicate_problem(), demands()).error
  == timer::Error::duplicate_candidate);
