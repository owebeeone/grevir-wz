#include "catalog_probe.cpp"

struct DuplicateMotorEvent { using Key = MotorKey; };
struct DuplicateMotorRequest {
  using InterruptEvents = setl::TypeArgs<DuplicateMotorEvent>;
};
using Duplicate = grevir::RequestedModule<
  setl::TypeArgs<DuplicateMotorRequest>, EmptyModule>;
using BadCatalog = grevir::interrupt::EventCatalog<
  grevir::ApplicationSpec<Board, Motor, Duplicate>>;
static_assert(BadCatalog::keys.size() == 2);
