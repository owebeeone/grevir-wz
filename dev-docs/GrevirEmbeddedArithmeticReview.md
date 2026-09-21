# Embedded arithmetic review — 21 September 2026

Reviewed the extracted production source in Base, Time, Core, Peripherals,
Registers and AVR for implicit floating-point arithmetic and unnecessarily wide
runtime arithmetic or storage. Compatibility branches were included in the
source search. Test Support is a host fixture package and was excluded from
target-cost findings. This review does not cover unextracted Ardoinus drivers.

No production code was changed during the initial review. No AVR compiler or hardware validation was run.
Native compiler probes were written under ignored `build/embedded-cost-review/`;
they are not additions to the project's test suite.

Classification corrected after review feedback: apply the
[cross-MCU policy](review-policies/CrossMcu.md) and
[AVR supplement](review-policies/Avr.md). Generic API choices must serve AVR and
32-bit ESP32; a cost concern for one target is not automatically a shared-API
defect. The earlier classification of wide time literals as a medium-severity
defect, and the implied generic no-FPU policy for scaling, are withdrawn below.

## Follow-up corrections — 21 September 2026

The user authorized fixes after checkpoint `203a8fc`. Findings 1 and 2 and the
related integer-promotion defect were corrected and committed at `654bdf7`:

- Integer AVR timer calls use 32-bit integer calculations, including a safe mask
  and successive ceiling divisions. Explicit floating calls use the selected
  floating precision and checked conversion bounds.
- Fallback `rand()` uses unsigned 32-bit modular arithmetic and masks before the
  result's conversion to `int`; its reference sequence and UBSan probe pass.
- Integer scaling casts to its output type before the expansion shift.

All 93 host cases pass. Optimized native IR verifies no floating-point or 64-bit
arithmetic in the inspected dynamic 32-bit timer calls. All 256 byte-to-32-bit
scaler values agree with a wide host oracle under UBSan; this does not reproduce
AVR's 16-bit promotion model. AVR compiler/hardware validation remains on hold.
The generic API choices in items 3 and 4 are unchanged. The findings below retain
the original source locations and review evidence for context.

## Confirmed findings

### 1. AVR policy violation: timer helpers force floating-point arithmetic for integer calls

Location: `grevir-avr/src/grevir/avr/timer/clock.hpp`, lines 104–120,
146–156 and 172–179. Introduced during the current extraction.
Scope: AVR backend, including dynamic integer frequency/count updates.

`getClockDividerMultiple`, `getTimerFrequency` and `getClockTimerTop` convert
inputs to `long double` regardless of the caller's input/output types. The
divider helper also forms a 1–32-bit mask with a 64-bit shift. Being `constexpr`
does not remove these costs when arguments are runtime values.

An optimized native LLVM-IR probe using dynamic `uint32_t` inputs retains
integer-to-floating conversions and floating divisions. This confirms the
runtime arithmetic category, not AVR instruction counts or floating precision.
The violation is the avoidable floating path in AVR integer helpers; its timing
impact and performance severity have not been measured.

Correction direction: separate integer and explicitly selected fractional
arithmetic, use a 32-bit mask, and handle overflow and rounding without blanket
widening. Preserve the existing invalid-input and count conventions.

### 2. High: fallback random generator uses overflowing signed 64-bit arithmetic

Location: `grevir-base/src/grevir/base/compat/cstdlib.hpp`, lines 16–24.
Inherited unchanged from `ardOinus/src/setlx_cstdlib.h`.
Scope: shared compatibility fallback, on configurations that select it.

When `HAS_STD_LIB` is absent, `rand()` stores its state in `long long` and
multiplies by `1103515245ll`, although the returned result retains only 15 bits.
There is no demonstrated need for 64-bit state to produce those low output bits,
but the confirmed defect is undefined signed overflow, not the type name:
a native sanitizer probe fails on the second call from the
initial seed. Optimized IR retains a signed 64-bit multiplication.

Correction direction: specify the intended recurrence using a suitable unsigned
state type, with defined modular arithmetic. Review output compatibility against
that definition; behavior after signed overflow is not a valid contract.
Native builds with `HAS_STD_LIB` use the standard library instead.

