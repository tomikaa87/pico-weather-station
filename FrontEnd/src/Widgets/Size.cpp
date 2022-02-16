#include "Size.h"

Size::Size(const int w, const int h)
    : _w{ w }
    , _h{ h }
{}

bool Size::isNull() const
{
    return _w == 0 && _h == 0;
}

int Size::width() const
{
    return _w;
}

int Size::height() const
{
    return _h;
}

bool Size::operator==(const Size& s) const
{
    return
        _w == s._w
        && _h == s._h;
}

bool Size::operator!=(const Size& s) const
{
    return !(*this == s);
}