#include "portal.hpp"
#include <algorithm>

surface Portal::calculate_surface() const
{
    auto const direction = vec2 { std::cos(m_rotation), std::sin(m_rotation) }; // major axis direction
    pos2 center = m_pos;

    pos2 a = {
        center.x - m_rad_x * direction.x,
        center.y - m_rad_x * direction.y
    };
    pos2 b = {
        center.x + m_rad_x * direction.x,
        center.y + m_rad_x * direction.y
    };
    return surface { a, b };
}

vec2 Portal::normal() const
{
    float normal_rotation = m_rotation + deg2rad(90.F);
    return {
        cos(normal_rotation),
        sin(normal_rotation)
    };
}

void Portal::update_position(float delta)
{
    if (m_is_reversing) {
        m_time_accumulator -= delta; // Decrease time if reversing
    } else {
        m_time_accumulator += delta; // Increase time otherwise
    }

    // Clamp time accumulator to be within the 0 to movement duration range
    m_time_accumulator = std::max(0.0F, std::min(m_time_accumulator, m_movement_duration));

    // Calculate interpolation factor (between 0.0 and 1.0)
    float t = m_time_accumulator / m_movement_duration;

    // Apply cubic easing function
    float eased_t = ease_in_out_sine(t);

    // Linearly interpolate the position based on the eased factor eased_t
    m_pos.x = (1 - eased_t) * m_start_pos.x + eased_t * m_end_pos.x;
    m_pos.y = (1 - eased_t) * m_start_pos.y + eased_t * m_end_pos.y;

    // If time accumulates past the movement duration, reverse direction
    if (m_time_accumulator == 0.0F || m_time_accumulator == m_movement_duration) {
        m_is_reversing = !m_is_reversing; // Toggle reversing state
    }
}
