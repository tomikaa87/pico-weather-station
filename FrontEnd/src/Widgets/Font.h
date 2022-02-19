#pragma once

#include <cstdint>

class Font
{
public:
    enum class Family
    {
        PfTempesta7,
        P01Type
    };

    enum class Style
    {
        Regular,
        Condensed,
        Compressed
    };

    Font() = default;

    Font(Family family, Style style);

    void setFamily(Family family);
    void setStyle(Style style);
    void setBold(bool bold);

    const uint8_t* data() const;

private:
    Family _family = Family::PfTempesta7;
    Style _style = Style::Condensed;
    bool _bold = false;
};