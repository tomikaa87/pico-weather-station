project(oled-widgets)
cmake_minimum_required(VERSION 3.18)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(U8G2_DIR ${CMAKE_SOURCE_DIR}/dependencies/u8g2)
set(WIRING_PI_DIR ${CMAKE_SOURCE_DIR}/dependencies/wiringpi)

add_executable(oled
    ${CMAKE_SOURCE_DIR}/src/main.cpp
    ${CMAKE_SOURCE_DIR}/src/ProgressBar.cpp
    ${CMAKE_SOURCE_DIR}/src/ProgressBar.h
    ${CMAKE_SOURCE_DIR}/src/u8x8_d_st7586s_erc240160_chunked.c
)

# u8g2 library
file(GLOB U8G2_CSOURCES "${U8G2_DIR}/csrc/*.c")
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

target_link_libraries(oled
    PRIVATE
        u8g2
        wiringpi
)