# Interrupt bindings

Grevir discovers a handler from the application header, selects a legal timer
configuration, and generates the target entry code. The first supported events
are a payload-free ATmega328P Timer1 overflow event, a classic ESP32
Timer Group 0 / Timer 0 alarm event, and the mock timer event. Other vectors
and peripheral interrupt types require their own backend inventories.

An application declares a stable event identity through the request carried
by a `RequestedModule`:

```cpp
using TimerKey = grevir::interrupt::EventKey<"motor", "timer", "period">;
struct PeriodElapsed { using Key = TimerKey; };
struct TimerRequest {
  using InterruptEvents = setl::TypeArgs<PeriodElapsed>;
};
```

The application header includes the board and module declarations, defines
`GrevirApplication`, then includes
`<grevir/interrupt/handler.hpp>` and the specialization:

```cpp
template <>
inline void grevir::on_interrupt<PeriodElapsed>() noexcept {
  Motor::tick();
}
```

The specialization must be visible in that header during both probe and final
compilation. Declaring an event without a specialization creates no interrupt
demand. A specialization for an event outside the catalog or one that the
selected allocation cannot bind fails compilation. There is no separate
user-written ISR or registration list. Event keys and request identities must
be stable and unique; reordering independent module declarations does not
change the selected allocation.

`on_event<Event>()` can also activate a catalogued hardware event. Its default
route elides repeated firings into a fixed-capacity main-loop queue. The
application dispatches a bounded number of callbacks in its `loop()`:

```cpp
template <>
inline void grevir::on_event<PeriodElapsed>() noexcept {
  Motor::tick();
}

void loop() {
  grevir::event::dispatch<GrevirApplication>(4);
  if (grevir::event::overrun<GrevirApplication>()) {
    // Record or report that at least one firing could not be queued.
    grevir::event::clear_overrun<GrevirApplication>();
  }
}
```

For one callback per **accepted** firing, specialize the route before the
handler:

```cpp
template <>
struct grevir::event::RouteFor<PeriodElapsed> {
  using Context = grevir::event::MainLoop;
  using Delivery = grevir::event::Stream;
};

template <>
inline void grevir::on_event<PeriodElapsed>() noexcept {
  Motor::tick();
}
```

`Stream` adds a record for every firing until the selected queue is full;
it never returns `coalesced`. A full queue drops that firing, returns `full`
to a caller of `post` or `post_from_isr`, and sets the same sticky overrun
flag. The hardware entry also uses this queue. A `Stream` record carries no
payload; a device needing bytes or frames retains them in its owned buffer.
Choose either this route or the default `Elide` route for each event.

The board supplies `event_queue_capacity`, an `EventLock` with task and ISR
guards, and a `MainLoopContext<Application>` policy. The generated plan includes
both the lock and context policy identities. Mock and AVR boards explicitly use
`UnrestrictedMainLoopContext<Application>`; the ESP32 board binds a task owner.
On a single-core target
the guards may use the same interrupt-state operation; on ESP32 they enter one
cross-core spinlock through the appropriate FreeRTOS context API. An accepted
event stays queued until
dispatch. With `Elide`, another firing while that event is queued returns
`coalesced` without adding a record. With `Stream`, each firing needs its own
record. When a new record is needed and the queue is full, that firing is
dropped and a sticky overrun flag is set. The callback runs
outside the queue lock and may post again. For a catalogued hardware event,
loop code may call `event::post<Application, Event>()`, and an ISR may call
`event::post_from_isr<Application, Event>()`; each returns `queued`,
`coalesced`, `full`, or `not_ready`. The queue is prepared during application
startup before interrupt sources are enabled and stopped if startup fails.
Preparing an already active or currently dispatching queue fails without
changing its owner or records; startup reports `event_context_failed`.
Capacity is the number of records the board selects, not a preallocated maximum;
this queue permits selections from 1 through 2048. The selected value is
checked at compile time before it is converted to an index or plan field.
`overrun<Application>()` reads the flag without clearing it;
`clear_overrun<Application>()` clears only the flag, leaving queued callbacks
intact. Preparing the queue during startup resets the flag. A nested or
competing `dispatch()` call returns zero while another dispatch is active;
callbacks for one application do not run concurrently through this queue.

A loop publisher can inspect the exact result, including a full queue or a
queue that has not been prepared (or has been stopped after startup failure):

```cpp
const auto posted = grevir::event::post<GrevirApplication, PeriodElapsed>();
if (posted == grevir::event::PostResult::full) {
  // The firing was dropped; decide whether to retry later.
} else if (posted == grevir::event::PostResult::not_ready) {
  // The application has not started successfully.
}
```

An explicit direct route invokes the handler in interrupt context:

```cpp
template <>
struct grevir::event::RouteFor<PeriodElapsed> {
  using Context = grevir::event::IsrLevel;
  using Delivery = grevir::event::Direct;
};

template <>
inline void grevir::on_event<PeriodElapsed>() noexcept {
  Motor::tick();
}
```

