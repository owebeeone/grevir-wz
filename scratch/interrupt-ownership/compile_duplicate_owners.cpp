#include "ownership.hpp"

using Application = grevir::AllocatedApplication<probe::Backend,
    probe::ConsumerA, probe::Rival>;

int main() {
  Application::runSetup();
}
