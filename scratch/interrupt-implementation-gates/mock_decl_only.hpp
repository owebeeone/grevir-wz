#pragma once
#include "mock_app_base.hpp"

template <>
void grevir::on_interrupt<mock_app::PeriodElapsed>() noexcept;
