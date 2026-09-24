#pragma once
#include "avr_app_base.hpp"

#if !defined(GREVIR_TEST_NO_HANDLER)
template <>
inline void grevir::on_interrupt<avr_app::PeriodElapsed>() noexcept {
  avr_app::grevir_irq_test_ticks =
    static_cast<unsigned char>(avr_app::grevir_irq_test_ticks + 1);
}
#endif
