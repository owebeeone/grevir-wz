#include "registration.hpp"

namespace probe {

unsigned calls = 0;

void mock_esp_intr_alloc(unsigned source) {
  if (source == 0) {
    ++calls;
  }
}

} // namespace probe

int main() {
  probe::register_first();
  probe::register_second();
  return probe::calls == 2 ? 0 : 1;
}
