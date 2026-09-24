#include <grevir/avr/devices/atmega328p/pwm_backend.hpp>
#include <grevir/test/register_memory.hpp>
#include "../../grevir-avr/tests/atmega328p_fixture.hpp"

struct MotorEventKey {};

struct MotorInstance {
  struct Timer {
    struct PeriodElapsed { using Key = MotorEventKey; };
  };
  static void tick() noexcept;
};

#if !defined(GREVIR_IRQ_PROBE)
#define GREVIR_GENERATED_IRQ_HEADER "generated_gate_fixture.hpp"
#endif
#include <grevir/interrupt/handler.hpp>

template <>
inline void grevir::on_interrupt<MotorInstance::Timer::PeriodElapsed>() noexcept {
  MotorInstance::tick();
}

template <class Event>
inline constexpr bool handler_present = requires { grevir::on_interrupt<Event>(); };

struct AbsentEvent {};
#if defined(GREVIR_IRQ_PROBE)
static_assert(handler_present<MotorInstance::Timer::PeriodElapsed>);
static_assert(!handler_present<AbsentEvent>);
#endif

namespace pwm = grevir::pwm;
namespace atmega = grevir::pwm::atmega328p;
using MotorRequest = pwm::Instance<"motor", pwm::PwmRequest<"pwm",
  pwm::Frequency<pwm::Hertz<1000>, pwm::Exact>,
  pwm::DutyStepAtMost<1, 256>,
  pwm::For<pwm::Target::avr, pwm::Pin<atmega::PB1>, pwm::avr::TopFromIcr>>>;

template <class Plan>
struct MotorModule : ardo::ModuleBase<ardo::Parameters<typename Plan::template Pwm<"motor">>> {
  static void runSetup() {}
  static void runLoop() {}
};

using Descriptor = grevir::RequestedModule<setl::TypeArgs<MotorRequest>, MotorModule>;
using Backend = atmega::Backend<atmega328p_mock::Bindings, 16'000'000>;
using Application = grevir::AllocatedApplication<Backend, Descriptor>;

// The handler body was parsed above, before Application and its binding existed.
// Only this facade definition needs the selected allocation.
void MotorInstance::tick() noexcept {
  using Pwm = Application::Allocation::Pwm<"motor">;
  (void)Pwm::write(1, 2);
}

int main() {
  atmega328p_mock::Memory::reset();
  Application::runSetup();
  grevir::on_interrupt<MotorInstance::Timer::PeriodElapsed>();
  return atmega328p_mock::word(0x88) == 7999 ? 0 : 1;
}