Use either `on_interrupt` or `on_event` for a given event, not both. The default
`on_event` route is `MainLoop`/`Elide`; `MainLoop`/`Stream` is also supported on
mock, ATmega328P, and classic ESP32. Extra dispatch contexts remain future work.
The `IsrLevel` route has the same interrupt-context
restrictions as `on_interrupt`.
The ESP32 example binds `MainLoop` to the Arduino task running `setup()` and
`loop()`; `dispatch` from another task returns zero. It polls the queue on each
`loop()` call, so handler latency depends on loop cadence. It creates no
additional task and does not sleep or notify the loop task. Software-only event
catalogues are also pending; the current handler probe treats each visible
catalogue handler as a hardware interrupt demand.
The ESP32 example's `delivered_callbacks` value counts handler invocations,
not every hardware period: Elide may coalesce firings and a full queue may
drop one.

The board inventory declares legal configurations, physical sources,
selectors, entry and acknowledgement policies. It also declares reservations
for sources owned by the core or another library. One physical source gets
one generated owner and one callback/entry. An interrupt handler runs in
interrupt context: keep it bounded, avoid blocking, and use only operations
safe under the selected backend. The classic ESP32 adapter registers its
callback on the core that calls `esp_intr_alloc`; it does not request an
IRAM-safe handler. Do not use this provisional example for cache-disabled or
flash-operation interrupt service.
Multiple logical events sharing one physical source are not yet emitted;
the current validator rejects such plans before generation.

## Build the examples

The complete [Uno Timer1 example](../examples/interrupt-avr/interrupt-avr.ino)
and [classic ESP32 example](../examples/interrupt-esp32/interrupt-esp32.ino)
include their application headers and selected board inventories. The examples
call `Application<Spec>::start()`, whose board lifecycle configures the selected
timer mode before registering and enabling the generated interrupt. Run the
staged build from a workspace checkout containing the
listed Grevir libraries. The command probes the target compiler, writes a
canonical plan and generated binding files in a private stage, compiles the
strict sketch, and publishes firmware with
`grevir_irq_firmware_ready.json` only after linking succeeds.

For the Uno on the Raspberry Pi with Debian AVR GCC 14.2 and Arduino AVR
1.8.8, ensure `arduino-cli` is on `PATH` (for a per-user install, run
`export PATH="$HOME/.local/bin:$PATH"` first):

```sh
python3 -B grevir-core/tools/grevir_irqgen/__main__.py arduino-build \
  --arduino-cli "$(command -v arduino-cli)" \
  --sketch docs/examples/interrupt-avr \
  --work-dir build/irq-arduino-work --output-dir build/irq-avr-export \
  --fqbn arduino:avr:uno --backend avr --target atmega328p --board uno \
  --compiler avr_gcc14 --application-header avr_app.hpp \
  --build-property compiler.path=/usr/bin/ \
  --library grevir-base --library grevir-time --library grevir-core \
  --library grevir-peripherals --library grevir-registers \
  --library grevir-avr --library grevir-arduino --library grevir-arduino-avr
```

For the classic ESP32 Dev Module with Arduino-ESP32 3.3.11:

```sh
python3 -B grevir-core/tools/grevir_irqgen/__main__.py arduino-build \
  --arduino-cli "$(command -v arduino-cli)" \
  --sketch docs/examples/interrupt-esp32 \
  --work-dir build/irq-arduino-work --output-dir build/irq-esp32-export \
  --fqbn esp32:esp32:esp32 --backend esp32 --target esp32_classic \
  --board esp32_dev_module --compiler esp_x32_2601 \
  --application-header esp_app.hpp \
  --library grevir-base --library grevir-time --library grevir-core \
  --library grevir-peripherals --library grevir-arduino \
  --library grevir-arduino-esp32
```

The `--compiler` value is an application/toolchain identity: it must match
the `Board::compiler` constant. It is not a compiler executable path. The
board, target and backend arguments must likewise match the board declaration.
Use a distinct work and export directory for simultaneous builds. A failed
attempt removes the current ready marker; do not use artifacts without a
matching marker. Direct Arduino IDE or bare `arduino-cli compile` does not
run generation.

The complete [mock application](../examples/interrupt-mock/mock_app.hpp),
[main](../examples/interrupt-mock/main.cpp), and
[CMake file](../examples/interrupt-mock/CMakeLists.txt) run from the same
workspace checkout on a native C++23 host:

```sh
cmake -S docs/examples/interrupt-mock -B build/irq-doc-mock
cmake --build build/irq-doc-mock
ctest --test-dir build/irq-doc-mock -C Debug --output-on-failure
```

CTest reports that the mock interrupt dispatch test passed. The CMake file
shows the complete `grevir_add_interrupt_bindings()` invocation and board
identities. The function compiles a probe from the same application header,
emits the binding unit, verifies the ready output set and adds it to the
firmware target. The mock controller comes from the development-only Grevir
Test Support package.

## Diagnostics and evidence

A missing generated unit causes a final link failure.
`GREVIR_IRQ_EVENT_NOT_IN_CATALOG` or
`GREVIR_IRQ_EVENT_TYPE_MISMATCH` indicates that the visible handler and
selected catalog differ. An incomplete `BoundEventKey` error indicates a
visible handler without a selected binding. Generation rejects malformed, relocated,
duplicated, mismatched or tampered probe records and plans.

The mock path runs on macOS, Linux and Win11/MSVC. The Uno example compiles
and links with AVR GCC 14.2 and its handler runs once in simavr. The classic
ESP32 example compiles and links for the named board and core; interrupt
routing and physical-board behavior have not been run. See
[supported platforms](../supported.md) for the validation boundary.
