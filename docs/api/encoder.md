# Encoder API

**Package:** Grevir Encoder. **Header:** `<GrevirEncoder.h>`.
**CMake target:** `grevir::encoder`. **Dependencies:** Base, Time, Core and
Peripherals.

`quad::QuadEncoder<PinA, PinB>` samples injected pins with static `get()`;
`quad::QuadEncoderModule<Encoder>` adds Core setup, loop and pin claims.
Physical high maps to a zero bit and low to a one bit. The first `iterate()`
latches the pair and returns zero. Adjacent Gray-code changes return +1 or
-1. A two-bit jump returns twice the last direction, or zero before a
direction is known. `getCurrentPosition()` accumulates scaled changes;
`setCurrentPosition()` replaces the count.

The default scaler is identity. The optional `InteractiveScaler<Clock>` is an
explicit floating-point algorithm; its runtime cost is relevant on no-FPU
targets. The inherited base class retains virtual input/scaling methods.
These choices do not imply dynamic allocation by themselves. A combined
Stepper/Encoder Arduino sketch has not been target-validated. See the
[motion guide](../guides/motion.md) and [support](../supported.md).
