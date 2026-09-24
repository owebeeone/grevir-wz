#pragma once

#include "mock_app_base.hpp"

template <>
inline void grevir::on_interrupt<mock_app::PeriodElapsed>() noexcept {
  ++mock_app::ticks;
  if (mock_app::reraising && mock_app::ticks == 1) {
    mock_app::controller.raise();
  }
}
