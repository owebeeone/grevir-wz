#pragma once

#include "mock_app_base.hpp"

template <>
struct grevir::event::RouteFor<mock_app::PeriodElapsed> {
#if defined(GREVIR_TEST_STALE_ROUTE)
  using Context = grevir::event::MainLoop;
  using Delivery = grevir::event::Elide;
#else
  using Context = grevir::event::IsrLevel;
  using Delivery = grevir::event::Direct;
#endif
};

template <>
inline void grevir::on_event<mock_app::PeriodElapsed>() noexcept {
  ++mock_app::ticks;
}
