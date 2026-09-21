# Grevir AVR implementation and review policy

Apply together with [the cross-MCU policy](CrossMcu.md). This supplement governs
AVR backend code and the AVR-specific assessment of shared-code instantiations.
It does not impose AVR representation choices on ESP32 or other generic callers.

## Current target assumptions

The current milestone is classic ATmega328P-class AVR: an 8-bit CPU, with the
conventional AVR C++ ABI using 16-bit `int` and 32-bit `long`, and no hardware
floating-point unit. weftpi `avr-g++` 14.2.0 (`-mmcu=atmega328p`) measured
`sizeof(int)==2`, `sizeof(long)==4` and `sizeof(void*)==2` on 22 September 2026.
That compiler provides C++23 as a language and avr-libc C headers; it does not
install libstdc++. Peripheral register widths are separate from CPU and C++ type
widths.

Name the device and ABI for target-specific claims. Extend this supplement if
other AVR devices or ABI options require different assumptions. Do not generalize
host `sizeof(long)`, floating precision or generated instructions to AVR.

## Arithmetic and storage

- AVR integer peripheral configuration and control paths must remain integer
  unless fractional arithmetic is an explicit requirement of the chosen API.
  Do not unconditionally convert integer inputs to `float`, `double` or
  `long double` for convenient bounds checks, rounding or overflow avoidance.
- Support explicitly selected fractional functionality deliberately. Prefer an
  integer ratio or suitable fixed-point implementation when it preserves the
  contract economically. Merely spelling a floating operation `constexpr` does
  not satisfy the runtime policy.
- Use the smallest practical representation that covers the required values and
  intermediates correctly. Eight- and sixteen-bit choices deserve consideration,
  but 32-bit clocks, counters and arithmetic are legitimate when their ranges
  require them. Do not mechanically shrink every operation to the CPU width.
- Require a range-based justification for runtime 64-bit work; it is acceptable
  when needed for correctness. First consider overflow-safe integer rearrangement,
  quotient/remainder methods, or specialization for known bounds. Never trade
  correct rounding or overflow behavior for a smaller intermediate.
- A runtime mask of up to 32 bits should not require a 64-bit intermediate when
  it can be formed safely in 32 bits. Handle boundary widths without an invalid
  shift. Casting the value before a widening shift matters; casting its result
  is too late. Pay particular attention to 8-/16-bit operands promoted to `int`.
- Distinguish retained state from temporary expressions and compile-time-only
  constants. Do not demand removal of wide metadata that adds no target work.

## Reviewing impact and validation

Report the concrete AVR instantiation and whether the cost occurs at setup,
on repeated updates or in an interrupt handler. Software arithmetic cost is
relevant, but performance severity requires the path's requirements or measured
impact. A source-level violation of the integer-path rule can be established
without inventing instruction counts.

For shared code, report “affects the AVR instantiation” when appropriate and
preserve the cross-MCU API. An AVR-specific implementation or numeric policy may
be preferable to changing a generic default used by 32-bit targets. Conversely,
integer-promotion UB in shared code is a portability defect even if a 32-bit host
or ESP32 instantiation works.

Host mocks and sanitizers can establish logic errors and some UB. Host optimized
IR can establish that a particular dynamic call retains floating/wide arithmetic.
Neither establishes AVR code size, cycle count or hardware behavior.

**AVR compiler and simavr validation are authorized on weftpi**
(`gianni@10.1.1.236`) per [GrevirAvrValidationPlan.md](../GrevirAvrValidationPlan.md).
Do not treat macOS or Pi native host builds as that evidence. **Silicon hardware
validation remains on hold** until that plan's Phase 5 is scheduled. simavr is
not silicon: analog pin filtering, Timer2 asynchronous clock-domain delays,
oscillator stabilization after sleep, Timer1 16-bit TEMP byte order, and the
datasheet 4-cycle interrupt entry gap stay unclaimed until a named-board check.
Do not report held silicon validation as completed.
