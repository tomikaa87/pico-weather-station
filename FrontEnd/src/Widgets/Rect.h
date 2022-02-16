#pragma once

#include "Point.h"
#include "Size.h"

class Rect
{
public:
    Rect() = default;
    Rect(int x, int y, int w, int h);
    Rect(Point pos, Size size);
    Rect(const Rect&) = default;
    Rect(Rect&&) = default;

    [[nodiscard]] bool isNull() const;

    [[nodiscard]] int x() const;
    [[nodiscard]] int y() const;
    [[nodiscard]] int w() const;
    [[nodiscard]] int h() const;

    [[nodiscard]] const Point& pos() const;
    [[nodiscard]] const Size& size() const;

    void setPos(Point pos);
    void setSize(Size size);

    [[nodiscard]] Rect adjusted(int dx1, int dy1, int dx2, int dy2) const;

    Rect& operator=(const Rect&) = default;
    Rect& operator=(Rect&&) = default;
    [[nodiscard]] bool operator==(const Rect& r) const;
    [[nodiscard]] bool operator!=(const Rect& r) const;

private:
    Point _pos;
    Size _size;
};