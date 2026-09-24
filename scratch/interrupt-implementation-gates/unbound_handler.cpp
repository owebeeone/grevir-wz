#include "mock_app_base.hpp"

struct UnboundEvent {
  using Key = grevir::interrupt::EventKey<"unbound", "timer", "period">;
};

template <>
inline void grevir::on_interrupt<UnboundEvent>() noexcept {}
