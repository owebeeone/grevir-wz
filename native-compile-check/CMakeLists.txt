cmake_minimum_required(VERSION 3.20)

project(grevir_legacy_native_compile_check LANGUAGES CXX)

set(ARDOINUS_DIR "" CACHE PATH "Path to the legacy ardOinus directory")

if(NOT ARDOINUS_DIR)
  message(FATAL_ERROR
    "ARDOINUS_DIR is required. Pass the path to the legacy ardOinus directory, "
    "for example -DARDOINUS_DIR=/path/to/ardoinus/ardOinus.")
endif()

set(ARDOINUS_SOURCE_DIR "${ARDOINUS_DIR}/src")
set(ARDOINUS_DEPENDENT_MODULE_SOURCE
  "${ARDOINUS_DIR}/tests/dependent_module.cxx")
set(ARDOINUS_ARRAYS_DIR
  "${ARDOINUS_DIR}/examples/Control/ArdOArrays")
set(ARDOINUS_ARRAYS_SOURCE
  "${ARDOINUS_ARRAYS_DIR}/ArdOArrays.ino")

foreach(required_path IN ITEMS
    "${ARDOINUS_SOURCE_DIR}"
    "${ARDOINUS_DEPENDENT_MODULE_SOURCE}"
    "${ARDOINUS_ARRAYS_SOURCE}")
  if(NOT EXISTS "${required_path}")
    message(FATAL_ERROR
      "ARDOINUS_DIR does not contain the expected legacy source: ${required_path}")
  endif()
endforeach()

add_library(legacy_dependent_module_compile OBJECT
  "${ARDOINUS_DEPENDENT_MODULE_SOURCE}")
set_target_properties(legacy_dependent_module_compile PROPERTIES
  CXX_STANDARD 23
  CXX_STANDARD_REQUIRED YES
  CXX_EXTENSIONS NO)
target_compile_definitions(legacy_dependent_module_compile PRIVATE HAS_STD_LIB=1)
target_include_directories(legacy_dependent_module_compile PRIVATE
  "${ARDOINUS_SOURCE_DIR}")

add_library(legacy_ardo_arrays_compile OBJECT arrays_compile.cxx)
set_target_properties(legacy_ardo_arrays_compile PROPERTIES
  CXX_STANDARD 23
  CXX_STANDARD_REQUIRED YES
  CXX_EXTENSIONS NO)
target_compile_definitions(legacy_ardo_arrays_compile PRIVATE
  HAS_STD_LIB=1
  ARDO_USE_MOCK_ARDUINO=1)
target_include_directories(legacy_ardo_arrays_compile PRIVATE
  "${ARDOINUS_SOURCE_DIR}"
  "${ARDOINUS_ARRAYS_DIR}")

add_custom_target(native_compile_check ALL
  DEPENDS
    legacy_dependent_module_compile
    legacy_ardo_arrays_compile)
