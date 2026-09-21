cmake_minimum_required(VERSION 3.20)
project(grevir_simavr_host LANGUAGES CXX)

# Native weftpi host driver. Do not configure this tree with the AVR toolchain.
add_executable(grevir_simavr_host host.cpp)
target_compile_features(grevir_simavr_host PRIVATE cxx_std_17)
target_include_directories(grevir_simavr_host PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
  /usr/include/simavr)
target_link_libraries(grevir_simavr_host PRIVATE simavr elf)
