# Portable PWM declaration with an ATmega328P host memory policy

This complete C++23 program binds a portable 1 kHz PWM request to the
ATmega328P inventory using a byte-array register policy. It runs on a native
host, where it checks Timer1 TOP and compare values. It does not access
physical AVR registers or establish silicon timing. The ESP32-specific request
section is present but inert under the selected AVR backend.

The source is [pwm-host.cpp](pwm-host.cpp). Build it through an installed
`grevir::avr` CMake target, or compile it with the required Grevir package
include paths in a workspace checkout. A real target application must replace
`Memory` and `Barrier` with suitable device access and synchronization policies.
See the [PWM guide](../guides/pwm.md) for the request, ownership and target
limits, and [support](../supported.md) for evidence levels.
