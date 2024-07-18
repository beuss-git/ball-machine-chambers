#ifndef MATH_HPP
#define MATH_HPP
#include <cmath>
#ifdef __cplusplus
extern "C" {
#endif
#include <libphysics/physics.h>
#ifdef __cplusplus
}
#endif
#include <numbers>

inline vec2 vec2_reflect(vec2 v, vec2 n)
{
    float dot_product = v.x * n.x + v.y * n.y;
    return {
        v.x - 2 * dot_product * n.x,
        v.y - 2 * dot_product * n.y
    };
}

inline vec2 vec2_rotate(vec2 const& vec, float angle)
{
    float cos_a = std::cos(angle);
    float sin_a = std::sin(angle);
    return {
        vec.x * cos_a - vec.y * sin_a,
        vec.x * sin_a + vec.y * cos_a
    };
}

inline float deg2rad(float deg)
{
    return deg * std::numbers::pi_v<float> / 180.F;
}

struct bounds {
    float left;
    float right;
    float top;
    float bottom;
};

struct Color {
    float r { 0 };
    float g { 0 };
    float b { 0 };
};

#endif // MATH_HPP
