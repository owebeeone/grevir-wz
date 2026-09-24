cmake_minimum_required(VERSION 3.25)
project(grevir_irq_installed_smoke LANGUAGES CXX)

find_package(grevir-core CONFIG REQUIRED)
get_filename_component(WORKSPACE "${CMAKE_CURRENT_LIST_DIR}/../../.." ABSOLUTE)
add_executable(installed_mock
  "${WORKSPACE}/scratch/interrupt-implementation-gates/mock_main.cpp")
target_link_libraries(installed_mock PRIVATE grevir::core)
target_compile_features(installed_mock PRIVATE cxx_std_23)
target_compile_definitions(installed_mock PRIVATE HAS_STD_LIB=1)
target_include_directories(installed_mock PRIVATE
  "${WORKSPACE}/grevir-peripherals/src"
  "${WORKSPACE}/grevir-test-support/include")
grevir_add_interrupt_bindings(
  TARGET installed_mock
  APPLICATION_HEADER "${WORKSPACE}/scratch/interrupt-implementation-gates/mock_app.hpp"
  BACKEND mock TARGET_ID mock_mcu BOARD mock_board COMPILER_ID scratch_compiler)
enable_testing()
add_test(NAME installed_mock_dispatch COMMAND installed_mock)
