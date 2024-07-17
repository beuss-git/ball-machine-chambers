#ifndef BALL_MACHINE_HPP
#define BALL_MACHINE_HPP

#ifdef __cplusplus
extern "C" {
#endif
#include <libphysics/physics.h>
#ifdef __cplusplus
}
#endif

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
            surface {
                { 0.2F, 0.5F },  // a
                { 0.8F, 0.45F }, // b
            },
            surface {
                { 0.85F, 0.2F }, // a
                { 1.0F, 0.8F },  // b
            },
            surface {
                { 0.0F, 0.8F },  // a
                { 0.15F, 0.2F }, // b
            }
        };
    }

    [[nodiscard]] void* balls_memory()
    {
        return m_balls.data();
    }

    [[nodiscard]] void* canvas_memory()
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
            for (auto const& surface : m_surfaces) {

                if (vec2 res {}; surface_collision_resolution(&surface, &ball->pos, &ball->velocity, &res)) {

                    // surface_push_if_colliding(&surface, ball, &ball->velocity, delta, 0.01F);
                    auto const normal = surface_normal(&surface);
                    auto const zero = vec2 { 0.0F, 0.0F };
                    apply_ball_collision(ball, &res, &normal, &zero, delta, 0.9F);
                }
            }
        }
    }

    static pos2 pix2pos(pos2 pos_norm, size_t canvas_width, size_t canvas_height)
    {
        return {
            pos_norm.x * static_cast<float>(canvas_width),
            static_cast<float>(canvas_height) - pos_norm.y * static_cast<float>(canvas_width)
        };
    }

    void render(size_t canvas_width, size_t canvas_height);

private:
    std::vector<ball> m_balls;
    Canvas m_canvas;
    std::vector<surface> m_surfaces;

    size_t m_last_canvas_width = 0;
    size_t m_last_canvas_height = 0;
};

#endif // BALL_MACHINE_HPP
