#include "api.hpp"

extern "C" void grevir_mock_TIMER1_OVF_vect() noexcept;

int main() {
  grevir_mock_TIMER1_OVF_vect();
  return MotorInstance::ticks == 1 ? 0 : 1;
}
