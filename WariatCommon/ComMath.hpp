#pragma once

#define _USE_MATH_DEFINES
#include <cmath>
#include <cstdint>
#include <corecrt_math_defines.h>

namespace WariatMath
{

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

    inline Vector2<T>& operator=(const T val)
    {
        x = y = val;
        return *this;
    }
    
    //template<class A>
    inline Vector2<T> operator/(const T val) const
    {
        return Vector2<T>(x / val, y / val);
    }

    inline Vector2<T>& operator+=(const Vector2<T>& other)
    {
        x += other.x;
        y += other.y;
        return *this;
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

    template<class T>
    inline Vector2<T>& Normalize()
    {
        const float length = Length();
        x /= length;
        y /= length;
        return *this;
    }

    template<class T>
    inline Vector2<T> GetNormalized() const
    {
        const float length = Length();
        return Vector2<T>(x/length, y/length);
    }

    template<class T>
    inline static float Dot(Vector2<T> a, Vector2<T> b)
    {
        return a.x * b.x + a.y * b.y;
    }

    template<class T>
    inline static float Angle(Vector2<T> a, Vector2<T> b)
    {
        return acosf(Dot(a.Normalize(), b.Normalize()));
    }
};

//inline float DegreesToRadians(float deg) { return deg * M_PI / 180; }

struct Transform
{
    Vector2<float> position;
    float rotation;
};

inline float NormalizeAngle(float angle)
{
    constexpr float twoPi = static_cast<float>(2.0 * M_PI);
    constexpr float pi = static_cast<float>(M_PI);
    float normalized = remainderf(angle, twoPi);
    if (normalized <= -pi)
    {
        normalized = pi;
    }
    return normalized;
}

}

using namespace WariatMath;