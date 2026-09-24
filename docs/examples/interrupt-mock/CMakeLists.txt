cmake_minimum_required(VERSION 3.25)
project(grevir_interrupt_mock LANGUAGES CXX)

get_filename_component(WORKSPACE "${CMAKE_CURRENT_LIST_DIR}/../../.." ABSOLUTE)
add_executable(interrupt_mock main.cpp)
target_compile_features(interrupt_mock PRIVATE cxx_std_23)
target_compile_definitions(interrupt_mock PRIVATE HAS_STD_LIB=1)
target_include_directories(interrupt_mock PRIVATE
  "${WORKSPACE}/grevir-base/src"
  "${WORKSPACE}/grevir-core/src"
  "${WORKSPACE}/grevir-peripherals/src"
  "${WORKSPACE}/grevir-test-support/include")
find_package(Threads REQUIRED)
target_link_libraries(interrupt_mock PRIVATE Threads::Threads)
set(GREVIR_IRQGEN_TOOL_DIR "${WORKSPACE}/grevir-core/tools/grevir_irqgen")
include("${WORKSPACE}/grevir-core/cmake/GrevirInterruptBindings.cmake")
grevir_add_interrupt_bindings(
  TARGET interrupt_mock
  APPLICATION_HEADER "${CMAKE_CURRENT_LIST_DIR}/mock_app.hpp"
  BACKEND mock TARGET_ID mock_mcu BOARD mock_board COMPILER_ID native_cxx23)
enable_testing()
add_test(NAME interrupt_mock_dispatch COMMAND interrupt_mock)
