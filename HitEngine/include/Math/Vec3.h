#pragma once

#include "Core/Assert.h"
#include "Core/Types.h"
#include "MathDefines.h"
#include "Vec2.h"

namespace hit
{
    union Vec3
    {
        f32 elements[3];
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
                f32 z, b, p, w;
            };
            
        };

        constexpr inline Vec3();
        inline Vec3(f32 scalar);
        inline Vec3(f32 x, f32 y, f32 z);
        inline Vec3(const Vec3&) = default;

        constexpr inline Vec3& operator=(const Vec3&) = default;

        inline Vec3& add(f32 scalar);
        inline Vec3& sub(f32 scalar);
        inline Vec3& mul(f32 scalar);
        inline Vec3& div(f32 scalar);

        inline Vec3& add(const Vec3& other);
        inline Vec3& sub(const Vec3& other);
        inline Vec3& mul(const Vec3& other);
        inline Vec3& div(const Vec3& other);

        inline Vec3& operator+=(f32 scalar);
        inline Vec3& operator-=(f32 scalar);
        inline Vec3& operator*=(f32 scalar);
        inline Vec3& operator/=(f32 scalar);

        inline Vec3& operator+=(const Vec3& other);
        inline Vec3& operator-=(const Vec3& other);
        inline Vec3& operator*=(const Vec3& other);
        inline Vec3& operator/=(const Vec3& other);

        inline f32& operator[](ui64 index);
        inline const f32& operator[](ui64 index) const;

        inline bool operator==(const Vec3& other) const;
        inline bool operator!=(const Vec3& other) const;
        inline bool compare_to(const Vec3& other, f32 tolerance = 0.001f) const;

        inline Vec2 to_vec2();

        inline f32 length() const;
        inline Vec3& normalize();
        inline Vec3& absolute();

        inline f32 dot(const Vec3& other) const;
        inline Vec3 cross(const Vec3& other) const;
    };

    constexpr inline Vec3::Vec3() : x(0.0f), y(0.0f), z(0.0f) { }

    inline Vec3::Vec3(f32 scalar) : x(scalar), y(scalar), z(scalar) { }

    inline Vec3::Vec3(f32 x, f32 y, f32 z) : x(x), y(y), z(z) { }

    inline Vec3& Vec3::add(f32 scalar)
    {
        x += scalar;
        y += scalar;
        z += scalar;
        return *this;
    }

    inline Vec3& Vec3::sub(f32 scalar)
    {
        x -= scalar;
        y -= scalar;
        z -= scalar;
        return *this;
    }

    inline Vec3& Vec3::mul(f32 scalar)
    {
        x *= scalar;
        y *= scalar;
        z *= scalar;
        return *this;
    }

    inline Vec3& Vec3::div(f32 scalar)
    {
        x /= scalar;
        y /= scalar;
        z /= scalar;
        return *this;
    }

    inline Vec3& Vec3::add(const Vec3& other)
    {
        x += other.x;
        y += other.y;
        z += other.z;
        return *this;
    }

    inline Vec3& Vec3::sub(const Vec3& other)
    {
        x -= other.x;
        y -= other.y;
        z -= other.z;
        return *this;
    }

    inline Vec3& Vec3::mul(const Vec3& other)
    {
        x *= other.x;
        y *= other.y;
        z *= other.z;
        return *this;
    }

    inline Vec3& Vec3::div(const Vec3& other)
    {
        x /= other.x;
        y /= other.y;
        z /= other.z;
        return *this;
    }

    inline Vec3& Vec3::operator+=(f32 scalar)
    {
        return add(scalar);
    }

    inline Vec3& Vec3::operator-=(f32 scalar)
    {
        return sub(scalar);
    }

    inline Vec3& Vec3::operator*=(f32 scalar)
    {
        return mul(scalar);
    }

    inline Vec3& Vec3::operator/=(f32 scalar)
    {
        return div(scalar);
    }

    inline Vec3& Vec3::operator+=(const Vec3& other)
    {
        return add(other);
    }

    inline Vec3& Vec3::operator-=(const Vec3& other)
    {
        return sub(other);
    }

    inline Vec3& Vec3::operator*=(const Vec3& other)
    {
        return mul(other);
    }

    inline Vec3& Vec3::operator/=(const Vec3& other)
    {
        return div(other);
    }

    inline f32& Vec3::operator[](ui64 index)
    {
        hit_assert(index <= 2, "Invalid Vec3 index!");
        return elements[index];
    }

    inline const f32& Vec3::operator[](ui64 index) const
    {
        hit_assert(index <= 2, "Invalid Vec3 index!");
        return elements[index];
    }

    inline bool Vec3::operator==(const Vec3& other) const
    {
        return compare_to(other);
    }

    inline bool Vec3::operator!=(const Vec3& other) const
    {
        return !compare_to(other);
    }

    inline bool Vec3::compare_to(const Vec3& other, f32 tolerance) const
    {
        return (std::abs(x - other.x) <= tolerance) && 
               (std::abs(y - other.y) <= tolerance) &&
               (std::abs(z - other.z) <= tolerance);
    }

    inline Vec2 Vec3::to_vec2()
    {
        return { x, y };
    }

    inline f32 Vec3::length() const
    {
        return (f32)hsqrt(x * x + y * y + z * z);
    }

    inline Vec3& Vec3::normalize()
    {
        const f32 len = length();
        x /= len;
        y /= len;
        z /= len;
        return *this;
    }

    inline Vec3& Vec3::absolute()
    {
        x = std::abs(x);
        y = std::abs(y);
        z = std::abs(z);
        return *this;
    }

    inline f32 Vec3::dot(const Vec3& other) const
    {
        return x * other.x + y * other.y + z * other.z;
    }

    inline Vec3 Vec3::cross(const Vec3& other) const
    {
        return { 
            y * other.z - z * other.y,
            z * other.x - x * other.z,
            x * other.y - y * other.x
        };
    }

    inline Vec3 operator+(const Vec3& v)
    {
        return { v.x, v.y, v.z };
    }

    inline Vec3 operator-(const Vec3& v)
    {
        return { -v.x, -v.y, -v.z };
    }

    inline Vec3 operator+(const Vec3& v1, const Vec3& v2)
    {
        return { v1.x + v2.x, v1.y + v2.y, v1.z + v2.z };
    }

    inline Vec3 operator-(const Vec3& v1, const Vec3& v2)
    {
        return { v1.x - v2.x, v1.y - v2.y, v1.z - v2.z };
    }

    inline Vec3 operator*(const Vec3& v1, const Vec3& v2)
    {
        return { v1.x * v2.x, v1.y * v2.y, v1.z * v2.z };
    }

    inline Vec3 operator/(const Vec3& v1, const Vec3& v2)
    {
        return { v1.x / v2.x, v1.y / v2.y, v1.z / v2.z };
    }

    inline Vec3 operator*(const Vec3& v, f32 scalar)
    {
        return { v.x * scalar, v.y * scalar, v.z * scalar };
    }

    inline Vec3 operator/(const Vec3& v, f32 scalar)
    {
        return { v.x / scalar, v.y / scalar, v.z / scalar };
    }
}