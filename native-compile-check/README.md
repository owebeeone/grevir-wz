# Temporary legacy native compile check

This small CMake project checks that two representative, existing Ardoinus
translation units compile to native object files with Xcode Clang. It does not
link or run an application, simulate hardware, or provide a test framework.

From the `grevir-wz` workspace root, configure and build it outside the repository:

```sh
build_dir="$(mktemp -d)"
cmake -S native-compile-check -B "$build_dir" \
  -G "Unix Makefiles" \
  -DCMAKE_CXX_COMPILER="$(xcrun --find clang++)" \
  -DARDOINUS_DIR=/path/to/ardoinus/ardOinus
cmake --build "$build_dir" --verbose
```

`ARDOINUS_DIR` must name the legacy directory containing `src/`, `tests/`, and
`examples/`. The build creates object files for `tests/dependent_module.cxx` and
the `examples/Control/ArdOArrays/ArdOArrays.ino` sketch. Both targets use C++23
and define `HAS_STD_LIB=1`; the sketch also uses the existing
`ARDO_USE_MOCK_ARDUINO=1` declarations.

The sketch wrapper includes `<utility>` because the legacy `setl_optional.h`
uses `std::move` without including that header itself. This setup deliberately
leaves the legacy source unchanged. The sketch currently emits legacy warnings;
they remain visible and do not fail this temporary compile check.
