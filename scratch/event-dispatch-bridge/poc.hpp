#pragma once

#include <grevir/interrupt/handler.hpp>

namespace grevir_poc {

struct IsrLevel {};
struct MainLoop {};

template <class Context>
struct EventRoute {
  using ContextType = Context;
};

struct DirectTick {
  using Route = EventRoute<IsrLevel>;
};

struct DeferredTick {
  using Route = EventRoute<MainLoop>;
};

struct MissingTick {
  using Route = EventRoute<IsrLevel>;
};

struct RawTick {};

struct ConflictingTick {
  using Route = EventRoute<IsrLevel>;
};

struct DispatchRecord {
  void (*invoke)(void*) noexcept;
  void* argument;
};

inline unsigned direct_calls = 0;
inline unsigned deferred_calls = 0;
inline unsigned raw_calls = 0;

} // namespace grevir_poc

namespace grevir {

template <class Event>
void on_event() noexcept = delete;

} // namespace grevir

template <>
inline void grevir::on_event<grevir_poc::DirectTick>() noexcept {
  ++grevir_poc::direct_calls;
}

template <>
inline void grevir::on_event<grevir_poc::DeferredTick>() noexcept {
  ++grevir_poc::deferred_calls;
}

template <>
inline void grevir::on_interrupt<grevir_poc::RawTick>() noexcept {
  ++grevir_poc::raw_calls;
}

template <>
inline void grevir::on_interrupt<grevir_poc::ConflictingTick>() noexcept {}

template <>
inline void grevir::on_event<grevir_poc::ConflictingTick>() noexcept {}
