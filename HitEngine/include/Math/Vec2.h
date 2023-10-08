#pragma once

#include "Core/Assert.h"
#include "Core/Types.h"
#include "MathDefines.h"

namespace hit
{
    union Vec2
    {
        f32 elements[2];
        struct 
        {
            union
            {
                f32 x, r, s, u;
            };

            union
            {
                f32 y, g, t, v;
            };
        };

        constexpr inline Vec2();
        inline Vec2(f32 scalar);
        inline Vec2(f32 x, f32 y);
        inline Vec2(const Vec2&) = default;

        inline Vec2& operator=(const Vec2&) = default;

        inline Vec2& add(f32 scalar);
        inline Vec2& sub(f32 scalar);
        inline Vec2& mul(f32 scalar);
        inline Vec2& div(f32 scalar);

        inline Vec2& add(const Vec2& other);
        inline Vec2& sub(const Vec2& other);
        inline Vec2& mul(const Vec2& other);
        inline Vec2& div(const Vec2& other);

        inline Vec2& operator+=(f32 scalar);
        inline Vec2& operator-=(f32 scalar);
        inline Vec2& operator*=(f32 scalar);
        inline Vec2& operator/=(f32 scalar);

        inline Vec2& operator+=(const Vec2& other);
        inline Vec2& operator-=(const Vec2& other);
        inline Vec2& operator*=(const Vec2& other);
        inline Vec2& operator/=(const Vec2& other);

        inline f32& operator[](ui64 index);
        inline const f32& operator[](ui64 index) const;

        inline bool operator==(const Vec2& other) const;
        inline bool operator!=(const Vec2& other) const;
        inline bool compare_to(const Vec2& other, f32 tolerance = 0.001f) const;

        inline f32 length() const;
        inline Vec2& normalize();
        inline Vec2& absolute();

        inline f32 dot(const Vec2& other) const;
        inline f32 cross(const Vec2& other) const;
    };

    constexpr inline Vec2::Vec2() : x(0.0f), y(0.0f) { }

    inline Vec2::Vec2(f32 scalar) : x(scalar), y(scalar) { }

    inline Vec2::Vec2(f32 x, f32 y) : x(x), y(y) { }

    inline Vec2& Vec2::add(f32 scalar)
    {
        x += scalar;
        y += scalar;
        return *this;
    }

    inline Vec2& Vec2::sub(f32 scalar)
    {
        x -= scalar;
        y -= scalar;
        return *this;
    }

    inline Vec2& Vec2::mul(f32 scalar)
    {
        x *= scalar;
        y *= scalar;
        return *this;
    }

    inline Vec2& Vec2::div(f32 scalar)
    {
        x /= scalar;
        y /= scalar;
        return *this;
    }


    inline Vec2& Vec2::add(const Vec2& other)
    {
        x += other.x;
        y += other.y;
        return *this;
    }

    inline Vec2& Vec2::sub(const Vec2& other)
    {
        x -= other.x;
        y -= other.y;
        return *this;
    }

    inline Vec2& Vec2::mul(const Vec2& other)
    {
        x *= other.x;
        y *= other.y;
        return *this;
    }

    inline Vec2& Vec2::div(const Vec2& other)
    {
        x /= other.x;
        y /= other.y;
        return *this;
    }

    inline Vec2& Vec2::operator+=(f32 scalar)
    {
        return add(scalar);
    }

    inline Vec2& Vec2::operator-=(f32 scalar)
    {
        return sub(scalar);
    }

    inline Vec2& Vec2::operator*=(f32 scalar)
    {
        return mul(scalar);
    }

    inline Vec2& Vec2::operator/=(f32 scalar)
    {
        return div(scalar);
    }

    inline Vec2& Vec2::operator+=(const Vec2& other)
    {
        return add(other);
    }

    inline Vec2& Vec2::operator-=(const Vec2& other)
    {
        return sub(other);
    }

    inline Vec2& Vec2::operator*=(const Vec2& other)
    {
        return mul(other);
    }

    inline Vec2& Vec2::operator/=(const Vec2& other)
    {
        return div(other);
    }

    inline f32& Vec2::operator[](ui64 index)
    {
        hit_assert(index <= 1, "Invalid Vec2 index!");
        return elements[index];
    }

    inline const f32& Vec2::operator[](ui64 index) const
    {
        hit_assert(index <= 1, "Invalid Vec2 index!");
        return elements[index];
    }

    inline bool Vec2::operator==(const Vec2& other) const
    {
        return compare_to(other);
    }

    inline bool Vec2::operator!=(const Vec2& other) const
    {
        return !compare_to(other);
    }

    inline bool Vec2::compare_to(const Vec2& other, f32 tolerance) const
    {
        return (std::abs(x - other.x) <= tolerance) && (std::abs(y - other.y) <= tolerance);
    }

    inline f32 Vec2::length() const
    {
        return (f32)hsqrt(x * x + y * y);
    }

    inline Vec2& Vec2::normalize()
    {
        const f32 len = length();
        x /= len;
        y /= len;
        return *this;
    }

    inline Vec2& Vec2::absolute()
    {
        x = std::abs(x);
        y = std::abs(y);
        return *this;
    }

    inline f32 Vec2::dot(const Vec2& other) const
    {
        return x * other.x + y * other.y;
    }

    inline f32 Vec2::cross(const Vec2& other) const
    {
        return x * other.y - y * other.x;
    }

    inline Vec2 operator+(const Vec2& v)
    {
        return { v.x, v.y };
    }

    inline Vec2 operator-(const Vec2& v)
    {
        return { -v.x, -v.y };
    }

    inline Vec2 operator+(const Vec2& v1, const Vec2& v2)
    {
        return { v1.x + v2.x, v1.y + v2.y };
    }

    inline Vec2 operator-(const Vec2& v1, const Vec2& v2)
    {
        return { v1.x - v2.x, v1.y - v2.y };
    }

    inline Vec2 operator*(const Vec2& v1, const Vec2& v2)
    {
        return { v1.x * v2.x, v1.y * v2.y };
    }

    inline Vec2 operator/(const Vec2& v1, const Vec2& v2)
    {
        return { v1.x / v2.x, v1.y / v2.y };
    }

    inline Vec2 operator*(const Vec2& v, f32 scalar)
    {
        return { v.x * scalar, v.y * scalar };
    }

    inline Vec2 operator/(const Vec2& v, f32 scalar)
    {
        return { v.x / scalar, v.y / scalar };
    }
}