## Generic API choices — not confirmed violations

### 3. Integer time literals select wide storage even for tiny values

Location: `grevir-time/src/grevir/time/time.hpp`, lines 239–298; for example,
`operator"" _msec` at line 280. Inherited from `ardOinus/src/setl_time.h`.
Scope: shared duration API, with target-dependent runtime costs.

`auto delay = 5_msec` has type `Period<unsigned long long, MILLIS>`. Retaining
that type in runtime state retains the wide representation. An optimized probe
dividing a stored literal-derived period by a dynamic divisor emits `udiv i64`;
the equivalent explicitly chosen `Period<uint32_t>` emits `udiv i32`.

This does not mean every use of a literal incurs wide runtime operations:
constant folding or conversion into an explicitly narrow period can remove them.
The cooked literal's `unsigned long long` parameter is required by C++, but its
return representation is an API choice. A wide representation may be required
by the generic duration range. Establish that contract and the affected target's
requirements before recommending a change; AVR cost alone does not justify
narrowing the cross-MCU default. Explicit narrow representations remain useful
for constrained callers. Do not silently truncate existing large literals.

### 4. Fractional interactive scaling

Location: `grevir-time/src/grevir/time/interactive_scaling.hpp`, lines 12,
34–54 and 68–72. Inherited from `ardOinus/src/setl_interactive_scaling.h`.
Scope: shared fractional algorithm, not an AVR backend integer helper.

`RelativeInteractiveScaler<int>` defaults its scale type to `float`; its
time-dependent smoothing and fractional remainder therefore use floating-point
arithmetic despite integer input/output values. An optimized native probe retains
floating division, multiplication and conversions. Additionally, elapsed time is
hard-coded as `Period<float>` instead of following `ScaleType`.

The fractional algorithm and floating default are API choices, not established
violations of the cross-MCU policy. AVR users may benefit from an explicit
fixed-point alternative; this does not require removing the generic floating
version. Whether hard-coding the elapsed type violates supported `ScaleType`
semantics requires establishing that contract. Simply selecting an integer
`ScaleType` is not a valid replacement: the `0.2` smoothing coefficient becomes
zero. A fixed-point version needs an explicit fractional representation, rounding
and range policy. No blanket floating-point capability is assumed for ESP32.

## Related integer-promotion defect found during review

Location: `grevir-base/src/grevir/base/int_scaler.hpp`, line 82.
Inherited from `ardOinus/src/setl_int_scaler.h`.
Scope: shared implementation, affecting AVR's 16-bit-`int` instantiation.

In `scale_mersenne<32, 8>`, the expression `value << 24` promotes the 8-bit input
to `int` before the result is assigned to `out_type`. On a target with 16-bit
`int`, the shift count exceeds the promoted operand width. A 32-bit destination
does not widen the expression retroactively. Cast to the required unsigned output
width before shifting. This is a source-level finding; no AVR compilation was
performed. It is a correctness issue about insufficient intermediate width,
separate from the unnecessary widening findings above.

## Reviewed uses that are not runtime-cost findings

- Storage-region end-address calculations use `uint64_t` in constant expressions
  for bounds and resource declarations; storage reads/writes do not thereby use
  64-bit address arithmetic.
- Register semantic distinguisher codes and integer type-selection lists are
  compile-time metadata, not wide arithmetic in register access.
- Timer frequency requirements currently carry an integer numerator/divider and
  a `frequency_type` alias. The default `float` alias does not itself execute
  floating-point arithmetic. Future backend consumers must avoid making it an
  implicit floating runtime path.
- Ordinary `Time<T>`/`Period<T>` operations and unit conversion follow their
  selected numeric types. Floating types explicitly selected by the caller are
  different from unconditional conversions inside integer helpers.
- GPIO virtual interfaces are optional; they are not forced onto the default
  concrete pin types. Host fixture allocations are outside production MCU code.

The scan found no other forced floating-point arithmetic in the extracted
production source. This is not a target code-size, timing or whole-program
allocation guarantee.
