#include "ownership.hpp"

using Application = grevir::AllocatedApplication<probe::Backend,
    probe::ConsumerA, probe::ConsumerB>;

int main() {
  Application::runSetup();
  return probe::registrations == 1 ? 0 : 1;
}
