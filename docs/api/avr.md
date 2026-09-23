# AVR API

**Package:** Grevir AVR. **Header:** `<GrevirAVR.h>` plus a selected device
header such as `<grevir/avr/devices/atmega328p/timers.hpp>`.
**CMake target:** `grevir::avr`. **Dependencies:** Base, Registers and Core.

The package provides explicit AVR register and synchronization policies,
GPIO port wrappers, timer clock/mode/definition/configuration/output types and
ATmega328P Timer0/1/2 facts. It has no Arduino dependency. Target code must
select a device and supply its register access and barrier policies. An I/O
offset is explicit: `nfp::IoRegisterDef<T, Address, Offset>` applies it once;
there is no universal offset silently added by the library.

The installed ATmega328P PWM backend is
`grevir::pwm::atmega328p::Backend<Device, ClockHz>`. `Device` binds the timer
inventory to access and barrier policies. Requests come from Core and portable
Peripherals; see the [PWM guide](../guides/pwm.md). The current application
allocation scope is fixed-frequency synchronous fast PWM. Timer0/1/2 routes,
TOP sources and resource reservations are represented, but additional timer
features and other AVR devices are not promised by this v0.1.0 path.

AVR is an 8-bit CPU; the selected ATmega328P ABI has 16-bit `int`, 32-bit
`long` and no hardware floating-point unit. Integer control paths do not
require incidental floating-point arithmetic. Hardware register ordering,
asynchronous clock behavior and electrical timing require physical-board
validation; see [support](../supported.md).
