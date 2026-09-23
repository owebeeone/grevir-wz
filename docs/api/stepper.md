# Stepper API

**Package:** Grevir Stepper. **Header:** `<GrevirStepper.h>`.
**CMake target:** `grevir::stepper`. **Dependencies:** Base, Time, Core and
Peripherals. Encoder is not a production dependency.

`step::StepperModule<Clock, StepPeriod, HoldPeriod, Unit, Pins...>` configures
and claims injected output pins, then polls a `step::Stepper` during
`runLoop()`. Default phase tables exist for 2, 3, 4 and 5 phases; the first
pin maps to the highest bit of a phase mask. A custom row array can be
supplied. `setTargetPosition()` and `incrementPosition()` queue movement;
position counts steps, not electrical degrees.

A step occurs only when elapsed time is strictly greater than the scaled
period. First forward movement applies sequence row 1. Hold period zero
leaves coils energized; a positive hold turns them off after that idle time.
`setTimeScale()` is an explicit floating-point multiplier of the period, so
runtime use on a no-FPU MCU may be costly. [Motion](../guides/motion.md)
shows a module declaration. This package is host-tested; a standalone AVR
Stepper target path has not been recorded.
