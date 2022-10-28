#pragma once

namespace Utils
{
    template <typename ValueType>
    constexpr ValueType abs(const ValueType n) noexcept
    {
        return n < 0 ? -n : n;
    }

    template <typename ValueType>
    constexpr ValueType min(const ValueType a, const ValueType b) noexcept
    {
        return a < b ? a : b;
    }

    template <typename ValueType>
    constexpr ValueType max(const ValueType a, const ValueType b) noexcept
    {
        return a > b ? a : b;
    }

    template <typename ValueType>
    constexpr ValueType clamp(const ValueType value, const ValueType rangeMin, const ValueType rangeMax)
    {
        return max(rangeMin, min(rangeMax, value));
    }
}