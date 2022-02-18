project(oled-widgets)
cmake_minimum_required(VERSION 3.18)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(U8G2_DIR ${CMAKE_SOURCE_DIR}/dependencies/u8g2)
set(WIRING_PI_DIR ${CMAKE_SOURCE_DIR}/dependencies/wiringpi)
set(FMT_DIR ${CMAKE_SOURCE_DIR}/dependencies/fmt)

add_executable(oled
    ${CMAKE_SOURCE_DIR}/src/main.cpp
    ${CMAKE_SOURCE_DIR}/src/Fonts.cpp
    ${CMAKE_SOURCE_DIR}/src/Fonts.h
    ${CMAKE_SOURCE_DIR}/src/Graphics.cpp
    ${CMAKE_SOURCE_DIR}/src/Graphics.h
    ${CMAKE_SOURCE_DIR}/src/u8x8_d_st7586s_erc240160_chunked.c

    ${CMAKE_SOURCE_DIR}/src/Widgets/Display.cpp
    ${CMAKE_SOURCE_DIR}/src/Widgets/Display.h
    ${CMAKE_SOURCE_DIR}/src/Widgets/Font.cpp
    ${CMAKE_SOURCE_DIR}/src/Widgets/Font.h
    ${CMAKE_SOURCE_DIR}/src/Widgets/Image.cpp
    ${CMAKE_SOURCE_DIR}/src/Widgets/Image.h
    ${CMAKE_SOURCE_DIR}/src/Widgets/Label.cpp
    ${CMAKE_SOURCE_DIR}/src/Widgets/Label.h
    ${CMAKE_SOURCE_DIR}/src/Widgets/Rect.h
    ${CMAKE_SOURCE_DIR}/src/Widgets/Painter.cpp
    ${CMAKE_SOURCE_DIR}/src/Widgets/Painter.h
    ${CMAKE_SOURCE_DIR}/src/Widgets/Point.cpp
    ${CMAKE_SOURCE_DIR}/src/Widgets/Point.h
    ${CMAKE_SOURCE_DIR}/src/Widgets/ProgressBar.cpp
    ${CMAKE_SOURCE_DIR}/src/Widgets/ProgressBar.h
    ${CMAKE_SOURCE_DIR}/src/Widgets/Rect.cpp
    ${CMAKE_SOURCE_DIR}/src/Widgets/Rect.h
    ${CMAKE_SOURCE_DIR}/src/Widgets/Size.cpp
    ${CMAKE_SOURCE_DIR}/src/Widgets/Size.h
    ${CMAKE_SOURCE_DIR}/src/Widgets/Widget.cpp
    ${CMAKE_SOURCE_DIR}/src/Widgets/Widget.h
)

# u8g2 library
file(GLOB U8G2_CSOURCES
    "${U8G2_DIR}/csrc/*.c"
    "${U8G2_DIR}/cppsrc/*.cpp"
)
add_library(u8g2 ${U8G2_CSOURCES})
target_include_directories(u8g2
    PUBLIC
        ${U8G2_DIR}/csrc
        ${U8G2_DIR}/cppsrc
)

# WiringPi library
file(GLOB WIRINGPI_SOURCES "${WIRING_PI_DIR}/wiringPi/*.c")
add_library(wiringpi ${WIRINGPI_SOURCES})
target_include_directories(wiringpi
    PUBLIC
        ${WIRING_PI_DIR}/wiringPi
)
target_link_libraries(wiringpi
    PRIVATE
        pthread
)

# FMT library
add_library(fmt
    ${FMT_DIR}/src/format.cc
    ${FMT_DIR}/src/os.cc
)
target_include_directories(fmt
    PUBLIC
        ${FMT_DIR}/include
)

target_link_libraries(oled
    PRIVATE
        fmt
        u8g2
        wiringpi
)

target_compile_definitions(oled
    PRIVATE
        RASPBERRY_PI
        WIDGET_DEBUG=1
)