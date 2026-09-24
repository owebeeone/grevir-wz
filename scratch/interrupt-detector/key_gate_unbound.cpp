#include "key_gate_fixture.hpp"

// A catalog event with no generated binding remains illegal to specialize.
template <>
inline void key_gate_probe::on_interrupt<key_gate_probe::Unbound>() noexcept {}
