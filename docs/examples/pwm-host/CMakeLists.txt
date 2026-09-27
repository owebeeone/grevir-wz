cmake_minimum_required(VERSION 3.20)
project(grevir_pwm_host_example LANGUAGES CXX)

find_package(grevir-avr CONFIG REQUIRED)
add_executable(grevir_pwm_host ../pwm-host.cpp)
target_link_libraries(grevir_pwm_host PRIVATE grevir::avr)
set_target_properties(grevir_pwm_host PROPERTIES CXX_EXTENSIONS OFF)
