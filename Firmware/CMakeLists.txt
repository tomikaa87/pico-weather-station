cmake_minimum_required(VERSION 3.12)

# Pull in SDK (must be before project)
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(PicoWeatherStationFirmware C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

if (PICO_SDK_VERSION_STRING VERSION_LESS "1.3.0")
    message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.3.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

# Initialize the SDK
pico_sdk_init()

add_compile_options(
    -Wall
    -Wno-format          # int != int32_t as far as the compiler is concerned because gcc has int32_t as long int
    -Wno-unused-function # we have some for the docs that aren't called
    -Wno-maybe-uninitialized
)

add_library(u8g2
    3rdparty/u8g2/csrc/u8g2_bitmap.c
    3rdparty/u8g2/csrc/u8g2_box.c
    3rdparty/u8g2/csrc/u8g2_buffer.c
    3rdparty/u8g2/csrc/u8g2_circle.c
    3rdparty/u8g2/csrc/u8g2_cleardisplay.c
    3rdparty/u8g2/csrc/u8g2_d_memory.c
    3rdparty/u8g2/csrc/u8g2_font.c
    3rdparty/u8g2/csrc/u8g2_hvline.c
    3rdparty/u8g2/csrc/u8g2_intersection.c
    3rdparty/u8g2/csrc/u8g2_kerning.c
    3rdparty/u8g2/csrc/u8g2_ll_hvline.c
    3rdparty/u8g2/csrc/u8g2_setup.c
    3rdparty/u8g2/csrc/u8x8_8x8.c
    3rdparty/u8g2/csrc/u8x8_byte.c
    3rdparty/u8g2/csrc/u8x8_cad.c
    3rdparty/u8g2/csrc/u8x8_capture.c
    3rdparty/u8g2/csrc/u8x8_display.c
    3rdparty/u8g2/csrc/u8x8_gpio.c
    3rdparty/u8g2/csrc/u8x8_d_st7586s_erc240160.c
    3rdparty/u8g2/csrc/u8x8_setup.c
    3rdparty/u8g2/csrc/u8x8_u16toa.c
)

target_include_directories(u8g2
    PUBLIC
        3rdparty/u8g2/cppsrc
        3rdparty/u8g2/csrc
)

add_subdirectory(3rdparty/fmt)

add_executable(PicoWeatherStationFirmware
    Fonts.cpp
    Fonts.h
    Graphics.cpp
    Graphics.h
    Icons.cpp
    Icons.h
    main.cpp

    Hardware/I2cDevice.cpp
    Hardware/I2cDevice.h
    Hardware/RealTimeClock.cpp
    Hardware/RealTimeClock.h

    Drivers/EERAM_47xxx.c
    Drivers/EERAM_47xxx.h
    Drivers/RTC_MCP7940N.c
    Drivers/RTC_MCP7940N.h
    Drivers/bme280_defs.h
    Drivers/bme280.c
    Drivers/bme280.h
    Drivers/nrf24.c
    Drivers/nrf24.h
    Drivers/u8x8_d_st7586s_erc240160_chunked.c

    Screens/WeatherStation.cpp
    Screens/WeatherStation.h

    Widgets/Display.cpp
    Widgets/Display.h
    Widgets/Font.cpp
    Widgets/Font.h
    Widgets/Image.cpp
    Widgets/Image.h
    Widgets/Label.cpp
    Widgets/Label.h
    Widgets/Painter.cpp
    Widgets/Painter.h
    Widgets/Point.cpp
    Widgets/Point.h
    Widgets/ProgressBar.cpp
    Widgets/ProgressBar.h
    Widgets/Rect.cpp
    Widgets/Rect.h
    Widgets/Size.cpp
    Widgets/Size.h
    Widgets/Utils.h
    Widgets/Widget.cpp
    Widgets/Widget.h
)

target_include_directories(PicoWeatherStationFirmware
    PRIVATE
        3rdparty/json/single_include
)

target_link_libraries(PicoWeatherStationFirmware
    pico_stdlib
    hardware_gpio
    hardware_i2c
    hardware_pwm
    hardware_spi
    fmt
    u8g2
)

pico_add_extra_outputs(PicoWeatherStationFirmware)
pico_enable_stdio_usb(PicoWeatherStationFirmware 1)
pico_enable_stdio_uart(PicoWeatherStationFirmware 0)