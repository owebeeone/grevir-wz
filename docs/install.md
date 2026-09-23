# Install and build

Grevir libraries use C++23. Install only the packages your application needs;
each package declares its Grevir dependencies. The [package index](api/index.md)
lists the entry header and CMake target for every runtime library.

## Arduino Uno/Nano development setup

Place the required library directories directly in the Arduino sketchbook's
`libraries/` directory, or pass each checkout with Arduino CLI `--library`.
Install the Arduino AVR platform 1.8.8. The verified target build uses Debian
`avr-g++` 14.2, selected with `compiler.path=/usr/bin/`, and
`compiler.cpp.extra_flags=-std=c++23`. For example, from the workspace root on
the AVR build host:

```sh
arduino-cli compile --fqbn arduino:avr:uno \
  --library grevir-base --library grevir-time --library grevir-core \
  --library grevir-peripherals --library grevir-arduino \
  --library grevir-arduino-avr --library grevir-avr \
  --build-property compiler.path=/usr/bin/ \
  --build-property compiler.cpp.extra_flags=-std=c++23 \
  grevir-arduino/examples/Blink
```

Change the FQBN to `arduino:avr:nano` for Nano. Arduino's library manager can
resolve declared dependencies when the packages are installed under the
sketchbook; explicit paths are useful for a source checkout. Other guides name
their additional packages. [Support](supported.md) explains what target builds
have been checked. An upload or physical-board run is outside the current
validation.

## Native CMake consumer

Install the desired packages and their dependencies into one prefix, then use
their exported targets. For a Base + Packet consumer from the workspace root:

```sh
cmake -S grevir-base -B build/install-base \
  -DCMAKE_INSTALL_PREFIX=/path/to/grevir-prefix
cmake --build build/install-base
cmake --install build/install-base

cmake -S grevir-packet -B build/install-packet \
  -DCMAKE_PREFIX_PATH=/path/to/grevir-prefix \
  -DCMAKE_INSTALL_PREFIX=/path/to/grevir-prefix
cmake --build build/install-packet
cmake --install build/install-packet
```

In the consumer's `CMakeLists.txt`:

```cmake
find_package(grevir-packet CONFIG REQUIRED)
add_executable(my_app main.cpp)
target_link_libraries(my_app PRIVATE grevir::packet)
```

Configure the consumer with `-DCMAKE_PREFIX_PATH=/path/to/grevir-prefix`.
The installed Packet config finds Base. The CMake targets request C++23.
Catch2 and Grevir Test Support are development dependencies and are not needed
by an installed production consumer.

Native builds test logic and API portability. They do not select an MCU's
registers, clocks or pins. For that, use a named adapter and backend described
in the [package index](api/index.md).
