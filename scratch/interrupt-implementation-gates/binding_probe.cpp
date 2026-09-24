#include "demand_probe.cpp"
#include <grevir/interrupt/binding.hpp>

namespace irq = grevir::interrupt;

consteval auto one_binding() {
  irq::SelectedPlan<2> plan{};
  plan.allocation_ok = true;
  plan.count = 1;
  plan.bindings[0] = {irq::identity<MotorKey>(), irq::literal("motor"),
    irq::literal("pwm_period"),
    irq::literal("timer1_ovf"), irq::literal("overflow"),
    irq::literal("timer1_ovf_vect"), irq::literal("timer1_status"),
    irq::literal("timer1_w1c"), 0, false};
  return plan;
}

constexpr auto demands = irq::DemandSet<MotorFirst>::value;
constexpr auto catalog = irq::EventCatalog<MotorFirst>::keys;
static_assert(irq::validate(catalog, demands, one_binding()) == irq::PlanError::none);

consteval auto duplicate() {
  auto plan = one_binding();
  plan.count = 2;
  plan.bindings[1] = plan.bindings[0];
  return plan;
}
static_assert(irq::validate(catalog, demands, duplicate()) == irq::PlanError::count_mismatch);

consteval auto wrong_event() {
  auto plan = one_binding();
  plan.bindings[0].event = irq::identity<FanKey>();
  return plan;
}
static_assert(irq::validate(catalog, demands, wrong_event()) == irq::PlanError::unknown_event);

consteval auto bad_source() {
  auto plan = one_binding();
  plan.bindings[0].source = irq::literal("timer.1");
  return plan;
}
static_assert(irq::validate(catalog, demands, bad_source()) == irq::PlanError::invalid_source);
