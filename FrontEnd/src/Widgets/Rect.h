#pragma once

class Rect
{
public:
    Rect() = default;
    Rect(
        const int x,
        const int y,
        const int w,
        const int h
    )
        : _x{ x }
        , _y{ y }
        , _w{ w }
        , _h{ h }
    {}

    [[nodiscard]] bool isNull() const
    {
        return _w <= 0 && _h <= 0;
    }

    [[nodiscard]] bool operator==(const Rect& r) const
    {
        return
            _x == r._x
            && _y == r._y
            && _w == r._w
            && _h == r._h;
    }

    [[nodiscard]] bool operator!=(const Rect& r) const
    {
        return !(*this == r);
    }

    [[nodiscard]] int x() const
    {
        return _x;
    }

    [[nodiscard]] int y() const
    {
        return _y;
    }

    [[nodiscard]] int w() const
    {
        return _w;
    }

    [[nodiscard]] int h() const
    {
        return _h;
    }

private:
    int _x = 0;
    int _y = 0;
    int _w = 0;
    int _h = 0;
};