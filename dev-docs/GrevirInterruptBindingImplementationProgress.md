# Interrupt binding implementation progress

Status: the first mock, ATmega328P Timer1-overflow, and classic ESP32
Timer Group 0/Timer 0 binding slice is implemented and under pre-commit
review. The second review identified two structural gaps: the selected timer
configuration was lost with no handler, and a candidate owner could be
self-asserted outside the module closure. Both have been corrected and the
focused code recheck found them closed. This record distinguishes
host execution, target compilation, simulation, and physical hardware.
The accepted [architecture](GrevirInterruptBindingArchitecture.md) and
[implementation plan](GrevirInterruptBindingImplementationPlan.md) remain the
scope references. The generated C++ is written from canonical JSON and is
never parsed back by the tool.

## Implemented path

`grevir::ApplicationSpec` derives a finite event catalog from the declared
`RequestedModule` dependency closure. Visible `on_interrupt<Event>()`
specializations create demands in the probe pass. The bounded timer allocator
selects a configuration and source for those demands; strict compilation
checks the generated key gate and the selected binding records. The probe
emits a pointer-free, checksummed object section. `grevir-irqgen` extracts
ELF, COFF, or Mach-O bytes; validates the record; writes canonical JSON; and
emits `grevir_generated_irq_bindings_<backend>.hpp/.cpp`. A ready marker is
published only after the complete generated set is written. CMake and staged
Arduino CLI adapters compile the probe and final application.

Mock execution covers dispatch, a zero-handler installer and disabled source,
source ownership, including rejection of a candidate owner outside the event
instance in the module closure,
PWM/event coexistence and conflict, repeated and concurrent startup, pending
events, and terminal setup/cleanup failures. The AVR entry uses the actual
Timer1 overflow vector. The classic ESP32 entry installs one callback via
`esp_intr_alloc` and directly configures TG0/T0 registers; it does not call
the Arduino or IDF LEDC software APIs.

## Validation on 24 September 2026

| Environment | Evidence | Limit |
| --- | --- | --- |
| macOS arm64, Apple Clang 21 | Object transport proofs, mock round trip, CMake smoke 3/3, public mock 1/1, installed consumer 1/1, full host CTest 186/186 | Host mock behavior only |
| Raspberry Pi, native GCC 14.2 | Mock round trip, CMake smoke 3/3, public mock 1/1, installed consumer 1/1, full host CTest 186/186 | Host mock behavior only |
| Win11 `gianni@dabeest`, MSVC 19.44 | Native COFF extraction, positive and expected-rejection compiler probes, generated mock dispatch, CMake smoke 3/3, public mock 1/1, installed consumer 1/1, full host CTest 186/186 | Host mock behavior only |
| Raspberry Pi, Debian AVR GCC 14.2, Arduino AVR 1.8.8 | Public Uno example two-pass target compile/link in normal and fast-PWM-8 Timer1 modes; final normal and zero-handler variants compile/link; zero-handler ELF has only the weak default vector and no generated source guard | No physical board run |
| Raspberry Pi, simavr 1.6 | Final normal-mode Uno ELF enters the generated Timer1 overflow vector and increments the handler counter once; earlier fast-PWM-8 mode also invoked it once | Simulation does not establish silicon timing |
| Raspberry Pi, classic ESP32 Dev Module, Arduino-ESP32 3.3.11, esp-x32 2601 | Public ESP32 example two-pass target compile/link through `Application::start()` in normal and zero-handler variants; zero-handler plan has no demand and its ELF has no generated source guard | No physical-board or interrupt-routing run |

The executable repeatable checks are in
[`scratch/interrupt-implementation-gates`](../scratch/interrupt-implementation-gates).
The named remote workspace paths are `/home/gianni/git/grevir-wz` on the Pi
and `/e/git/grevir-wz` on Win11. Both hold the same implementation files used
for these checks; the remotes are test workspaces, not a substitute for a
committed Git revision.

## Remaining boundary

Only a period/alarm event without payload is emitted. AVR startup preserves a
pending overflow for delivery when its source is enabled, and both target
sketches enter an application-defined failure path if startup fails. Pending
overflow injection on a simulated target and a forced ESP32 registration
failure have not been run. No Timer0/2 AVR event,
capture/compare event, ESP32-S2/S3 or RISC-V source, UART/CAN interrupt,
arbitrary source sharing, direct IDE build, or physical-device behavior is
claimed. The ESP32 adapter's failure path has target compile/link evidence and
host-model tests, not a target fault-injection run. The CMake adapter has
host mock tests and rejects a tampered generated header before final
compilation; Arduino target firmware is exported through the staged
`arduino-build` command. Hardware validation remains on hold.
