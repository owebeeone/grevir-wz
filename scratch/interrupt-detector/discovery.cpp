// Host-only probe for discovering a visible handler via an unresolved symbol.
// The generated discovery translation unit is the current full-cycle probe.
#define GREVIR_IRQ_DISCOVERY 1
#include "api.hpp"
#include "handler.hpp"

struct Timer1OverflowBinding {};

template <class Event>
concept HasHandler = requires { on_interrupt<Event>(); };

static_assert(HasHandler<MotorInstance::Timer::PeriodElapsed>);
static_assert(!HasHandler<MappedButNoHandler>);

// Deliberately declared but undefined. The object file records the selected
// event and binding as an undefined, mangled C++ symbol.
template <class Event, class Binding>
void grevir_irq_request() noexcept;

int main() {
  if constexpr (HasHandler<MotorInstance::Timer::PeriodElapsed>) {
    grevir_irq_request<MotorInstance::Timer::PeriodElapsed,
                       Timer1OverflowBinding>();
  }
}
