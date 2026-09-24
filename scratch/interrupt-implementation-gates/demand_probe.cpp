#include "catalog_probe.cpp"
#include <grevir/interrupt/demand.hpp>

template <>
inline void grevir::on_interrupt<MotorEvent>() noexcept {}

using MotorFirst = grevir::interrupt::ApplicationSpec<Board, Motor, Fan>;
using FanFirst = grevir::interrupt::ApplicationSpec<Board, Fan, Motor>;
static_assert(grevir::interrupt::DemandSet<MotorFirst>::value.count == 1);
static_assert(grevir::interrupt::DemandSet<FanFirst>::value.count == 1);
static_assert(grevir::interrupt::DemandSet<MotorFirst>::value.keys[0]
  == grevir::interrupt::identity<MotorKey>());
static_assert(grevir::interrupt::DemandSet<MotorFirst>::value.keys[0]
  == grevir::interrupt::DemandSet<FanFirst>::value.keys[0]);
