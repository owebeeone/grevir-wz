# Portable PWM declaration with an ATmega328P host memory policy

This complete C++23 program binds two 1 kHz PWM uses owned by one module to
the ATmega328P Timer1 inventory using a byte-array register policy. It runs on
a native host, where it checks TOP 15999 and both compare values (3999 and
11999). It does not access
physical AVR registers or establish silicon timing. The ESP32-specific request
section is present but inert under the selected AVR backend.

The source is [pwm-host.cpp](pwm-host.cpp). Once the AVR package and its Grevir
dependencies are installed as described in the [installation guide](../install.md),
build and run its [consumer CMake project](pwm-host/CMakeLists.txt) from the
workspace root:

```sh
cmake -S docs/examples/pwm-host -B build/pwm-host \
  -DCMAKE_PREFIX_PATH=/path/to/grevir-prefix
cmake --build build/pwm-host
./build/pwm-host/grevir_pwm_host
```

An exit status of zero means both outputs and the shared TOP matched the
expected register values. A real target application must replace
`Memory` and `Barrier` with suitable device access and synchronization policies.
See the [PWM guide](../guides/pwm.md) for the request, ownership and target
limits, and [support](../supported.md) for evidence levels.
