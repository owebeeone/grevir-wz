# Interrupt implementation gate proofs

Run from the workspace root on a macOS host with Apple Clang:

```sh
python3 -B scratch/interrupt-implementation-gates/check_proofs.py
```

The first proof uses the existing `grevir::RequestedModule` and ATmega328P
PWM allocator with a host register-memory fixture. An inline
`on_interrupt<MotorInstance::Timer::PeriodElapsed>()` specialization calls a
preallocation facade. The handler is parsed and detected before the application
allocation type exists; the facade's out-of-line definition subsequently uses
the selected PWM binding. The script compiles both a permissive probe mode and
a strict mode with a stand-in generated key gate, then calls the handler after
setup and checks the Timer1 duty value. The compile uses the repository's
native `HAS_STD_LIB=1` setting. This proves the declaration/definition seam
for one representative handler, not that the current allocator consumes
interrupt demands or that the handwritten stand-in is a production emitter.

The second proof compiles one pointer-free, relocation-free byte record as a
section in three object formats: native arm64 Mach-O, cross-compiled x86-64
ELF and cross-compiled x86-64 COFF. The production-started
`grevir-core/tools/grevir_irqgen/object_section.py` extracts identical bytes
from all three and rejects missing, duplicate, relocated and truncated
sections. The test intentionally compiles a pointer-bearing record to verify
relocation rejection. Cross-compiling these objects proves the compiler's
object emission and the host parser; it does not prove execution or native
compiler behavior on Linux/Windows, AVR/ESP32 section emission, or the final
wire schema/JSON generator. Those remain implementation gates.

The section name is `.grevir_irq_plan` in ELF/COFF. Mach-O requires a segment
and section pair, so the compiler uses `__DATA,__grevir_irq`; the extractor
maps it to the same logical record. The emitted record in this proof is a
fixed sample, not a selected binding plan.
