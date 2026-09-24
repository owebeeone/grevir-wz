#pragma once

#include "api.hpp"

template <>
inline void on_interrupt<MotorInstance::Timer::PeriodElapsed>() noexcept {
  MotorInstance::tick();
}
