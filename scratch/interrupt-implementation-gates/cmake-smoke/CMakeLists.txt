cmake_minimum_required(VERSION 3.25)
project(grevir_irq_smoke LANGUAGES CXX)

get_filename_component(WORKSPACE "${CMAKE_CURRENT_LIST_DIR}/../../.." ABSOLUTE)
add_executable(mock_firmware "${WORKSPACE}/scratch/interrupt-implementation-gates/mock_main.cpp")
target_compile_features(mock_firmware PRIVATE cxx_std_23)
target_compile_definitions(mock_firmware PRIVATE HAS_STD_LIB=1)
target_include_directories(mock_firmware PRIVATE
  "${WORKSPACE}/grevir-base/src"
  "${WORKSPACE}/grevir-core/src"
  "${WORKSPACE}/grevir-peripherals/src"
  "${WORKSPACE}/grevir-test-support/include")
set(GREVIR_IRQGEN_TOOL_DIR "${WORKSPACE}/grevir-core/tools/grevir_irqgen")
include("${WORKSPACE}/grevir-core/cmake/GrevirInterruptBindings.cmake")
grevir_add_interrupt_bindings(
  TARGET mock_firmware
  APPLICATION_HEADER "${WORKSPACE}/scratch/interrupt-implementation-gates/mock_app.hpp"
  BACKEND mock TARGET_ID mock_mcu BOARD mock_board COMPILER_ID scratch_compiler)

enable_testing()
add_test(NAME mock_dispatch COMMAND mock_firmware)

add_executable(mock_zero_firmware
  "${WORKSPACE}/scratch/interrupt-implementation-gates/mock_zero_main.cpp")
target_compile_features(mock_zero_firmware PRIVATE cxx_std_23)
target_compile_definitions(mock_zero_firmware PRIVATE HAS_STD_LIB=1)
target_include_directories(mock_zero_firmware PRIVATE
  "${WORKSPACE}/grevir-base/src"
  "${WORKSPACE}/grevir-core/src"
  "${WORKSPACE}/grevir-peripherals/src"
  "${WORKSPACE}/grevir-test-support/include")
grevir_add_interrupt_bindings(
  TARGET mock_zero_firmware
  APPLICATION_HEADER "${WORKSPACE}/scratch/interrupt-implementation-gates/mock_app_base.hpp"
  BACKEND mock TARGET_ID mock_mcu BOARD mock_board COMPILER_ID scratch_compiler)
add_test(NAME mock_zero_handler COMMAND mock_zero_firmware)

find_package(Threads REQUIRED)
add_executable(mock_start_test
  "${WORKSPACE}/scratch/interrupt-implementation-gates/mock_start_test.cpp")
target_compile_features(mock_start_test PRIVATE cxx_std_23)
target_compile_definitions(mock_start_test PRIVATE HAS_STD_LIB=1)
target_include_directories(mock_start_test PRIVATE
  "${WORKSPACE}/grevir-base/src"
  "${WORKSPACE}/grevir-core/src"
  "${WORKSPACE}/grevir-peripherals/src"
  "${WORKSPACE}/grevir-test-support/include")
target_link_libraries(mock_start_test PRIVATE Threads::Threads)
grevir_add_interrupt_bindings(
  TARGET mock_start_test
  APPLICATION_HEADER "${WORKSPACE}/scratch/interrupt-implementation-gates/mock_app.hpp"
  BACKEND mock TARGET_ID mock_mcu BOARD mock_board COMPILER_ID scratch_compiler)
add_test(NAME mock_start_outcomes COMMAND mock_start_test)
