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
