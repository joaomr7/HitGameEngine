#include "Math/Mat4.h"

#include "Core/Memory.h"
#include "Math/MathDefines.h"

namespace hit
{
    constexpr Mat4::Mat4() : data{} { }

    Mat4& Mat4::add(const Mat4& other)
    {
        for (ui32 i = 0; i < 4; i++)
        {
            for (ui32 j = 0; j < 4; j++)
            {
                columns[j][i] += other[j][i];
            }
        }

        return *this;
    }

    Mat4& Mat4::sub(const Mat4& other)
    {
        for (ui32 i = 0; i < 4; i++)
        {
            for (ui32 j = 0; j < 4; j++)
            {
                columns[j][i] -= other[j][i];
            }
        }

        return *this;
    }

    Mat4& Mat4::mul(const Mat4& other)
    {
        Mat4 new_mat = mat4_mul(*this, other);

        Memory::copy_memory((ui8*)data, (ui8*)new_mat.data, sizeof(f32) * 16);

        return *this;
    }

    Mat4& Mat4::mul(f32 scalar)
    {
        for (ui32 i = 0; i < 16; i++) data[i] *= scalar;
        return *this;
    }

    Mat4& Mat4::div(f32 scalar)
    {
        for (ui32 i = 0; i < 16; i++) data[i] /= scalar;
        return *this;
    }

    Mat4& Mat4::operator+=(const Mat4& other)
    {
        return add(other);
    }

    Mat4& Mat4::operator-=(const Mat4& other)
    {
        return sub(other);
    }

    Mat4& Mat4::operator*=(const Mat4& other)
    {
        return mul(other);
    }

    Mat4& Mat4::operator*=(f32 scalar)
    {
        return mul(scalar);
    }

    Mat4& Mat4::operator/=(f32 scalar)
    {
        return div(scalar);
    }

    Mat4 Mat4::transposed() const
    {
        Mat4 t_mat;

        for (ui32 i = 0; i < 4; i++)
        {
            for (ui32 j = 0; j < 4; j++)
            {
                t_mat[j][i] = columns[i][j];
            }
        }

        return t_mat;
    }

