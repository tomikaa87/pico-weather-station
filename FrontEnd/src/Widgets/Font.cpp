#include "Font.h"

#include "../Fonts.h"

#include <u8g2.h>

Font::Font(
    Family family = Family::PfTempesta7,
    Style style = Style::Condensed
)
    : _family{ family }
    , _style{ style }
{}

void Font::setFamily(const Family family)
{
    _family = family;
}

void Font::setStyle(const Style style)
{
    _style = style;
}

void Font::setBold(const bool bold)
{
    _bold = bold;
}

const uint8_t* Font::data() const
{
    switch (_family) {
        case Family::PfTempesta7: {
            switch (_style) {
                case Style::Regular:
                    return _bold
                        ? Fonts::PfTempesta7Bold
                        : Fonts::PfTempesta7;

                case Style::Compressed:
                    return _bold
                        ? Fonts::PfTempesta7CompressedBold
                        : Fonts::PfTempesta7CompressedBold;

                case Style::Condensed:
                default:
                    return _bold
                        ? Fonts::PfTempesta7CondensedBold
                        : Fonts::PfTempesta7Condensed;
            }
        }

        case Family::P01Type:
            return u8g2_font_p01type_tn;
    }

    return nullptr;
}