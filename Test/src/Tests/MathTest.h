#pragma once

#include "../TestFramework.h"
#include "Math/Math.h"

namespace hit
{
    test_val math_vec2_test()
    {
        Vec2 vec;
        test_check(vec == Vec2(0.0f, 0.0f));

        vec += Vec2(1.0f, 0.0f);
        test_check(vec == Vec2(1.0f, 0.0f));

        vec -= Vec2(1.0f, 0.0f);
        test_check(vec == Vec2(0.0f, 0.0f));

        vec += Vec2(1.0f, 1.0f);
        vec *= 2.f;
        test_check(vec == Vec2(2.0f, 2.0f));

        vec /= 2.f;
        test_check(vec == Vec2(1.0f, 1.0f));

        vec = vec + Vec2(5.0, 4.0) * 3;
        test_check(vec == Vec2(16.0f, 13.0f));

        test_success();
    }

    test_val math_vec3_test()
    {
        Vec3 vec;
        test_check(vec == Vec3(0.0f, 0.0f, 0.0f));

        vec += Vec3(1.0f, 0.0f, 0.0f);
        test_check(vec == Vec3(1.0f, 0.0f, 0.0f));

        vec -= Vec3(1.0f, 0.0f, 0.0f);
        test_check(vec == Vec3(0.0f, 0.0f, 0.0f));

        vec += Vec3(1.0f, 1.0f, 1.0f);
        vec *= 2.f;
        test_check(vec == Vec3(2.0f, 2.0f, 2.0f));

        vec /= 2.f;
        test_check(vec == Vec3(1.0f, 1.0f, 1.0f));

        vec = vec + Vec3(5.0, 4.0, 3.0) * 3;
        test_check(vec == Vec3(16.0f, 13.0f, 10.0f));

        test_success();
    }

    test_val math_vec4_test()
    {
        Vec4 vec;
        test_check(vec == Vec4(0.0f, 0.0f, 0.0f, 0.0f));

        vec += Vec4(1.0f, 0.0f, 0.0f, 0.0f);
        test_check(vec == Vec4(1.0f, 0.0f, 0.0f, 0.0f));

        vec -= Vec4(1.0f, 0.0f, 0.0f, 0.0f);
        test_check(vec == Vec4(0.0f, 0.0f, 0.0f, 0.0f));

        vec += Vec4(1.0f, 1.0f, 1.0f, -1.0f);
        vec *= 2.f;
        test_check(vec == Vec4(2.0f, 2.0f, 2.0f, -2.0f));

        vec /= 2.f;
        test_check(vec == Vec4(1.0f, 1.0f, 1.0f, -1.0f));

        vec = vec + Vec4(5.0, 4.0, 3.0, 2.0f) * 3;
        test_check(vec == Vec4(16.0f, 13.0f, 10.0f, 5.0f));

        test_success();
    }

    void add_math_tests(TestSystem& test_system)
    {
        test_system.add_test(get_test(math_vec2_test));
        test_system.add_test(get_test(math_vec3_test));
        test_system.add_test(get_test(math_vec4_test));
    }
}