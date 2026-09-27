#include "mock_interrupt_source.hpp"

#include <cassert>

static_assert(grevir_poc::HasEventHandler<grevir_poc::DirectTick>);
static_assert(grevir_poc::HasEventHandler<grevir_poc::DeferredTick>);
static_assert(!grevir_poc::HasEventHandler<grevir_poc::MissingTick>);
static_assert(grevir_poc::HasRawInterruptHandler<grevir_poc::RawTick>);
static_assert(!grevir_poc::HasRawInterruptHandler<grevir_poc::DirectTick>);

int main() {
  grevir_poc::MockInterruptSource<grevir_poc::RawTick>::fire();
  assert(grevir_poc::raw_calls == 1);

  grevir_poc::MockInterruptSource<grevir_poc::DirectTick>::fire();
  assert(grevir_poc::direct_calls == 1);
  assert(grevir_poc::MockBackend::queued() == 0);

  grevir_poc::MockInterruptSource<grevir_poc::DeferredTick>::fire();
  grevir_poc::MockInterruptSource<grevir_poc::DeferredTick>::fire(); // Elided while already pending.
  assert(grevir_poc::deferred_calls == 0);
  assert(grevir_poc::MockBackend::queued() == 1);

  const bool first_dispatched = grevir_poc::MockBackend::dispatch_one();
  assert(first_dispatched);
  assert(grevir_poc::deferred_calls == 1);
  assert(grevir_poc::MockBackend::queued() == 0);

  grevir_poc::MockInterruptSource<grevir_poc::DeferredTick>::fire(); // Accepted after dequeue.
  const bool second_dispatched = grevir_poc::MockBackend::dispatch_one();
  assert(second_dispatched);
  assert(grevir_poc::deferred_calls == 2);
}
