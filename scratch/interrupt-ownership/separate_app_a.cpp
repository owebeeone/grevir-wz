#include "ownership.hpp"

using ApplicationA = grevir::AllocatedApplication<probe::Backend, probe::Owner>;

void setup_a() { ApplicationA::runSetup(); }
