# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if (WIN32)
    set(USERHOME $ENV{USERPROFILE})
else ()
    set(USERHOME $ENV{HOME})
endif ()
set(sdkVersion 2.1.1)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.1.1)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif ()
# ====================================================================================

cmake_minimum_required(VERSION 3.20)

set(PROJECT stepper_direct)

set(CMAKE_C_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# cmake -DPICO_BOARD=pico2 -DCMAKE_BUILD_TYPE=Debug --fresh ..
set(PICO_BOARD pico CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be done before project)
include(pico_sdk_import.cmake)

if (PICO_SDK_VERSION_STRING VERSION_LESS "2.1.1")
    message(FATAL_ERROR "Raspberry Pi Pico SDK version 2.1.1 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

project(${PROJECT} C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable
add_executable(${PROJECT}
        main.c
)

pico_set_program_name(${PROJECT} "${PROJECT}")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(${PROJECT} 0)
# Note: USB serial console adds significant overhead (TinyUSB init and stuff)
pico_enable_stdio_usb(${PROJECT} 1)
# See Segger RTT readme section on how to use RTT
pico_enable_stdio_rtt(${PROJECT} 1)


target_include_directories(${PROJECT} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(${PROJECT} PRIVATE 
        pico_stdlib
)

# Picotool processing (signing/encrypting/etc) and creating hex/bin/map/uf2 files in addition to ELF
pico_add_extra_outputs(${PROJECT})


# "make flash" to flash the firmware with openocd (connect the board with SWD connector first)
add_custom_target(flash
        openocd ${CMAKE_CURRENT_LIST_DIR}/openocd.cfg -c "program ${PROJECT}.elf verify reset exit"
        COMMENT "openocd program verify reset"
        DEPENDS ${PROJECT}
)

# "make gdb" to launch gdb server for both cores and RTT console
# Connect to serial console: telnet 127.0.0.1 9090
# Connect with gdb (core0): gdb "${TARGET}.elf" -ex "target extended-remote localhost:3333"
add_custom_target(gdb
        openocd -f ${CMAKE_CURRENT_LIST_DIR}/openocd_gdb.cfg
        COMMENT "openocd GDB server with RTT"
        DEPENDS ${PROJECT}
)

# "make flash_usb" to flash the firmware with picotool (connect the board in BOOTSEL mode first)
add_custom_target(flash_usb
        picotool load -f -x ${PROJECT}.uf2
        COMMENT "picotool load"
        DEPENDS ${PROJECT}
)