    Mat4 Mat4::inverse() const
    {
        Mat4 inv_mat;
        f32* inv_data = inv_mat.data;

        inv_data[0] = 
             data[5]  * data[10] * data[15] -
             data[5]  * data[11] * data[14] -
             data[9]  * data[6]  * data[15] +
             data[9]  * data[7]  * data[14] +
             data[13] * data[6]  * data[11] -
             data[13] * data[7]  * data[10];

        inv_data[4] =
            -data[4]  * data[10] * data[15] + 
             data[4]  * data[11] * data[14] + 
             data[8]  * data[6]  * data[15] - 
             data[8]  * data[7]  * data[14] - 
             data[12] * data[6]  * data[11] + 
             data[12] * data[7]  * data[10];

        inv_data[8] = 
             data[4]  * data[9]  * data[15] - 
             data[4]  * data[11] * data[13] - 
             data[8]  * data[5]  * data[15] + 
             data[8]  * data[7]  * data[13] + 
             data[12] * data[5]  * data[11] - 
             data[12] * data[7]  * data[9];

        inv_data[12] = 
            -data[4]  * data[9]  * data[14] + 
             data[4]  * data[10] * data[13] +
             data[8]  * data[5]  * data[14] - 
             data[8]  * data[6]  * data[13] - 
             data[12] * data[5]  * data[10] + 
             data[12] * data[6]  * data[9];

        inv_data[1] = 
            -data[1]  * data[10] * data[15] + 
             data[1]  * data[11] * data[14] + 
             data[9]  * data[2]  * data[15] - 
             data[9]  * data[3]  * data[14] - 
             data[13] * data[2]  * data[11] + 
             data[13] * data[3]  * data[10];

        inv_data[5] = 
             data[0]  * data[10] * data[15] - 
             data[0]  * data[11] * data[14] - 
             data[8]  * data[2]  * data[15] + 
             data[8]  * data[3]  * data[14] + 
             data[12] * data[2]  * data[11] - 
             data[12] * data[3]  * data[10];

        inv_data[9] = 
            -data[0]  * data[9]  * data[15] + 
             data[0]  * data[11] * data[13] + 
             data[8]  * data[1]  * data[15] - 
             data[8]  * data[3]  * data[13] - 
             data[12] * data[1]  * data[11] + 
             data[12] * data[3]  * data[9];

        inv_data[13] = 
             data[0]  * data[9]  * data[14] - 
             data[0]  * data[10] * data[13] - 
             data[8]  * data[1]  * data[14] + 
             data[8]  * data[2]  * data[13] + 
             data[12] * data[1]  * data[10] - 
             data[12] * data[2]  * data[9];

        inv_data[2] = 
             data[1]  * data[6] * data[15] - 
             data[1]  * data[7] * data[14] - 
             data[5]  * data[2] * data[15] + 
             data[5]  * data[3] * data[14] + 
             data[13] * data[2] * data[7]  - 
             data[13] * data[3] * data[6];

        inv_data[6] = 
            -data[0]  * data[6] * data[15] + 
             data[0]  * data[7] * data[14] + 
             data[4]  * data[2] * data[15] - 
             data[4]  * data[3] * data[14] - 
             data[12] * data[2] * data[7]  + 
             data[12] * data[3] * data[6];

        inv_data[10] = 
             data[0]  * data[5] * data[15] - 
             data[0]  * data[7] * data[13] - 
             data[4]  * data[1] * data[15] + 
             data[4]  * data[3] * data[13] + 
             data[12] * data[1] * data[7]  - 
             data[12] * data[3] * data[5];

        inv_data[14] = 
            -data[0]  * data[5] * data[14] + 
             data[0]  * data[6] * data[13] + 
             data[4]  * data[1] * data[14] - 
             data[4]  * data[2] * data[13] - 
             data[12] * data[1] * data[6]  + 
             data[12] * data[2] * data[5];

        inv_data[3] = 
            -data[1] * data[6] * data[11] + 
             data[1] * data[7] * data[10] + 
             data[5] * data[2] * data[11] - 
             data[5] * data[3] * data[10] - 
             data[9] * data[2] * data[7]  + 
             data[9] * data[3] * data[6];

        inv_data[7] = 
             data[0] * data[6] * data[11] - 
             data[0] * data[7] * data[10] - 
             data[4] * data[2] * data[11] + 
             data[4] * data[3] * data[10] + 
             data[8] * data[2] * data[7]  - 
             data[8] * data[3] * data[6];

        inv_data[11] = 
            -data[0] * data[5] * data[11] + 
             data[0] * data[7] * data[9]  + 
             data[4] * data[1] * data[11] - 
             data[4] * data[3] * data[9]  - 
             data[8] * data[1] * data[7]  + 
             data[8] * data[3] * data[5];

        inv_data[15] = 
             data[0] * data[5] * data[10] - 
             data[0] * data[6] * data[9]  - 
             data[4] * data[1] * data[10] + 
             data[4] * data[2] * data[9]  + 
             data[8] * data[1] * data[6]  - 
             data[8] * data[2] * data[5];

        f32 determinant = 
            data[0] * inv_data[0] + 
            data[1] * inv_data[4] + 
            data[2] * inv_data[8] + 
            data[3] * inv_data[12];

        if(determinant == 0.0f) [[unlikely]]
            return mat4_identity();

        determinant = 1.0f / determinant;
        inv_mat *= determinant;

        return inv_mat;
    }

    // mat4 functions
    Mat4 mat4_add(const Mat4& m1, const Mat4& m2)
    {
        Mat4 out_m;

        for (ui32 i = 0; i < 4; i++)
        {
            for (ui32 j = 0; j < 4; j++)
            {
                out_m[j][i] = m1[j][i] + m2[j][i];
            }
        }

        return out_m;
    }

    Mat4 mat4_sub(const Mat4& m1, const Mat4& m2)
    {
        Mat4 out_m;

        for (ui32 i = 0; i < 4; i++)
        {
            for (ui32 j = 0; j < 4; j++)
            {
                out_m[j][i] = m1[j][i] - m2[j][i];
            }
        }

        return out_m;
    }

