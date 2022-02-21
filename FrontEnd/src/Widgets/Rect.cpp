#include "Rect.h"

#include "Utils.h"

Rect Rect::operator|(const Rect& r) const noexcept
{
    if (isNull()) {
        return r;
    }

    if (r.isNull()) {
        return *this;
    }

    int l1 = _x1;
    int r1 = _x1 - 1;
    if (_x2 < _x1 - 1)
        l1 = _x2 + 1;
    else
        r1 = _x2;

    int l2 = r._x1;
    int r2 = r._x1 - 1;
    if (r._x2 < r._x1 - 1)
        l2 = r._x2 + 1;
    else
        r2 = r._x2;

    int t1 = _y1;
    int b1 = _y1 - 1;
    if (_y2 < _y1 - 1)
        t1 = _y2 + 1;
    else
        b1 = _y2;

    int t2 = r._y1;
    int b2 = r._y1 - 1;
    if (r._y2 < r._y1 - 1)
        t2 = r._y2 + 1;
    else
        b2 = r._y2;

    Rect tmp;
    tmp._x1 = Utils::min(l1, l2);
    tmp._x2 = Utils::max(r1, r2);
    tmp._y1 = Utils::min(t1, t2);
    tmp._y2 = Utils::max(b1, b2);

    return tmp;
}

Rect Rect::operator&(const Rect& r) const noexcept
{
    if (isNull() || r.isNull()) {
        return Rect{};
    }

    int l1 = _x1;
    int r1 = _x2;
    if (_x2 < _x1 - 1) {
        l1 = _x2 + 1;
        r1 = _x1 - 1;
    }

    int l2 = r._x1;
    int r2 = r._x2;
    if (r._x2 < r._x1 - 1) {
        l2 = r._x2 + 1;
        r2 = r._x1 - 1;
    }

    if (l1 > r2 || l2 > r1) {
        return Rect{};
    }

    int t1 = _y1;
    int b1 = _y2;
    if (_y2 < _y1 - 1) {
        t1 = _y2 + 1;
        b1 = _y1 - 1;
    }

    int t2 = r._y1;
    int b2 = r._y2;
    if (r._y2 < r._y1 - 1) {
        t2 = r._y2 + 1;
        b2 = r._y1 - 1;
    }

    if (t1 > b2 || t2 > b1) {
        return Rect{};
    }

    Rect tmp;
    tmp._x1 = Utils::max(l1, l2);
    tmp._x2 = Utils::min(r1, r2);
    tmp._y1 = Utils::max(t1, t2);
    tmp._y2 = Utils::min(b1, b2);

    return tmp;
}

std::ostream& operator<<(std::ostream& os, const Rect& r)
{
    os << '{'
        << r.x() << ';' << r.y()
        << ' '
        << r.width() << 'x' << r.height()
        << '}';

    return os;
}