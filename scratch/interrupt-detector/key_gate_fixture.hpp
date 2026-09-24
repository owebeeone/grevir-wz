#pragma once

#include <concepts>
#include <cstddef>

namespace key_gate_probe {

template <std::size_t N>
struct Text {
  char data[N];
  consteval Text(const char (&value)[N]) {
    for (std::size_t i = 0; i < N; ++i) { data[i] = value[i]; }
  }
  constexpr bool operator==(const Text&) const = default;
};

template <Text Key> struct CatalogLookup;
template <Text Key> struct BoundEventKey;

struct Period { static constexpr Text key{"motor.period"}; };
struct Rogue { static constexpr Text key{"motor.period"}; };
struct Unbound { static constexpr Text key{"motor.unbound"}; };

template <> struct CatalogLookup<"motor.period"> { using type = Period; };
template <> struct CatalogLookup<"motor.unbound"> { using type = Unbound; };

// Stand-in for one generated key specialization; no C++ type name is emitted.
template <> struct BoundEventKey<"motor.period"> { static constexpr unsigned id = 13; };

template <class Event, class Binding = BoundEventKey<Event::key>,
          unsigned = Binding::id,
          class CatalogEvent = typename CatalogLookup<Event::key>::type>
  requires std::same_as<Event, CatalogEvent>
void on_interrupt() noexcept = delete;

} // namespace key_gate_probe
