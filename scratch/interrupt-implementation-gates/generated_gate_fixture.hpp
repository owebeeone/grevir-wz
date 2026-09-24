#pragma once

namespace grevir::interrupt::detail {
template <>
struct BoundEventKey<MotorEventKey> {
  using Event = MotorInstance::Timer::PeriodElapsed;
  static constexpr unsigned id = 1;
};
}
