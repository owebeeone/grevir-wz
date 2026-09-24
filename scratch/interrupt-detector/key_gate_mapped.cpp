#include "key_gate_fixture.hpp"

template <>
inline void key_gate_probe::on_interrupt<key_gate_probe::Period>() noexcept {}

int main() {
  key_gate_probe::on_interrupt<key_gate_probe::Period>();
}
