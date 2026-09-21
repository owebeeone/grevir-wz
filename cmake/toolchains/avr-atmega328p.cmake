# ATmega328P / 16 MHz Uno-class toolchain for weftpi Debian avr-g++ 14.2.
# Use a separate build tree. Do not configure host-mock with this file.
# CMake must not try to run AVR binaries on the Raspberry Pi CPU.

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR avr)

set(CMAKE_C_COMPILER avr-gcc)
set(CMAKE_CXX_COMPILER avr-g++)
set(CMAKE_ASM_COMPILER avr-gcc)
set(CMAKE_OBJCOPY avr-objcopy)
set(CMAKE_OBJDUMP avr-objdump)
set(CMAKE_SIZE avr-size)
find_program(CMAKE_AR avr-gcc-ar)
find_program(CMAKE_RANLIB avr-gcc-ranlib)

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

set(GREVIR_AVR_MCU atmega328p CACHE STRING "AVR MCU for this toolchain")
set(GREVIR_AVR_F_CPU 16000000UL CACHE STRING "CPU frequency in Hz")
set(GREVIR_HAS_STD_LIB OFF CACHE BOOL "Debian avr-g++ has no libstdc++")
set(GREVIR_BUILD_COMPILE_CHECKS OFF CACHE BOOL "Host compile probes are not AVR firmware")
set(GREVIR_BUILD_HOST_TESTS OFF CACHE BOOL "Catch2 host tests are not AVR firmware")

add_compile_options(
  -mmcu=${GREVIR_AVR_MCU}
  -DF_CPU=${GREVIR_AVR_F_CPU}
  -ffunction-sections
  -fdata-sections
  -fno-exceptions
  -fno-rtti)
add_link_options(
  -mmcu=${GREVIR_AVR_MCU}
  -Wl,--gc-sections
  -Wl,-Map=${CMAKE_BINARY_DIR}/avr.map)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
