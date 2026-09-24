#include "mock_app_base.hpp"

struct ForeignEvent { using Key = mock_app::TimerKey; };

template <>
inline void grevir::on_interrupt<ForeignEvent>() noexcept {}
