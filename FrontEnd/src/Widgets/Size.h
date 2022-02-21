#pragma once

#include <ostream>

class Size
{
public:
    constexpr inline Size() noexcept = default;

    constexpr inline Size(const int width, const int height) noexcept
        : _w{ width }
        , _h{ height }
    {}

    [[nodiscard]] constexpr inline bool isNull() const noexcept
    {
        return _w == 0 && _h == 0;
    }

    [[nodiscard]] constexpr inline bool isEmpty() const noexcept
    {
        return _w < 1 || _h < 1;
    }

    [[nodiscard]] constexpr inline bool isValid() const noexcept
    {
        return _w >= 0 && _h >= 0;
    }

    [[nodiscard]] constexpr inline int width() const noexcept
    {
        return _w;
    }

    [[nodiscard]] constexpr inline int height() const noexcept
    {
        return _h;
    }

    constexpr inline void setWidth(const int w) noexcept
    {
        _w = w;
    }

    constexpr inline void setHeight(const int h) noexcept
    {
        _h = h;
    }

    [[nodiscard]] constexpr inline int& rWidth() noexcept
    {
        return _w;
    }

    [[nodiscard]] constexpr inline int& rHeight() noexcept
    {
        return _h;
    }

    constexpr void transpose() noexcept
    {
        const int tmp = _w;
        _w = _h;
        _h = tmp;
    }

    [[nodiscard]] constexpr inline Size transposed() const noexcept
    {
        return Size{ _h, _w };
    }

    constexpr inline Size& operator+=(const Size& s) noexcept
    {
        _w += s._w;
        _h += s._h;
        return *this;
    }

    constexpr inline Size& operator-=(const Size& s) noexcept
    {
        _w -= s._w;
        _h -= s._h;
        return *this;
    }

    constexpr inline Size& operator*=(const int factor) noexcept
    {
        _w *= factor;
        _h *= factor;
        return *this;
    }

    friend inline constexpr bool operator==(const Size& s1, const Size& s2) noexcept
    {
        return s1._w == s2._w && s1._h == s2._h;
    }

    friend inline constexpr bool operator!=(const Size& s1, const Size& s2) noexcept
    {
        return s1._w != s2._w || s1._h != s2._h;
    }

    [[nodiscard]] friend inline constexpr Size operator+(const Size& s1, const Size& s2) noexcept
    {
        return Size{ s1._w + s2._w, s1._h + s2._h };
    }

    [[nodiscard]] friend inline constexpr Size operator-(const Size& s1, const Size& s2) noexcept
    {
        return Size{ s1._w - s2._w, s1._h - s2._h };
    }

    friend std::ostream& operator<<(std::ostream& os, const Size& s);

private:
    int _w = 0;
    int _h = 0;
};