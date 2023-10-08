#pragma once

#include "Core/Assert.h"
#include "Core/Types.h"
#include "Vec4.h"
#include "Vec3.h"

namespace hit
{
    union Mat4
    {
        f32 data[16];
        struct 
        {
            Vec4 columns[4];
        };

        constexpr Mat4();
        constexpr Mat4(const Mat4&) = default;
        constexpr Mat4(Mat4&&) noexcept = default;

        constexpr Mat4& operator=(const Mat4&) = default;
        constexpr Mat4& operator=(Mat4&&) = default;

        Mat4& add(const Mat4& other);
        Mat4& sub(const Mat4& other);

        Mat4& mul(const Mat4& other);
        Mat4& mul(f32 scalar);

        Mat4& div(f32 scalar);

        Mat4& operator+=(const Mat4& other);
        Mat4& operator-=(const Mat4& other);
        Mat4& operator*=(const Mat4& other);

        Mat4& operator*=(f32 scalar);
        Mat4& operator/=(f32 scalar);

        inline Vec4& operator[](ui64 index);
        inline const Vec4& operator[](ui64 index) const;

        Mat4 transposed() const;
        Mat4 inverse() const;
    };

    inline Vec4& Mat4::operator[](ui64 index)
    {
        hit_assert(index <= 3, "Invalid Mat4 column index!");
        return columns[index];
    }

    inline const Vec4& Mat4::operator[](ui64 index) const
    {
        hit_assert(index <= 3, "Invalid Mat4 column index!");
        return columns[index];
    }

    // Mat4 helper functions
    Mat4 mat4_add(const Mat4& m1, const Mat4& m2);
    Mat4 mat4_sub(const Mat4& m1, const Mat4& m2);
    Mat4 mat4_mul(const Mat4& m1, const Mat4& m2);
    Mat4 mat4_mul(const Mat4& m1, f32 scalar);
    Mat4 mat4_div(const Mat4& m1, f32 scalar);

    Vec4 mat4_mult_vec4(const Mat4& m, const Vec4& v);
    Vec4 mat4_mult_vec4(const Vec4& v, const Mat4& m);

    Vec3 mat4_mult_vec3(const Mat4& m, const Vec3& v);
    Vec3 mat4_mult_vec3(const Vec3& v, const Mat4& m);

    Mat4 mat4_orthographic(f32 left, f32 right, f32 bottom, f32 top, f32 near, f32 far);
    Mat4 mat4_perspective(f32 fov, f32 aspect_ratio, f32 near, f32 far);

    Mat4 mat4_identity();

    Mat4 mat4_scale(f32 scalar);
    Mat4 mat4_scale(const Vec3& v);
    Mat4 mat4_scale(f32 x, f32 y, f32 z);

    Mat4 mat4_translation(const Vec3& v);
    Mat4 mat4_translation(f32 x, f32 y, f32 z);

    Mat4 mat4_euler_x(f32 angle);
    Mat4 mat4_euler_y(f32 angle);
    Mat4 mat4_euler_z(f32 angle);

    Mat4 mat4_euler_rotation(const Vec3& v);    
    Mat4 mat4_euler_rotation(f32 x, f32 y, f32 z);

    Vec3 mat4_get_forward(const Mat4& mat);
    Vec3 mat4_get_backward(const Mat4& mat);
    Vec3 mat4_get_up(const Mat4& mat);
    Vec3 mat4_get_down(const Mat4& mat);
    Vec3 mat4_get_right(const Mat4& mat);
    Vec3 mat4_get_left(const Mat4& mat);

    // Mat4 operators
    inline Mat4 operator+(const Mat4& m)
    {
        return m;
    }

    inline Mat4 operator-(const Mat4& m)
    {
        return mat4_mul(m, -1.f);
    }

    inline Mat4 operator+(const Mat4& m1, const Mat4& m2)
    {
        return mat4_add(m1, m2);
    }

    inline Mat4 operator-(const Mat4& m1, const Mat4& m2)
    {
        return mat4_sub(m1, m2);
    }

    inline Mat4 operator*(const Mat4& m1, const Mat4& m2)
    {
        return mat4_mul(m1, m2);
    }

    inline Mat4 operator*(const Mat4& m1, f32 scalar)
    {
        return mat4_mul(m1, scalar);
    }

    inline Mat4 operator/(const Mat4& m1, f32 scalar)
    {
        return mat4_div(m1, scalar);
    }

    inline Vec4 operator*(const Mat4& m, const Vec4& v)
    {
        return mat4_mult_vec4(m, v);
    }

    inline Vec4 operator*(const Vec4& v, const Mat4& m)
    {
        return mat4_mult_vec4(v, m);
    }

    inline Vec3 operator*(const Mat4& m, const Vec3& v)
    {
        return mat4_mult_vec3(m, v);
    }

    inline Vec3 operator*(const Vec3& v, const Mat4& m)
    {
        return mat4_mult_vec3(v, m);
    }
}