# Win11 native validation — 23 September 2026

At source root `bab54d206ece83c67b7e41ac634a20b3fc53c843`, dabeest pulled
the Packet AVR portability changes, rebuilt the full Visual Studio native
workspace with compiler probes enabled, and passed **186/186** CTest cases,
including all 12 Packet cases. This is a host portability recheck; the target
result is recorded separately in [Packet AVR evidence](GrevirPacketAvrEvidence.md).

At source root commit `a3abeac9c17e3c4918a37ca7fb889b070c2d6b08`, the
full Win11 MSVC build passes with both native runtime tests and compiler-only
checks enabled. All configured positive and expected-rejection probes pass;
CTest passes **186/186** cases. The probes use the same standard C++23
`static_assert` and template cases as Clang/GNU. The CMake scripts select
MSVC's `/std:c++latest`, `/Zs`, `/D` and `/I` command-line forms and accept
MSVC's static-assert diagnostic wording.

The workspace was cloned through GitHub with GWZ to
`gianni@dabeest:/e/git/grevir-wz` in the remote MinGW Bash shell. At source root
commit `aa9b30917bbe0a72d7c1be8c4f4414c13863cc0d`, Windows 11 build 26200,
Visual Studio 2022 Build Tools, MSVC 19.44.35228.0 and its CMake 3.31.6
configured and built the full native runtime suite. Catch2 3.8.1 was fetched by
the existing test setup. CTest passed **186/186** cases, including all 12 Packet
cases and both Pulse IO cases.

The initial CMake configuration used the Visual Studio 17 2022 x64 generator
with `GREVIR_BUILD_HOST_TESTS=ON`, `GREVIR_FETCH_TEST_DEPENDENCIES=ON` and
`GREVIR_BUILD_COMPILE_CHECKS=OFF`; the build and CTest selected `Debug`.
Compiler-only checks were enabled in the follow-up above. That exposed one
encoder compile-test access issue; its assertions now compare the protected
enum inside the derived test helper.

From `/e/git/grevir-wz`, using the CMake and CTest executables installed under
Visual Studio Build Tools, the steps were:

```sh
export PATH="/c/Program Files (x86)/Microsoft Visual Studio/2022/BuildTools/Common7/IDE/CommonExtensions/Microsoft/CMake/CMake/bin:$PATH"
cmake -S . -B build/win11-msvc -G "Visual Studio 17 2022" -A x64 \
  -DGREVIR_BUILD_HOST_TESTS=ON -DGREVIR_BUILD_COMPILE_CHECKS=ON \
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
