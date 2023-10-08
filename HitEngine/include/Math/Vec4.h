#pragma once

#include "Core/Assert.h"
#include "Core/Types.h"
#include "MathDefines.h"
#include "Vec2.h"
#include "Vec3.h"

namespace hit
{
    union Vec4
    {
        f32 elements[4];
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

            union
            {
                f32 z, b, p, width;
            };
            
            union
            {
                f32 w, a, q, height;
            };
        };

        constexpr inline Vec4();
        inline Vec4(f32 scalar);
        inline Vec4(f32 x, f32 y, f32 z, f32 w);
        inline Vec4(const Vec4&) = default;

        inline Vec4& operator=(const Vec4&) = default;

        inline Vec4& add(f32 scalar);
        inline Vec4& sub(f32 scalar);
        inline Vec4& mul(f32 scalar);
        inline Vec4& div(f32 scalar);

        inline Vec4& add(const Vec4& other);
        inline Vec4& sub(const Vec4& other);
        inline Vec4& mul(const Vec4& other);
        inline Vec4& div(const Vec4& other);

        inline Vec4& operator+=(f32 scalar);
        inline Vec4& operator-=(f32 scalar);
        inline Vec4& operator*=(f32 scalar);
        inline Vec4& operator/=(f32 scalar);

        inline Vec4& operator+=(const Vec4& other);
        inline Vec4& operator-=(const Vec4& other);
        inline Vec4& operator*=(const Vec4& other);
        inline Vec4& operator/=(const Vec4& other);

        inline f32& operator[](ui64 index);
        inline const f32& operator[](ui64 index) const;

        inline bool operator==(const Vec4& other) const;
        inline bool operator!=(const Vec4& other) const;
        inline bool compare_to(const Vec4& other, f32 tolerance = 0.001f) const;

        inline Vec2 to_vec2();
        inline Vec3 to_vec3();

        inline f32 length() const;
        inline Vec4& normalize();
        inline Vec4& absolute();

        inline f32 dot(const Vec4& other) const;
    };

    constexpr inline Vec4::Vec4() : x(0.0f), y(0.0f), z(0.0f), w(0.0f) { }

    inline Vec4::Vec4(f32 scalar) : x(scalar), y(scalar), z(scalar), w(scalar) { }

    inline Vec4::Vec4(f32 x, f32 y, f32 z, f32 w) : x(x), y(y), z(z), w(w) { }

    inline Vec4& Vec4::add(f32 scalar)
    {
        x += scalar;
        y += scalar;
        z += scalar;
        w += scalar;
        return *this;
    }

    inline Vec4& Vec4::sub(f32 scalar)
    {
        x -= scalar;
        y -= scalar;
        z -= scalar;
        w -= scalar;
        return *this;
    }

    inline Vec4& Vec4::mul(f32 scalar)
    {
        x *= scalar;
        y *= scalar;
        z *= scalar;
        w *= scalar;
        return *this;
    }

    inline Vec4& Vec4::div(f32 scalar)
    {
        x /= scalar;
        y /= scalar;
        z /= scalar;
        w /= scalar;
        return *this;
    }

    inline Vec4& Vec4::add(const Vec4& other)
    {
        x += other.x;
        y += other.y;
        z += other.z;
        w += other.w;
        return *this;
    }

    inline Vec4& Vec4::sub(const Vec4& other)
    {
        x -= other.x;
        y -= other.y;
        z -= other.z;
        w -= other.w;
        return *this;
    }

    inline Vec4& Vec4::mul(const Vec4& other)
    {
        x *= other.x;
        y *= other.y;
        z *= other.z;
        w *= other.w;
        return *this;
    }

    inline Vec4& Vec4::div(const Vec4& other)
    {
        x /= other.x;
        y /= other.y;
        z /= other.z;
        w /= other.w;
        return *this;
    }

    inline Vec4& Vec4::operator+=(f32 scalar)
    {
        return add(scalar);
    }

    inline Vec4& Vec4::operator-=(f32 scalar)
    {
        return sub(scalar);
    }

    inline Vec4& Vec4::operator*=(f32 scalar)
    {
        return mul(scalar);
    }

    inline Vec4& Vec4::operator/=(f32 scalar)
    {
        return div(scalar);
    }

    inline Vec4& Vec4::operator+=(const Vec4& other)
    {
        return add(other);
    }

    inline Vec4& Vec4::operator-=(const Vec4& other)
    {
        return sub(other);
    }

    inline Vec4& Vec4::operator*=(const Vec4& other)
    {
        return mul(other);
    }

    inline Vec4& Vec4::operator/=(const Vec4& other)
    {
        return div(other);
    }

    inline f32& Vec4::operator[](ui64 index)
    {
        hit_assert(index <= 3, "Invalid Vec4 index!");
        return elements[index];
    }

    inline const f32& Vec4::operator[](ui64 index) const
    {
        hit_assert(index <= 3, "Invalid Vec4 index!");
        return elements[index];
    }

    inline bool Vec4::operator==(const Vec4& other) const
    {
        return compare_to(other);
    }

    inline bool Vec4::operator!=(const Vec4& other) const
    {
        return !compare_to(other);
    }

    inline bool Vec4::compare_to(const Vec4& other, f32 tolerance) const
    {
        return (std::abs(x - other.x) <= tolerance) && 
               (std::abs(y - other.y) <= tolerance) &&
               (std::abs(z - other.z) <= tolerance) &&
               (std::abs(w - other.w) <= tolerance);
    }

    inline Vec2 Vec4::to_vec2()
    {
        return { x, y };
    }

    inline Vec3 Vec4::to_vec3()
    {
        return { x, y, z };
    }

    inline f32 Vec4::length() const
    {
        return (f32)hsqrt(x * x + y * y + z * z + w * w);
    }

    inline Vec4& Vec4::normalize()
    {
        const f32 len = length();
        x /= len;
        y /= len;
        z /= len;
        w /= len;
        return *this;
    }

    inline Vec4& Vec4::absolute()
    {
        x = std::abs(x);
        y = std::abs(y);
        z = std::abs(z);
        w = std::abs(w);
        return *this;
    }

    inline f32 Vec4::dot(const Vec4& other) const
    {
        return x * other.x + y * other.y + z * other.z + w * other.w;
    }

    inline Vec4 operator+(const Vec4& v)
    {
        return { v.x, v.y, v.z, v.w };
    }

    inline Vec4 operator-(const Vec4& v)
    {
        return { -v.x, -v.y, -v.z, -v.w };
    }

    inline Vec4 operator+(const Vec4& v1, const Vec4& v2)
    {
        return { v1.x + v2.x, v1.y + v2.y, v1.z + v2.z, v1.w + v2.w };
    }

    inline Vec4 operator-(const Vec4& v1, const Vec4& v2)
    {
        return { v1.x - v2.x, v1.y - v2.y, v1.z - v2.z, v1.w - v2.w };
    }

    inline Vec4 operator*(const Vec4& v1, const Vec4& v2)
    {
        return { v1.x * v2.x, v1.y * v2.y, v1.z * v2.z, v1.w * v2.w };
    }

    inline Vec4 operator/(const Vec4& v1, const Vec4& v2)
    {
        return { v1.x / v2.x, v1.y / v2.y, v1.z / v2.z, v1.w / v2.w };
    }

    inline Vec4 operator*(const Vec4& v, f32 scalar)
    {
        return { v.x * scalar, v.y * scalar, v.z * scalar, v.w * scalar };
    }

    inline Vec4 operator/(const Vec4& v, f32 scalar)
    {
        return { v.x / scalar, v.y / scalar, v.z / scalar, v.w / scalar };
    }
}