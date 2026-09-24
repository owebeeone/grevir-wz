#include "ownership.hpp"

using ApplicationB = grevir::AllocatedApplication<probe::Backend, probe::Rival>;

void setup_b() { ApplicationB::runSetup(); }
