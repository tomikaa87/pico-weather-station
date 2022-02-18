#include "Rect.h"

#include <algorithm>
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

Rect Rect::operator&(const Rect& r) const
{
    int left1 = x();
    int right1 = x();

    if (w() < 0) {
        left1 += w();
    } else {
        right1 += w();
    }

    if (left1 == right1) {
        return {};
    }

    int left2 = r.x();
    int right2 = r.x();

    if (r.w() < 0) {
        left2 += r.w();
    } else {
        right2 += r.w();
    }

    if (left2 == right2) {
        return {};
    }

    int top1 = y();
    int bottom1 = y();

    if (h() < 0) {
        top1 += h();
    } else {
        bottom1 += h();
    }

    if (top1 == bottom1) {
        return {};
    }

    int top2 = r.y();
    int bottom2 = r.y();

    if (r.h() < 0) {
        top2 += r.h();
    } else {
        bottom2 += r.h();
    }

    if (top2 == bottom2) {
        return {};
    }

    return Rect{
        std::max(left1, left2),
        std::max(top1, top2),
        std::min(right1, right2) - std::max(left1, left2),
        std::min(bottom1, bottom2) - std::max(top1, top2)
    };
}

Rect Rect::operator|(const Rect& r) const
{
    if (isNull()) {
        return r;
    }

    if (r.isNull()) {
        return *this;
    }

    int left = x();
    int right = x();

    if (w() < 0) {
        left += w();
    } else {
        right += w();
    }

    if (r.w() < 0) {
        left = std::min(left, r.x() + r.w());
        right = std::max(right, r.x());
    } else {
        left = std::min(left, r.x());
        right = std::max(right, r.x() + r.w());
    }

    int top = y();
    int bottom = y();

    if (h() < 0) {
        top += h();
    } else {
        bottom += h();
    }

    if (r.h() < 0) {
        top = std::min(top, r.y() + r.h());
        bottom = std::max(bottom, r.y() + r.h());
    }

    return Rect{
        left,
        top,
        right - left,
        bottom - top
    };
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

void Rect::setWidth(const int width)
{
    _size.setWidth(width);
}

void Rect::setHeight(const int height)
{
    _size.setHeight(height);
}

Rect Rect::adjusted(int dx1, int dy1, int dx2, int dy2) const
{
    // TODO
    return{};
}