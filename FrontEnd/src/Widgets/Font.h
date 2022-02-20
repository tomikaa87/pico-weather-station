#pragma once

#include <cstdint>

class Font
{
public:
    enum class Family
    {
        PfTempesta7,
        P01Type,
        VcrOsd,
        RpgSystem,
        Pxl16x8,
        Pxl16x8_x2,
        Pxl16x8_Mono,
        Pxl16x8_Mono_x2
    };

    enum class Style
    {
        Regular,
        Condensed,
        Compressed
    };

    Font() = default;

    explicit Font(
        Family family,
        Style style = Style::Regular
    );

    void setFamily(Family family);
    void setStyle(Style style);
    void setBold(bool bold);

    const uint8_t* data() const;

private:
    Family _family = Family::PfTempesta7;
    Style _style = Style::Condensed;
    bool _bold = false;
};