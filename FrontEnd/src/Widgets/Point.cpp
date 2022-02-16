#include "Point.h"

Point::Point(const int x, const int y)
    : _x{ x }
    , _y{ y }
{}

int Point::x() const
{
    return _x;
}

int Point::y() const
{
    return _y;
}

bool Point::operator==(const Point& p) const
{
    return
        _x == p._x
        && _y == p._y;
}

bool Point::operator!=(const Point& p) const
{
    return !(*this == p);
}