    Mat4 mat4_mul(const Mat4& m1, const Mat4& m2)
    {
        Mat4 out_m;

        for (ui32 i = 0; i < 4; i++)
        {
            for (ui32 j = 0; j < 4; j++)
            {
                out_m[j][i] = 
                    m1[0][i] * m2[j][0] +
                    m1[1][i] * m2[j][1] +
                    m1[2][i] * m2[j][2] +
                    m1[3][i] * m2[j][3];
            }
        }

        return out_m;
    }

    Mat4 mat4_mul(const Mat4& m1, f32 scalar)
    {
        Mat4 out_mat = m1;
        out_mat *= scalar;
        return out_mat;
    }

    Mat4 mat4_div(const Mat4& m1, f32 scalar)
    {
        Mat4 out_mat = m1;
        out_mat /= scalar;
        return out_mat;
    }

    Vec4 mat4_mult_vec4(const Mat4& m, const Vec4& v)
    {
        return {
            v.x * m.data[0]  + v.y * m.data[1]  + v.z * m.data[2]  + v.w * m.data[3],
            v.x * m.data[4]  + v.y * m.data[5]  + v.z * m.data[6]  + v.w * m.data[7],
            v.x * m.data[8]  + v.y * m.data[9]  + v.z * m.data[10] + v.w * m.data[11],
            v.x * m.data[12] + v.y * m.data[13] + v.z * m.data[14] + v.w * m.data[15]
        };
    }

    Vec4 mat4_mult_vec4(const Vec4& v, const Mat4& m)
    {
        return {
            v.x * m.data[0] + v.y * m.data[4] + v.z * m.data[8]  + v.w * m.data[12],
            v.x * m.data[1] + v.y * m.data[5] + v.z * m.data[9]  + v.w * m.data[13],
            v.x * m.data[2] + v.y * m.data[6] + v.z * m.data[10] + v.w * m.data[14],
            v.x * m.data[3] + v.y * m.data[7] + v.z * m.data[11] + v.w * m.data[15]
        };
    }

    Vec3 mat4_mult_vec3(const Mat4& m, const Vec3& v)
    {
        return {
            v.x * m.data[0] + v.y * m.data[1] + v.z * m.data[2]  + m.data[3],
            v.x * m.data[4] + v.y * m.data[5] + v.z * m.data[6]  + m.data[7],
            v.x * m.data[8] + v.y * m.data[9] + v.z * m.data[10] + m.data[11]
        };
    }

    Vec3 mat4_mult_vec3(const Vec3& v, const Mat4& m)
    {
        return {
            v.x * m.data[0] + v.y * m.data[4] + v.z * m.data[8]  + v.w * m.data[12],
            v.x * m.data[1] + v.y * m.data[5] + v.z * m.data[9]  + v.w * m.data[13],
            v.x * m.data[2] + v.y * m.data[6] + v.z * m.data[10] + v.w * m.data[14]
        };
    }

    Mat4 mat4_orthographic(f32 left, f32 right, f32 bottom, f32 top, f32 near, f32 far)
    {
        Mat4 ortho = mat4_identity();

        const f32 right_plus_left = right + left;
        const f32 right_less_left = right - left;

        const f32 top_plus_bottom = top + bottom;
        const f32 top_less_bottom = top - bottom;

        const f32 far_plus_near = far + near;
        const f32 far_less_near = far - near;

        ortho[0][0] =  2.0f / right_less_left;
        ortho[1][1] =  2.0f / top_less_bottom;
        ortho[2][2] = -2.0f / far_less_near;
        ortho[3][0] = -right_plus_left / right_less_left;
        ortho[3][1] = -top_plus_bottom / top_less_bottom;
        ortho[3][2] = -far_plus_near   / far_less_near;

        return ortho;
    }

    Mat4 mat4_perspective(f32 fov, f32 aspect_ratio, f32 near, f32 far)
    {
        hit_assert(near >= 0.0f, "Perspective near must be positive.");
        hit_assert(far > 0.0f, "Perspective far must be positive.");
        hit_assert(far > near, "Perspective far must be larger than near.");

        fov = to_rad(fov);
        const f32 tan_half_fov = htan(fov / 2.0f);

        Mat4 perspective;

        perspective[0][0] = 1.0f / (aspect_ratio * tan_half_fov);
        perspective[1][1] = 1.0f / tan_half_fov;
        perspective[2][2] = - (far + near) / (far - near);
        perspective[3][2] = - (2.0f * far * near) / (far - near);
        perspective[2][3] = -1.0f;

        return perspective;
    }

