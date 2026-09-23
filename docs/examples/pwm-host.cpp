#include <GrevirAVR.h>
#include <grevir/avr/devices/atmega328p/timers.hpp>
#include <grevir/avr/devices/atmega328p/pwm_backend.hpp>
#include <array>
#include <cstring>

struct Memory {
  inline static std::array<unsigned char, 256> bytes{};

  template <typename T>
  static T read(std::ptrdiff_t address) {
    T value{};
    std::memcpy(&value, bytes.data() + address, sizeof(value));
    return value;
  }

  template <typename T>
  static void write(std::ptrdiff_t address, T value) {
    std::memcpy(bytes.data() + address, &value, sizeof(value));
  }

  template <typename T>
  static void modify(std::ptrdiff_t address, T value, T mask) {
    write<T>(address, static_cast<T>((read<T>(address) & ~mask) | (value & mask)));
  }
};

struct Barrier {
  Barrier() = default;
  ~Barrier() = default;
};

using Device = ardo::sys::avr::arch_atmega328p::TimerBindings<Memory, Barrier>;
namespace p = grevir::pwm;
namespace avr_pwm = p::atmega328p;

using MotorPwm = p::Instance<"motor", p::PwmRequest<"pwm",
  p::Frequency<p::Hertz<1000>, p::Exact>,
  p::DutyStepAtMost<1, 256>,
  p::For<p::Target::avr, p::Pin<avr_pwm::PB1>, p::avr::TopFromIcr>,
  p::For<p::Target::esp32, p::Pin<18>, p::esp32::ApbClock>>>;

template <typename Allocation>
struct Motor : ardo::ModuleBase<
    ardo::Parameters<typename Allocation::template Pwm<"motor">>> {
  using Output = typename Allocation::template Pwm<"motor">;
  static void runSetup() {
    Output::write(1, 4);
  }
};

using MotorModule = grevir::RequestedModule<setl::TypeArgs<MotorPwm>, Motor>;
using App = grevir::AllocatedApplication<
  avr_pwm::Backend<Device, 16'000'000>, MotorModule>;

int main() {
  App::runSetup();
  return Memory::read<std::uint16_t>(0x86) == 15999
    && Memory::read<std::uint16_t>(0x88) == 3999 ? 0 : 1;
}
