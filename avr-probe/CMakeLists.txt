cmake_minimum_required(VERSION 3.20)
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  project(grevir_avr_probe LANGUAGES CXX)
endif()

add_executable(grevir_avr_abi abi.cpp)
add_executable(grevir_avr_features features.cpp)
target_compile_features(grevir_avr_abi PRIVATE cxx_std_23)
target_compile_features(grevir_avr_features PRIVATE cxx_std_23)

if(TARGET grevir::avr)
  add_executable(grevir_avr_firmware firmware.cpp)
  target_link_libraries(grevir_avr_firmware PRIVATE grevir::avr)
  target_compile_features(grevir_avr_firmware PRIVATE cxx_std_23)

  function(grevir_avr_sim_firmware name source)
    add_executable(grevir_avr_sim_${name} sim/${source})
    target_link_libraries(grevir_avr_sim_${name} PRIVATE grevir::avr)
    target_compile_features(grevir_avr_sim_${name} PRIVATE cxx_std_23)
    target_include_directories(grevir_avr_sim_${name} PRIVATE
      ${CMAKE_CURRENT_SOURCE_DIR}/sim)
  endfunction()

  grevir_avr_sim_firmware(ocr ocr.cpp)
  grevir_avr_sim_firmware(latch latch.cpp)
  grevir_avr_sim_firmware(irq irq.cpp)

  function(grevir_avr_header_probe name target)
    add_library(grevir_avr_header_${name} OBJECT headers/${name}.cpp)
    target_link_libraries(grevir_avr_header_${name} PRIVATE ${target})
    target_compile_features(grevir_avr_header_${name} PRIVATE cxx_std_23)
  endfunction()

  grevir_avr_header_probe(base grevir::base)
  grevir_avr_header_probe(time grevir::time)
  grevir_avr_header_probe(core grevir::core)
  grevir_avr_header_probe(peripherals grevir::peripherals)
  grevir_avr_header_probe(registers grevir::registers)
  grevir_avr_header_probe(avr grevir::avr)

  add_custom_target(grevir_avr_claim_checks ALL
    COMMAND "${CMAKE_COMMAND}"
      "-DCXX=${CMAKE_CXX_COMPILER}"
      "-DCORE_INCLUDE=${CMAKE_CURRENT_SOURCE_DIR}/../grevir-core/src"
      "-DBASE_INCLUDE=${CMAKE_CURRENT_SOURCE_DIR}/../grevir-base/src"
      "-DCASE_SOURCE=${CMAKE_CURRENT_SOURCE_DIR}/../grevir-core/tests/claim_cases.cpp"
      "-DLOG_DIR=${CMAKE_CURRENT_BINARY_DIR}/claim-results"
      -P "${CMAKE_CURRENT_SOURCE_DIR}/check_claims.cmake"
    COMMENT "AVR expected-failure GPIO claim collision probe"
    VERBATIM)
endif()
