#include "ownership.hpp"

void setup_a();
void setup_b();

int main() {
  setup_a();
  setup_b();
  return probe::registrations == 2 ? 0 : 1;
}
