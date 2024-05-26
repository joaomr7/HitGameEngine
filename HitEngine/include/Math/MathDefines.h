#pragma once

#include "Core/Types.h"

#include <numbers>
#include <cmath>

namespace hit
{
    inline constexpr f64 PI_64 = std::numbers::pi_v<f64>;
    inline constexpr f32 PI_32 = std::numbers::pi_v<f32>;

    inline constexpr f64 PI_rad_64 = PI_64 / 180.0;
    inline constexpr f32 PI_rad_32 = PI_32 / 180.0f;

    inline constexpr auto to_rad(f32 value) { return value * PI_rad_32; }
    inline constexpr auto to_rad(f64 value) { return value * PI_rad_32; }

    inline auto hcos(f32 value) { return std::cos(value); }
    inline auto hcos(f64 value) { return std::cos(value); }

    inline auto hsin(f32 value) { return std::sin(value); }
    inline auto hsin(f64 value) { return std::sin(value); }

    inline auto htan(f32 value) { return std::tan(value); }
    inline auto htan(f64 value) { return std::tan(value); }

    inline auto hsqrt(f32 value) { return std::sqrt(value); }
    inline auto hsqrt(f64 value) { return std::sqrt(value); }
}