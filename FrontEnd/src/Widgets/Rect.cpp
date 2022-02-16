#include "Rect.h"

#include <utility>

Rect::Rect(
    const int x,
    const int y,
    const int w,
    const int h
)
    : _pos{ x, y }
    , _size{ w, h }
{}

Rect::Rect(
    Point pos,
    Size size
)
    : _pos{ std::move(pos) }
    , _size{ std::move(size) }
{}

bool Rect::isNull() const
{
    return _size.isNull();
}

bool Rect::operator==(const Rect& r) const
{
    return
        _pos == r.pos()
        && _size == r.size();
}

bool Rect::operator!=(const Rect& r) const
{
    return !(*this == r);
}

int Rect::x() const
{
    return _pos.x();
}

int Rect::y() const
{
    return _pos.y();
}

int Rect::w() const
{
    return _size.width();
}

int Rect::h() const
{
    return _size.height();
}

const Point& Rect::pos() const
{
    return _pos;
}

const Size& Rect::size() const
{
    return _size;
}

void Rect::setPos(Point pos)
{
    _pos = std::move(pos);
}

void Rect::setSize(Size size)
{
    _size = std::move(size);
}
