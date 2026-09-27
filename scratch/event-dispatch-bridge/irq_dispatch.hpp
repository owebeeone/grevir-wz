#pragma once

#include "bridge.hpp"

namespace grevir_poc {

template <class Event>
concept HasRawInterruptHandler = requires {
  grevir::on_interrupt<Event>();
};

template <class Event>
concept HasEventHandler = requires {
  typename Event::Route::ContextType;
  grevir::on_event<Event>();
};

template <class Event, class Backend>
void dispatch_bound_interrupt() noexcept {
  static_assert(HasRawInterruptHandler<Event> != HasEventHandler<Event>,
                "exactly one interrupt or event handler is required");
  if constexpr (HasRawInterruptHandler<Event> && !HasEventHandler<Event>) {
    grevir::on_interrupt<Event>();
  } else if constexpr (HasEventHandler<Event> &&
                       !HasRawInterruptHandler<Event>) {
    OnEventBridge<Event, Backend>::from_isr();
  }
}

} // namespace grevir_poc
