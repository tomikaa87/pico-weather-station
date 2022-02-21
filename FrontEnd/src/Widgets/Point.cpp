#include "Point.h"

std::ostream& operator<<(std::ostream& os, const Point& p)
{
    os << '{' << p._x << ';' << p._y << '}';

    return os;
}