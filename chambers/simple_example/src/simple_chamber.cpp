#include <libchamber/chamber.hpp>
#include <libchamber/exports.h>
#ifdef __cplusplus
extern "C" {
#endif
#include <libphysics/physics.h>
#ifdef __cplusplus
}
#endif
#include <ranges>

class Simple : public chamber::Chamber {
public:
    ~Simple() override = default;
    Simple(size_t max_balls, size_t max_canvas_size)
        : Chamber(max_balls, max_canvas_size)
    {
        m_surface = {
            .a = { 0.2F, 0.5F },
            .b = { 0.8F, 0.5F },
        };
    }

    void step(size_t num_balls, float delta) override
    {
        for (auto& ball : std::ranges::views::take(m_balls, num_balls)) {
            apply_gravity(&ball, delta);
        }

        for (auto& ball : std::ranges::views::take(m_balls, num_balls)) {
            vec2 res {};
            if (surface_collision_resolution(&m_surface, &ball.pos, &ball.velocity, &res)) {
                auto surf_normal = surface_normal(&m_surface);
                vec2 zero = { 0, 0 };
                apply_ball_collision(&ball, &res, &surf_normal, &zero, delta, 0.90F);
            }
        }
    }

    void render(size_t canvas_width, size_t canvas_height) override
    {
        if (m_prev_canvas_width == canvas_width && m_prev_canvas_height == canvas_height) {
            return;
        }
        m_prev_canvas_width = canvas_width;
        m_prev_canvas_height = canvas_height;

        auto const draw_horizontal_line = [&](int x1, int x2, int y) {
            for (int i = x1; i < x2; i++) {
                m_canvas[i + y * canvas_width] = 0xFF000000;
            }
        };
        auto const pos2pix_x = [&](float x_norm) {
            return x_norm * static_cast<float>(canvas_width);
        };
        auto const pos2pix_y = [&](float y_norm) {
            return static_cast<float>(canvas_height) - y_norm * static_cast<float>(canvas_width);
        };

        draw_horizontal_line(pos2pix_x(m_surface.a.x), pos2pix_x(m_surface.b.x), pos2pix_y(m_surface.a.y));
    }

private:
    surface m_surface;
    size_t m_prev_canvas_width;
    size_t m_prev_canvas_height;
};

void init(size_t max_num_balls, size_t max_canvas_size)
{
    chamber::init<Simple>(max_num_balls, max_canvas_size);
}
