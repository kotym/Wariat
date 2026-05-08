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
    inline constexpr Vector2(T _x, T _y)
        : x(_x), y(_y)
    {
    }
    template<class A>
    inline Vector2(const Vector2<A>& other)
        : x(other.x), y(other.y)
    {
    }

    void Negate()
    {
        x = -x;
        y = -y;
    }

    inline float LengthSq() const
    {
        return x*x+y*y;
    }

    inline float Length() const
    {
        return sqrtf(LengthSq());
    }

    //template<class A>
    inline Vector2<T> operator-() const
    {
        return Vector2<T>(-x, -y);
    }

    //template<class A>
    inline Vector2<T> operator-(const Vector2<T>& other) const
    {
        return Vector2<T>(x - other.x, y - other.y);
    }

    //template<class A>
    inline Vector2<T> operator+(const Vector2<T>& other) const
    {
        return Vector2<T>(x + other.x, y + other.y);
    }

    //template<class A>
    inline Vector2<T> operator*(const Vector2<T>& other) const
    {
        return Vector2<T>(x * other.x, y * other.y);
    }
    
    //template<class A>
    inline Vector2<T> operator*(const T val) const
    {
        return Vector2<T>(x * val, y * val);
    }
    
    //template<class A>
    inline Vector2<T> operator/(const T val) const
    {
        return Vector2<T>(x / val, y / val);
    }

    inline Vector2<T>& operator+=(const T val)
    {
        x += val;
        y += val;
        return *this;
    }

    inline Vector2<T>& operator-=(const T val)
    {
        x -= val;
        y -= val;
        return *this;
    }

    inline Vector2<T>& operator*=(const T val)
    {
        x *= val;
        y *= val;
        return *this;
    }

    inline Vector2<T>& operator/=(const T val)
    {
        x /= val;
        y /= val;
        return *this;
    }

    inline bool operator==(const Vector2<T>& other) const
    {
        return x == other.x && y == other.y;
    }

    inline bool operator==(const T val) const
    {
        return x == val && y == val;
    }
};

//inline float DegreesToRadians(float deg) { return deg * M_PI / 180; }

struct Transform
{
    Vector2<float> position;
    float rotation;
};