# Win11 native validation — 23 September 2026

The workspace was cloned through GitHub with GWZ to
`gianni@dabeest:/e/git/grevir-wz` in the remote MinGW Bash shell. At source root
commit `aa9b30917bbe0a72d7c1be8c4f4414c13863cc0d`, Windows 11 build 26200,
Visual Studio 2022 Build Tools, MSVC 19.44.35228.0 and its CMake 3.31.6
configured and built the full native runtime suite. Catch2 3.8.1 was fetched by
the existing test setup. CTest passed **186/186** cases, including all 12 Packet
cases and both Pulse IO cases.

The CMake configuration used the Visual Studio 17 2022 x64 generator with
`GREVIR_BUILD_HOST_TESTS=ON`, `GREVIR_FETCH_TEST_DEPENDENCIES=ON` and
`GREVIR_BUILD_COMPILE_CHECKS=OFF`; the build and CTest selected `Debug`. The
compiler-only expected-rejection probes require a Clang/GNU command-line driver
and their CMake configuration rejects MSVC. They were therefore not run on Win11.
From `/e/git/grevir-wz`, using the CMake and CTest executables installed under
Visual Studio Build Tools, the steps were:

```sh
export PATH="/c/Program Files (x86)/Microsoft Visual Studio/2022/BuildTools/Common7/IDE/CommonExtensions/Microsoft/CMake/CMake/bin:$PATH"
cmake -S . -B build/win11-msvc -G "Visual Studio 17 2022" -A x64 \
  -DGREVIR_BUILD_HOST_TESTS=ON -DGREVIR_BUILD_COMPILE_CHECKS=OFF \
  -DGREVIR_FETCH_TEST_DEPENDENCIES=ON
cmake --build build/win11-msvc --config Debug --parallel 8
ctest --test-dir build/win11-msvc -C Debug --output-on-failure
```

This run exposed two portability defects and a startup-order defect. Base's
pre-C++17 type-trait fallback used MSVC's legacy `__cplusplus` value and
redeclared `std::remove_cvref`; it now checks the active `_MSVC_LANG` value.
The ATmega328P PWM claim template now expands an endpoint-claim alias accepted
by MSVC. The Pulse IO waveform factory is now constant-initialized so its
settings exist before the module's static decoder and encoder instances.
The corrected sources also rebuild and pass 186/186 native tests on macOS.

The affected shared code was rerun on weftpi with Debian `avr-g++` 14.2,
Arduino AVR 1.8.8 and simavr 1.6; see
[Pulse IO AVR evidence](GrevirPulseIoAvrEvidence.md). Win11 native tests do not
establish AVR target behavior or physical hardware timing. Silicon validation
remains on hold.
