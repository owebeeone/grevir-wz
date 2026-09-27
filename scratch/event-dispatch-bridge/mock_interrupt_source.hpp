#pragma once

#include "irq_dispatch.hpp"
#include "mock_backend.hpp"

namespace grevir_poc {

template <class Event>
struct MockInterruptSource {
  static void fire() noexcept {
    dispatch_bound_interrupt<Event, MockBackend>();
  }
};

} // namespace grevir_poc
