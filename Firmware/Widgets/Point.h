#pragma once

#include "Utils.h"

#include <ostream>

class Point
{
public:
    constexpr Point() noexcept = default;

    constexpr Point(const int x, const int y) noexcept
        : _x{ x }
        , _y{ y }
    {}

    constexpr inline bool isNull() const noexcept
    {
        return _x == 0 && _y == 0;
    }

    [[nodiscard]] constexpr inline int x() const noexcept
    {
        return _x;
    }

    [[nodiscard]] constexpr inline int y() const noexcept
    {
        return _y;
    }

    constexpr inline void setX(const int x) noexcept
    {
        _x = x;
    }

    constexpr inline void setY(const int y) noexcept
    {
        _y = y;
    }

    [[nodiscard]] constexpr inline int& rx() noexcept
    {
        return _x;
    }

    [[nodiscard]] constexpr inline int& ry() noexcept
    {
        return _y;
    }

    [[nodiscard]] constexpr inline int manhattanLength() const
    {
        return Utils::abs(x()) + Utils::abs(y());
    }

    constexpr inline Point& operator+=(const Point& p) noexcept
    {
        _x += p._x;
        _y += p._y;
        return *this;
    }

    constexpr inline Point& operator-=(const Point& p) noexcept
    {
        _x -= p._x;
        _y -= p._y;
        return *this;
    }

    constexpr inline Point& operator*=(const int factor) noexcept
    {
        _x *= factor;
        _y *= factor;
        return *this;
    }

    [[nodiscard]] friend constexpr inline bool operator==(const Point& p1, const Point& p2) noexcept
    {
        return p1._x == p2._x && p1._y == p2._y;
    }

    [[nodiscard]] friend constexpr inline bool operator!=(const Point& p1, const Point& p2) noexcept
    {
        return p1._x != p2._x || p1._y != p2._y;
    }

    [[nodiscard]] friend constexpr inline Point operator+(const Point& p1, const Point& p2) noexcept
    {
        return Point{ p1._x + p2._x, p1._y + p2._y };
    }

    [[nodiscard]] friend constexpr inline Point operator-(const Point& p1, const Point& p2) noexcept
    {
        return Point{ p1._x - p2._x, p1._y - p2._y };
    }

    [[nodiscard]] friend constexpr inline Point operator*(const Point& p, int factor) noexcept
    {
        return Point{ p._x * factor, p._y * factor };
    }

    [[nodiscard]] friend constexpr inline Point operator*(const int factor, const Point& p) noexcept
    {
        return Point{ p._x * factor, p._y * factor };
    }

    [[nodiscard]] friend constexpr inline Point operator+(const Point& p) noexcept
    {
        return p;
    }

    [[nodiscard]] friend constexpr inline Point operator-(const Point& p) noexcept
    {
        return Point{ -p._x, -p._y };
    }

    friend std::ostream& operator<<(std::ostream& os, const Point& p);

private:
    int _x = 0;
    int _y = 0;
};