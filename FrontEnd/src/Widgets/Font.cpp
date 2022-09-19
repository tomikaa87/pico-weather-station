#include "Font.h"

#include "../Fonts.h"

#include <U8g2lib.h>

Font::Font(
    Family family,
    Style style
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

        case Family::VcrOsd:
            return u8g2_font_VCR_OSD_tr;

        case Family::RpgSystem:
            return Fonts::RpgSystem;

        case Family::Pxl16x8:
            return Fonts::Pxl16x8;

        case Family::Pxl16x8_x2:
            return Fonts::Pxl16x8_x2;

        case Family::Pxl16x8_Mono:
            return Fonts::Pxl16x8_Mono;

        case Family::Pxl16x8_Mono_x2:
            return Fonts::Pxl16x8_Mono_x2;

        case Family::BitCell:
            return Fonts::BitCell;

        case Family::BitCellMonoNumbers:
            return Fonts::BitCellMonoNumbers;
    }

    return nullptr;
}