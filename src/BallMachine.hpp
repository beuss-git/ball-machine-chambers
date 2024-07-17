#ifndef BALL_MACHINE_HPP
#define BALL_MACHINE_HPP

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#ifdef __cplusplus
extern "C" {
#endif
#include <libphysics/physics.h>
#ifdef __cplusplus
}
#endif
#include <array>
#include <span>

class Canvas {
public:
    Canvas() = default;
    Canvas(int32_t* canvas_memory, size_t max_canvas_size)
        : m_canvas(std::span { canvas_memory, max_canvas_size })

    {
    }

    [[nodiscard]] void* canvas_memory() const
    {
        return m_canvas.data();
    }

    void clear()
    {
        for (int32_t& pixel : m_canvas) {
            pixel = 0xFFFFFFFF; // NOLINT(cppcoreguidelines-narrowing-conversions)
        }
    }

    void set_pixel(size_t x, size_t y, int32_t color)
    {
        m_canvas[y * x + x] = color;
    }

private:
    std::span<int32_t> m_canvas;
};

class BallMachine {
public:
    BallMachine() = default;
    void init(ball* ball_memory, size_t max_balls, int32_t* canvas_memory, size_t max_canvas_size)
    {
        m_balls = std::span { ball_memory, max_balls };
        m_canvas = Canvas { canvas_memory, max_canvas_size };

        m_surfaces = {

            // Single surface in the middle of the screen
            // screen space is 0,0 in the top left corner
            // 1,1 is the bottom right corner
            surface {
                { 0.2F, 0.5F }, // a
                { 0.8F, 0.5F }, // b
            }
        };
    }

    [[nodiscard]] void* balls_memory() const
    {
        return m_balls.data();
    }

    [[nodiscard]] void* canvas_memory() const
    {
        return m_canvas.canvas_memory();
    }

    void step(size_t num_balls, float delta)
    {
        for (size_t i = 0; i < num_balls; ++i) {
            ball* ball = &m_balls[i];
            apply_gravity(ball, delta);
        }

        for (size_t i = 0; i < num_balls; ++i) {
            ball* ball = &m_balls[i];
            // ball->pos.x += ball->velocity.x * delta;
            // ball->pos.y += ball->velocity.y * delta;
            surface_push_if_colliding(m_surfaces.data(), ball, &ball->velocity, delta, 0.1F);
        }
    }

    void render(size_t canvas_width, size_t canvas_height)
    {
        if (m_last_canvas_width == canvas_width && m_last_canvas_height == canvas_height) {
            return;
        }

        m_canvas.clear();

        m_last_canvas_width = canvas_width;
        m_last_canvas_height = canvas_height;
    }

private:
    std::span<ball> m_balls;
    Canvas m_canvas;
    std::array<surface, 1> m_surfaces;

    size_t m_last_canvas_width = 0;
    size_t m_last_canvas_height = 0;
};

#endif // BALL_MACHINE_HPP
