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
    template<class A>
    Vector2(const Vector2<A>& other)
        : x(other.x), y(other.y)
    {
    }

    //template<class A>
    Vector2<T> operator-(const Vector2<T>& other) const
    {
        return Vector2<T>(x-other.x, y-other.y);
    }
    
    //template<class A>
    Vector2<T> operator/(const T val) const
    {
        return Vector2<T>(x/val, y/val);
    }

    Vector2<T>& operator+=(const T val)
    {
        x+=val;
        y+=val;
        return *this;
    }

    Vector2<T>& operator/=(const T val)
    {
        x/=val;
        y/=val;
        return *this;
    }
};

//inline float DegreesToRadians(float deg) { return deg * M_PI / 180; }

struct Transform
{
    Vector2<float> position;
    float rotation;
};