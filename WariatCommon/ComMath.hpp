#pragma once

#include <cstdint>
#define _USE_MATH_DEFINES
#include <cmath>

template <typename T>
class Vector2{
public:
    T x{};
    T y{};

    constexpr Vector2() = default;
    constexpr Vector2(T _x, T _y)
        : x(_x), y(_y)
    {
    }

    Vector2<T> operator-(const Vector2<T>& other)
    {
        return Vector2<T>(x-other.x, y-other.y);
    }
};