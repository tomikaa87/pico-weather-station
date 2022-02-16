#include "Font.h"

#include "../Fonts.h"

#include <u8g2.h>

Font::Font(const Family family)
    : _family{ family }
{}

void Font::setFamily(const Family family)
{
    _family = family;
}

const uint8_t* Font::data() const
{
    switch (_family) {
        case Family::NormalText:
            return Fonts::PFT7Condensed;

        case Family::TinyNumbers:
            return u8g2_font_p01type_tn;
    }

    return nullptr;
}