cmake_minimum_required(VERSION 3.13)

include(lib/pico-sdk/external/pico_sdk_import.cmake)

project(org2-datapack-test)

pico_sdk_init()

file(GLOB SOURCES
    ${CMAKE_SOURCE_DIR}/src/*.cpp
)

add_executable(org2-datapack-test ${SOURCES})

target_link_libraries(org2-datapack-test pico_stdlib pico_multicore)

pico_enable_stdio_usb(org2-datapack-test 1)
pico_enable_stdio_uart(org2-datapack-test 0)

pico_add_extra_outputs(org2-datapack-test)