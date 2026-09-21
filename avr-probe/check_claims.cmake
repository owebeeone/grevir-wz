cmake_minimum_required(VERSION 3.20)

file(MAKE_DIRECTORY "${LOG_DIR}")
set(flags
  -std=c++23
  -mmcu=atmega328p
  -fno-exceptions
  -fno-rtti
  -fsyntax-only
  "-I${CORE_INCLUDE}"
  "-I${BASE_INCLUDE}")

function(run_claim_case case expect_success diagnostic)
  execute_process(COMMAND "${CXX}" ${flags} "-DCASE_ID=${case}" "${CASE_SOURCE}"
    RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE errors)
  file(WRITE "${LOG_DIR}/case-${case}.log" "${output}${errors}")
  if(expect_success)
    if(NOT result STREQUAL "0")
      message(FATAL_ERROR "AVR positive claim case ${case} failed:\n${output}${errors}")
    endif()
    return()
  endif()
  if(NOT result MATCHES "^[1-9][0-9]*$")
    message(FATAL_ERROR "AVR claim case ${case}: expected compiler rejection, got ${result}")
  endif()
  if(NOT errors MATCHES "static assertion failed[^\n]*${diagnostic}")
    message(FATAL_ERROR
      "AVR claim case ${case} failed without its expected diagnostic:\n${errors}")
  endif()
endfunction()

# Same source and compiler for the passing control and the expected collision.
run_claim_case(1 TRUE "")
run_claim_case(2 FALSE "Application has resource conflict")
message(STATUS "AVR claim probes: 1 valid application and 1 expected collision passed")
