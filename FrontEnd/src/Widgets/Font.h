#pragma once

#include <cstdint>

class Font
{
public:
    enum class Family
    {
        NormalText,
        TinyNumbers
    };

    Font() = default;
    explicit Font(Family family);

    void setFamily(Family family);

    const uint8_t* data() const;

private:
    Family _family = Family::NormalText;
};