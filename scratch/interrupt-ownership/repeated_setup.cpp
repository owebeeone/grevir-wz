#include "ownership.hpp"

using Application = grevir::AllocatedApplication<probe::Backend, probe::Owner>;

int main() {
  Application::runSetup();
  Application::runSetup();
  return probe::registrations == 2 ? 0 : 1;
}
