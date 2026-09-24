#include "key_gate_fixture.hpp"

// A foreign event cannot borrow the valid catalog event's stable key.
template <>
inline void key_gate_probe::on_interrupt<key_gate_probe::Rogue>() noexcept {}
