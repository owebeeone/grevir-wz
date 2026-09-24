#include "generated_detector.hpp"
#include "handler.hpp"

template <class Event>
concept HasHandler = requires { on_interrupt<Event>(); };

static_assert(HasHandler<MotorInstance::Timer::PeriodElapsed>);
static_assert(!HasHandler<MappedButNoHandler>);

int main() {
  on_interrupt<MotorInstance::Timer::PeriodElapsed>();
  return MotorInstance::ticks == 1 ? 0 : 1;
}
