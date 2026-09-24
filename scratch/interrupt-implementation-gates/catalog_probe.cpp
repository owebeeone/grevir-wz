#include <grevir/interrupt/catalog.hpp>
#include <grevir/core/allocated_application.hpp>

using MotorKey = grevir::interrupt::EventKey<"motor", "timer", "period">;
using FanKey = grevir::interrupt::EventKey<"fan", "timer", "period">;
struct MotorEvent { using Key = MotorKey; };
struct FanEvent { using Key = FanKey; };
struct MotorRequest { using InterruptEvents = setl::TypeArgs<MotorEvent>; };
struct FanRequest { using InterruptEvents = setl::TypeArgs<FanEvent>; };
struct NoEventRequest {};
template <class>
struct EmptyModule : ardo::ModuleBase<ardo::Parameters<>> {};
using Motor = grevir::RequestedModule<setl::TypeArgs<MotorRequest, NoEventRequest>, EmptyModule>;
using Fan = grevir::RequestedModule<setl::TypeArgs<FanRequest>, EmptyModule,
  ardo::ResourceClaim<>, Motor>;
struct Board {};
using Forward = grevir::interrupt::EventCatalog<
  grevir::interrupt::ApplicationSpec<Board, Motor, Fan>>;
using Reverse = grevir::interrupt::EventCatalog<
  grevir::interrupt::ApplicationSpec<Board, Fan, Motor>>;

static_assert(std::is_same_v<Forward::ByKey<MotorKey>, MotorEvent>);
static_assert(std::is_same_v<Forward::ByKey<FanKey>, FanEvent>);
static_assert(std::is_same_v<Forward::ByKey<grevir::interrupt::EventKey<"none", "x", "y">>, void>);
static_assert(Forward::keys.size() == 2);
static_assert(Forward::keys[0] == Reverse::keys[0]);
static_assert(Forward::keys[1] == Reverse::keys[1]);
static_assert(Forward::keys[0] == grevir::interrupt::identity<FanKey>());
static_assert(Forward::keys[1] == grevir::interrupt::identity<MotorKey>());
