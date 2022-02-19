#pragma once

namespace Graphics
{
    using ImageData = const unsigned char;

    constexpr auto MainScreen_width = 240;
    constexpr auto MainScreen_height = 160;
    extern ImageData MainScreen_bits[];

    constexpr auto MainScreenInv_width = 240;
    constexpr auto MainScreenInv_height = 160;
    extern ImageData MainScreenInv_bits[];
}