    Mat4 mat4_identity()
    {
        Mat4 mat;
        mat[0][0] = 1.0f;
        mat[1][1] = 1.0f;
        mat[2][2] = 1.0f;
        mat[3][3] = 1.0f;
        return mat;
    };

    Mat4 mat4_scale(f32 scalar)
    {
        return mat4_scale(scalar, scalar, scalar);
    }

    Mat4 mat4_scale(const Vec3& v)
    {
        return mat4_scale(v.x, v.y, v.z);
    }

    Mat4 mat4_scale(f32 x, f32 y, f32 z)
    {
        Mat4 mat = mat4_identity();
        mat[0][0] = x;
        mat[1][1] = y;
        mat[2][2] = z;
        return mat;
    }

    Mat4 mat4_translation(const Vec3& v)
    {
        return mat4_translation(v.x, v.y, v.z);
    }

    Mat4 mat4_translation(f32 x, f32 y, f32 z)
    {
        Mat4 mat = mat4_identity();
        mat[3][0] = x;
        mat[3][1] = y;
        mat[3][2] = z;
        return mat;
    };

    Mat4 mat4_euler_x(f32 angle)
    {
        const auto rad_angle = to_rad(angle);
        const auto angle_cos = hcos(rad_angle);
        const auto angle_sin = hsin(rad_angle);

        Mat4 mat = mat4_identity();
        mat[1][1] = angle_cos;
        mat[1][2] = angle_sin;
        mat[2][1] = -angle_sin;
        mat[2][2] = angle_cos;
        return mat;
    }

    Mat4 mat4_euler_y(f32 angle)
    {
        const auto rad_angle = to_rad(angle);
        const auto angle_cos = hcos(rad_angle);
        const auto angle_sin = hsin(rad_angle);

        Mat4 mat = mat4_identity();
        mat[0][0] = angle_cos;
        mat[0][2] = -angle_sin;
        mat[2][0] = angle_sin;
        mat[2][2] = angle_cos;
        return mat;
    }

    Mat4 mat4_euler_z(f32 angle)
    {
        const auto rad_angle = to_rad(angle);
        const auto angle_cos = hcos(rad_angle);
        const auto angle_sin = hsin(rad_angle);

        Mat4 mat = mat4_identity();
        mat[0][0] = angle_cos;
        mat[0][1] = angle_sin;
        mat[1][0] = -angle_sin;
        mat[1][1] = angle_cos;
        return mat;
    }

    Mat4 mat4_euler_rotation(const Vec3& v)
    {
        return mat4_euler_rotation(v.x, v.y, v.z);
    }

    Mat4 mat4_euler_rotation(f32 x, f32 y, f32 z)
    {
        const Mat4 rotation_x = mat4_euler_x(x);
        const Mat4 rotation_y = mat4_euler_y(y);
        const Mat4 rotation_z = mat4_euler_z(z);

        Mat4 out_m = (rotation_x * rotation_y) * rotation_z;

        return out_m;
    }

    Vec3 mat4_get_forward(const Mat4& mat)
    {
        Vec3 forward;

        forward.x = -mat.data[2];
        forward.y = -mat.data[6];
        forward.z = -mat.data[10];

        forward.normalize();

        return forward;
    }

    Vec3 mat4_get_backward(const Mat4& mat)
    {
        return -mat4_get_forward(mat);
    }

    Vec3 mat4_get_up(const Mat4& mat)
    {
        Vec3 up;

        up.x = mat.data[1];
        up.x = mat.data[5];
        up.x = mat.data[9];

        up.normalize();

        return up;
    }

    Vec3 mat4_get_down(const Mat4& mat)
    {
        return -mat4_get_up(mat);
    }

    Vec3 mat4_get_right(const Mat4& mat)
    {
        Vec3 right;

        right.x = mat.data[0];
        right.x = mat.data[4];
        right.x = mat.data[8];

        right.normalize();

        return right;
    }

    Vec3 mat4_get_left(const Mat4& mat)
    {
        return -mat4_get_right(mat);
    }
}