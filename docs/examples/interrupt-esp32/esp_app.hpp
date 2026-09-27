#pragma once
#include "esp_app_base.hpp"

#if !defined(GREVIR_TEST_NO_HANDLER)
template <>
inline void grevir::on_event<esp_app::PeriodElapsed>() noexcept {
  ++esp_app::ticks;
}
#endif
