cmake_minimum_required(VERSION 3.20)
project(grevir_workspace LANGUAGES CXX)

# Only extracted libraries participate at this stage.
option(GREVIR_BUILD_COMPILE_CHECKS "Compile native headers and template users" ON)
option(GREVIR_BUILD_HOST_TESTS "Run deterministic native behavior tests" OFF)
if(GREVIR_BUILD_HOST_TESTS)
  enable_testing()
endif()
add_subdirectory(grevir-base)
add_subdirectory(grevir-time)
add_subdirectory(grevir-core)
