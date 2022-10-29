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

add_subdirectory(
    3rdparty/u8g2
)

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
        3rdparty/fmt/include
        3rdparty/json/single_include
        3rdparty/u8g2/cppsrc
        3rdparty/u8g2/csrc
)

target_link_libraries(PicoWeatherStationFirmware
    pico_stdlib
    hardware_gpio
    hardware_i2c
    hardware_pwm
    hardware_spi
    u8g2
)

pico_add_extra_outputs(PicoWeatherStationFirmware)
pico_enable_stdio_usb(PicoWeatherStationFirmware 1)
pico_enable_stdio_uart(PicoWeatherStationFirmware 0)