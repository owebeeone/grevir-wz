#include "mock_app.hpp"

namespace irq = grevir::interrupt;
using Spec = GrevirApplication;
using Allocation = mock_app::Board::Allocate<Spec>;

static_assert(grevir::timer::validates<Spec>(
  Allocation::problem, irq::DemandSet<Spec>::value, Allocation::plan));

consteval auto foreign_owner() {
  auto value = Allocation::plan;
  value.bindings[0].owner = irq::literal("foreign");
  return value;
}
static_assert(!grevir::timer::validates<Spec>(
  Allocation::problem, irq::DemandSet<Spec>::value, foreign_owner()));
static_assert(!irq::validate_inventory(irq::EventCatalog<Spec>::keys,
  Allocation::problem, foreign_owner()));

consteval auto unoffered_configuration() {
  auto value = Allocation::plan;
  value.bindings[0].configuration = irq::literal("not_offered");
  return value;
}
static_assert(!grevir::timer::validates<Spec>(
  Allocation::problem, irq::DemandSet<Spec>::value, unoffered_configuration()));
static_assert(!irq::validate_inventory(irq::EventCatalog<Spec>::keys,
  Allocation::problem, unoffered_configuration()));

consteval auto unoffered_selector() {
  auto value = Allocation::plan;
  value.bindings[0].selector = irq::literal("not_offered");
  return value;
}
static_assert(!grevir::timer::validates<Spec>(
  Allocation::problem, irq::DemandSet<Spec>::value, unoffered_selector()));
static_assert(!irq::validate_inventory(irq::EventCatalog<Spec>::keys,
  Allocation::problem, unoffered_selector()));

consteval auto foreign_request() {
  auto value = Allocation::problem;
  value.requests[0].event = irq::identity<irq::EventKey<"other", "timer", "period">>();
  return value;
}
static_assert(!grevir::timer::validates<Spec>(
  foreign_request(), irq::DemandSet<Spec>::value, Allocation::plan));
static_assert(!irq::validate_inventory(irq::EventCatalog<Spec>::keys,
  foreign_request(), Allocation::plan));

consteval auto foreign_candidate_owner() {
  auto value = Allocation::problem;
  value.candidates[1].owner = irq::literal("alien");
  return value;
}
static_assert(!irq::validate_inventory(irq::EventCatalog<Spec>::keys,
  foreign_candidate_owner(), Allocation::plan));

constexpr auto shared = [] {
  auto value = Allocation::plan;
  value.bindings[0].shared_source = true;
  return value;
}();
static_assert(irq::validate(irq::EventCatalog<Spec>::keys,
  irq::DemandSet<Spec>::value, shared) == irq::PlanError::unsupported_shared_source);
