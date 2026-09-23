# Base API

**Package:** Grevir Base. **Header:** `<GrevirBase.h>` or a focused
`<grevir/base/...hpp>` header. **CMake target:** `grevir::base`.
**Dependencies:** none of the other Grevir packages.

Base supplies integer-type selection, cyclic integers, optional values,
circular buffers, color conversion, scaling, type/tuple algorithms and the
compatibility headers used by the selected AVR path. Names remain in `setl`
and `ardo` where inherited. A native CMake consumer gets C++23 and
`HAS_STD_LIB=1` through the target. On the selected AVR compiler, Base's
compatibility subset replaces unavailable C++ library headers. This subset is
validated for the named target compositions, not for every standard-library
feature or instantiation.

`setl::System` is a sequential-use default. Its barrier hooks are no-ops;
they do not make `CircularBuffer` thread- or interrupt-safe. Supply an
appropriate synchronization policy when one is required. Dynamic arithmetic
and storage cost depend on the type and template arguments selected by the
caller; a `constexpr` declaration alone does not guarantee a runtime call is
eliminated.

Use [Time](time.md) for typed durations and [Core](core.md) for application
composition. See [supported platforms](../supported.md) for target evidence.
