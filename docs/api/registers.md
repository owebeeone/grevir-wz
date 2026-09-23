# Registers API

**Package:** Grevir Registers. **Header:** `<GrevirRegisters.h>` or a focused
`<grevir/registers/...hpp>`. **CMake target:** `grevir::registers`.
**Dependencies:** Base.

Registers supplies typed bit mappings, field values, register selection and
grouped operations. A caller provides an address/type definition and an access
policy implementing `read<T>(address)`, `write<T>(address, value)` and
`modify<T>(address, value, mask)`. The backend owns address interpretation and
real I/O. `setl::McuRegister`, `setl::IoRegister`, `setl::BitFields` and
`setl::BitsRW` combine those parts; no global mock/real switch is required.

`Evaluate` performs no I/O. `Read()` takes one snapshot for field decoding.
Partial `ReadModifyWrite` uses the access policy's `modify`; a full-width write
uses `write`. `Write(values...)` clears unspecified bits, whereas
`Write(defaults, values...)` preserves defaults. Duplicate fields in one
grouped write are compile-time errors. Grouped operations are ordered, but a
multi-register operation is not automatically atomic.

`ApplierRunner::applySync<Operations, Barrier>()` uses a caller-supplied RAII
barrier; its construction and destruction surround the operations. The policy
must provide any interrupt or memory-ordering guarantee. The [AVR API](avr.md)
supplies concrete register bindings for its selected device scope.
