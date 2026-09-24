#pragma once

#include "api.hpp"

// Stand-in for the generated binding. This must be visible before handler.hpp.
template <>
struct DetectorImpl<MotorInstance::Timer::PeriodElapsed> {
  static constexpr unsigned binding_id = 13u;
};

template <>
struct DetectorImpl<MappedButNoHandler> {
  static constexpr unsigned binding_id = 14u;
};
