cmake_minimum_required(VERSION 3.13)

include(lib/pico-sdk/external/pico_sdk_import.cmake)

project(org2-datapack-test)

pico_sdk_init()

file(GLOB SOURCES
    ${CMAKE_SOURCE_DIR}/src/*.cpp
)

add_definitions(-DWIFI_SSID="$ENV{WIFI_SSID}" -DWIFI_PSK="$ENV{WIFI_PSK}")

add_executable(org2-datapack-test ${SOURCES})

target_include_directories(org2-datapack-test PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(org2-datapack-test
    pico_stdlib
    pico_multicore 
    pico_cyw43_arch_lwip_threadsafe_background
    pico_lwip_http
    pico_lwip_mdns
)

pico_enable_stdio_usb(org2-datapack-test 1)
pico_enable_stdio_uart(org2-datapack-test 0)

pico_add_extra_outputs(org2-datapack-test)