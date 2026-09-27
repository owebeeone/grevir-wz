#pragma once

#include "poc.hpp"

namespace grevir_poc {

template <class Event, class Backend,
          class Scope = typename Event::Route::ContextType>
struct OnEventBridge;

template <class Event, class Backend>
struct OnEventBridge<Event, Backend, IsrLevel> {
  static void from_isr() noexcept {
    grevir::on_event<Event>();
  }
};

template <class Event, class Backend>
struct OnEventBridge<Event, Backend, MainLoop> {
  static void invoke(void*) noexcept {
    grevir::on_event<Event>();
  }

  static bool from_isr() noexcept {
    return Backend::template post_from_isr<Event>(&invoke);
  }
};

} // namespace grevir_poc
