#ifndef PORTAL_HPP
#define PORTAL_HPP
#ifdef __cplusplus
extern "C" {
#endif
#include <libphysics/physics.h>
#ifdef __cplusplus
}
#endif
#include "utils/math.hpp"

class Portal {
public:
    Portal() = default;
    Portal(pos2 start_pos, pos2 end_pos, Color color, float rad_x, float rad_y, float rotation = 0.0F, float movement_duration = 1.F)
        : m_start_pos(start_pos)
        , m_end_pos(end_pos)
        , m_pos(start_pos)
        , m_color(color)
        , m_rad_x(rad_x)
        , m_rad_y(rad_y)
        , m_rotation(rotation)
        , m_movement_duration(movement_duration)
    {
    }
    [[nodiscard]] pos2 pos() const { return m_pos; }
    [[nodiscard]] vec2 pos_vec() const { return { m_pos.x, m_pos.y }; }
    void set_pos(pos2 pos) { m_pos = pos; }
    [[nodiscard]] Color color() const { return m_color; }
    [[nodiscard]] float rad_x() const { return m_rad_x; }
    [[nodiscard]] float rad_y() const { return m_rad_y; }
    [[nodiscard]] float rotation() const { return m_rotation; }
    [[nodiscard]] surface calculate_surface() const;
    [[nodiscard]] vec2 normal() const;

    [[maybe_unused]] float cubic_ease_in_out(float t) const
    {
        if (t < 0.5F) {
            return 4 * t * t * t;
        } else {
            float f = ((2 * t) - 2);
            return 0.5F * f * f * f + 1;
        }
    }
    static float ease_in_out_sine(float t)
    {
        return -(std::cos(std::numbers::pi_v<float> * t) - 1.F) / 2.F;
    }
    [[nodiscard]] static float linear_ease(float t)
    {
        return t;
    }
    void update_position(float delta);

private:
    pos2 m_start_pos { 0, 0 }; // Starting position
    pos2 m_end_pos { 0, 0 };   // End position
    float m_time_accumulator {};
    bool m_is_reversing { false };     // Flag to reverse direction
    float m_movement_duration { 0.5 }; // Duration from start to end
    pos2 m_pos { 0, 0 };               // End position
    Color m_color {};
    float m_rad_x {};
    float m_rad_y {};
    float m_rotation {};
};
#endif // PORTAL_HPP
