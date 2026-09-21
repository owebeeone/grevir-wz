foreach(case RANGE 0 7)
  execute_process(
    COMMAND "${COMPILER}" -std=c++23 -fsyntax-only "-DPROTOTYPE_CASE=${case}" "${SOURCE}/compile_probe.cpp"
    RESULT_VARIABLE status OUTPUT_VARIABLE output ERROR_VARIABLE error)
  file(WRITE "${OUTPUT}/probe-${case}.log" "${output}${error}")
  if(case EQUAL 0)
    if(NOT status EQUAL 0)
      message(FATAL_ERROR "Positive control failed: ${error}")
    endif()
  else()
    if(status EQUAL 0 OR NOT error MATCHES "GREVIR_TIMER_PROTOTYPE_ALLOCATION_FAILED"
        OR error MATCHES "UNEXPECTED_PROTOTYPE_STATUS")
      message(FATAL_ERROR "Case ${case} did not fail for its expected reason: ${error}")
    endif()
  endif()
endforeach()
message(STATUS "Positive control and seven expected allocation failures checked")
