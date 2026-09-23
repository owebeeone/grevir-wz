# Time API

**Package:** Grevir Time. **Header:** `<GrevirTime.h>` or
`<grevir/time/...hpp>`. **CMake target:** `grevir::time`.
**Dependencies:** Base.

Time provides `setl::Time<T, Unit>`, periods, unit conversions, literals and
interactive scaling. Choose an integer type and a `setl::TimeUnit` that cover
the intended range and resolution. A target clock supplies values in matching
units; see [Pins and clocks](../concepts/pins-and-clocks.md).

`Period::operator/` divides its stored period by the scalar operand and keeps
the period's storage type and units. Integer division can truncate; callers
needing fractional semantics must choose a suitable representation.
`setl::RelativeInteractiveScaler` is explicitly fractional. Using it at
runtime on a no-FPU MCU can incur software floating-point work; it is not an
implicit cost of every `Time` or `Period` operation.

Typed time is used by [Peripherals](peripherals.md), [Pulse Codec](pulse-codec.md)
and motion packages. The selected AVR paths use integer clocks; the full Time
API has native test coverage, not exhaustive target coverage.
