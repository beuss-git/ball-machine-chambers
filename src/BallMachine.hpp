#ifndef BALL_MACHINE_HPP
#define BALL_MACHINE_HPP

#include <cmath>
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
    Portal(pos2 pos, Color color, float rad_x, float rad_y, float rotation = 0.0F)
        : m_pos(pos)
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

private:
    pos2 m_pos {};
    Color m_color {};
    float m_rad_x {};
    float m_rad_y {};
    float m_rotation {};
};

class BallMachine {
public:
    BallMachine() = default;
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

        m_portals = {
            // Blue portal
            Portal { { 0.7, 0.3 }, { 0.0F, 0.7F, 1.0F }, 0.15F, 0.05F, 0.0F },
            // Orange portal
            Portal { { 0.3, 0.2 }, { 1.0F, 0.5F, 0.0F }, 0.15F, 0.05F, 0.0F }
        };
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

private:
    std::vector<ball> m_balls;
    Canvas m_canvas;
    std::vector<surface> m_surfaces;
    std::array<Portal, 2> m_portals {};

    size_t m_last_canvas_width = 0;
    size_t m_last_canvas_height = 0;
};

#endif // BALL_MACHINE_HPP
