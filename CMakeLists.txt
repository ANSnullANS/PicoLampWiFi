cmake_minimum_required(VERSION 3.13)
set(PICO_SDK_FETCH_FROM_GIT 1)
set(PICO_EXTRAS_FETCH_FROM_GIT 1)
include(pico_sdk_import.cmake)
include(pico_extras_import.cmake)
project(PicoLampWiFi C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(PICO_BOARD pico_w)

pico_sdk_init()

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/3rdparty/pi-pico-adafruit-neopixels/pico_neopixels build)

set(FS_DATA_DIR ${CMAKE_CURRENT_LIST_DIR}/fsdata)
set(MAKE_FS_DATA_SCRIPT ${CMAKE_CURRENT_LIST_DIR}/fsdata/makefsdata)

set(marker ${CMAKE_CURRENT_BINARY_DIR}/.rebuilt)
add_custom_command(
  OUTPUT ${marker}
  COMMAND
    ${CMAKE_COMMAND} -E touch ${marker}
)
add_custom_command(OUTPUT ${FS_DATA_DIR}/fsdata.c
    COMMAND ${MAKE_FS_DATA_SCRIPT}
    DEPENDS ${marker}
    WORKING_DIRECTORY ${FS_DATA_DIR}
)
file(REMOVE ${marker})

set(CMAKE_BUILD_TYPE Debug)

add_executable(PicoLampWiFi
    PicoLampWiFi.cpp
    Rainbow.cpp
    dhcpserver/dhcpserver.cpp
    mdns_server.c
    ${PICO_SDK_PATH}/lib/lwip/src/apps/mdns/mdns.c
)

target_include_directories(PicoLampWiFi PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/dhcpserver
        ${PICO_SDK_PATH}/lib/lwip/src/apps/mdns
)

target_compile_definitions(PicoLampWiFi PRIVATE
        HTTPD_FSDATA_FILE="${CMAKE_CURRENT_LIST_DIR}/fsdata/fsdata.c"
        WIFI_SSID=\"$ENV{WIFI_SSID}\"
        WIFI_PASSWORD=\"$ENV{WIFI_PASSWORD}\"
)

pico_enable_stdio_usb(PicoLampWiFi 1)
pico_enable_stdio_uart(PicoLampWiFi 1)

target_link_libraries(PicoLampWiFi 
    pico_stdlib
    pico_cyw43_arch_lwip_threadsafe_background
    pico_lwip_http
    pico_lwip_mdns
    pico_stdlib
    pico_multicore
    pico_neopixel
    pico_unique_id
    pico_bootsel_via_double_reset
    hardware_flash
)

pico_add_extra_outputs(PicoLampWiFi)