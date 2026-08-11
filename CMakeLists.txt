# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.3.0)
set(toolchainVersion 15_2_Rel1)
set(picotoolVersion 2.3.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================

# Tells the SDK to use the Feather's specific pinout and 8MB flash size
set(PICO_BOARD adafruit_feather_rp2040 CACHE STRING "Board type")

cmake_minimum_required(VERSION 3.13)
include(pico_sdk_import.cmake)

project(max30101_heart_rate)
pico_sdk_init()

add_executable(max30101_hr main.c)

# Link the standard library and I2C hardware support
target_link_libraries(max30101_hr pico_stdlib hardware_i2c)

# Enable USB serial output (for printf) and disable UART
pico_enable_stdio_usb(max30101_hr 1)
pico_enable_stdio_uart(max30101_hr 0)

# Generate the .uf2, .hex, and .bin files needed for flashing
pico_add_extra_outputs(max30101_hr)
