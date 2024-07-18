#ifndef BALL_MACHINE_HPP
#define BALL_MACHINE_HPP

#include <cmath>
#include <limits>
#ifdef __cplusplus
extern "C" {
#endif
#include <libphysics/physics.h>
#ifdef __cplusplus
}
#endif

#include "printer.hpp"
#include <canvas_ity/canvas_ity.hpp>
#include <vector>

using pixel = int32_t;
class Canvas {
public:
    Canvas() = default;
    explicit Canvas(size_t max_canvas_size)
        : m_canvas(max_canvas_size)

    {
    }

    [[nodiscard]] int32_t* canvas_memory()
    {
        return m_canvas.data();
    }

    void clear()
    {
        for (pixel& pixel : m_canvas) {
            pixel = 0xFF00FFFF; // NOLINT(cppcoreguidelines-narrowing-conversions)
        }
    }

    void set_pixel(size_t x, size_t y, pixel color)
    {
        m_canvas[y * x + x] = color;
    }

private:
    std::vector<int32_t> m_canvas;
};

struct Color {
    float r { 0 };
    float g { 0 };
    float b { 0 };
};

class Portal {
public:
    Portal() = default;
    Portal(pos2 start_pos, pos2 end_pos, Color color, float rad_x, float rad_y, float rotation = 0.0F)
        : m_start_pos(start_pos)
        , m_end_pos(end_pos)
        , m_pos(start_pos)
        , m_color(color)
        , m_rad_x(rad_x)
        , m_rad_y(rad_y)
        , m_rotation(rotation)
    {
    }
    [[nodiscard]] pos2 pos() const { return m_pos; }
    [[nodiscard]] vec2 pos_vec() const { return { m_pos.x, m_pos.y }; }
    void set_pos(pos2 pos) { m_pos = pos; }
    [[nodiscard]] Color color() const { return m_color; }
    [[nodiscard]] float rad_x() const { return m_rad_x; }
    [[nodiscard]] float rad_y() const { return m_rad_y; }
    [[nodiscard]] float rotation() const { return m_rotation; }
    void set_rotation(float rotation) { m_rotation = rotation; }
    [[nodiscard]] surface calculate_surface() const
    {
        float theta = m_rotation;
        auto const direction = vec2 { cos(theta), sin(theta) }; // major axis direction
        pos2 center = m_pos;

        pos2 a = {
            center.x - m_rad_x * direction.x,
            center.y - m_rad_y * direction.y
        };
        pos2 b = {
            center.x + m_rad_x * direction.x,
            center.y + m_rad_y * direction.y
        };
        return surface { a, b };
    }

    float cubic_ease_in_out(float t) const
    {
        if (t < 0.5F) {
            return 4 * t * t * t;
        } else {
            float f = ((2 * t) - 2);
            return 0.5F * f * f * f + 1;
        }
    }
    float linear_ease(float t) const
    {
        return t;
    }
    void update_position(float delta)
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
        float eased_t = linear_ease(t);

        // Linearly interpolate the position based on the eased factor eased_t
        m_pos.x = (1 - eased_t) * m_start_pos.x + eased_t * m_end_pos.x;
        m_pos.y = (1 - eased_t) * m_start_pos.y + eased_t * m_end_pos.y;

        // If time accumulates past the movement duration, reverse direction
        if (m_time_accumulator == 0.0F || m_time_accumulator == m_movement_duration) {
            m_is_reversing = !m_is_reversing; // Toggle reversing state
        }
    }

private:
    pos2 m_start_pos { 0, 0 }; // Starting position
    pos2 m_end_pos { 0, 0 };   // End position
    float m_time_accumulator {};
    bool m_is_reversing { false };     // Flag to reverse direction
    float m_movement_duration { 4.0 }; // Duration from start to end
    pos2 m_pos { 0, 0 };               // End position
    Color m_color {};
    float m_rad_x {};
    float m_rad_y {};
    float m_rotation {};
};

struct Image {
    int width;
    int height;
    std::vector<int32_t> data;
};

// struct bounds {
//     vec2 min { std::numeric_limits<float>::max(), std::numeric_limits<float>::max() };
//     vec2 max { std::numeric_limits<float>::min(), std::numeric_limits<float>::min() };
// };
struct bounds {
    float left;
    float right;
    float top;
    float bottom;
};

class BallMachine {
public:
    BallMachine(size_t max_balls, size_t max_canvas_size)
        : m_ctx(300, 209)
    {
        init(max_balls, max_canvas_size);
    }
    void init(size_t max_balls, size_t max_canvas_size)
    {
        m_balls = std::vector<ball>(max_balls);
        m_canvas = Canvas { max_canvas_size };

        m_surfaces = {

            // Single surface in the middle of the screen
            // screen space is 0,0 in the top left corner
            // 1,1 is the bottom right corner
            //     surface {
            //         { 0.2F, 0.5F },  // a
            //         { 0.8F, 0.45F }, // b
            //     },
            //     surface {
            //         { 0.85F, 0.2F }, // a
            //         { 1.0F, 0.8F },  // b
            //     },
            //     surface {
            //         { 0.0F, 0.8F },  // a
            //         { 0.15F, 0.2F }, // b
            //     }
        };
        m_blue_portal = Portal { { 0.905F, 0.5F }, { 0.905F, 0.0F }, { 0.0F, 0.7F, 1.0F }, 0.15F, 0.05F, 1.5708F };
        m_orange_portal = Portal { { 0.2F, 0.095F }, { 1.F - 0.2F, 0.095F }, { 1.0F, 0.5F, 0.0F }, 0.15F, 0.05F, 0.0F };

        m_blue_portal_texture = render_portal_to_texture(m_ctx, 300, 209, m_blue_portal);
        m_orange_portal_texture = render_portal_to_texture(m_ctx, 300, 209, m_orange_portal);
    }

    [[nodiscard]] void* balls_memory() { return m_balls.data(); }
    [[nodiscard]] void* canvas_memory() { return m_canvas.canvas_memory(); }
    void step(size_t num_balls, float delta);

    void render(size_t canvas_width, size_t canvas_height);

    [[nodiscard]] float pix2pos_x(float x_norm) const
    {
        return x_norm * static_cast<float>(m_last_canvas_width);
    }
    [[nodiscard]] float pix2pos_y(float y_norm) const
    {
        return static_cast<float>(m_last_canvas_height) - y_norm * static_cast<float>(m_last_canvas_width);
    }
    [[nodiscard]] pos2 pix2pos(pos2 pos_norm) const
    {
        return {
            pix2pos_x(pos_norm.x),
            pix2pos_y(pos_norm.y)
        };
    }

    void draw_portal(canvas_ity::canvas& context, float cx, float cy, float rx, float ry, Color const& portal_color, float rotation);
    Image render_portal_to_texture(canvas_ity::canvas& context, size_t canvas_width, size_t canvas_height, Portal const& portal);

    void put_image(canvas_ity::canvas& context, size_t x, size_t y, Image const& image);

private:
    canvas_ity::canvas m_ctx;
    std::vector<ball> m_balls;
    Canvas m_canvas;
    std::vector<surface> m_surfaces;
    Portal m_blue_portal;
    Portal m_orange_portal;
    Image m_blue_portal_texture;
    Image m_orange_portal_texture;

    size_t m_last_canvas_width = 0;
    size_t m_last_canvas_height = 0;
};

#endif // BALL_MACHINE_HPP
