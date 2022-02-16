#pragma once

class Point
{
public:
    Point() = default;
    Point(const int x, const int y)
        : _x{ x }
        , _y{ y }
    {}

    int x() const
    {
        return _x;
    }

    int y() const
    {
        return _y;
    }

private:
    int _x = 0;
    int _y = 0;
};