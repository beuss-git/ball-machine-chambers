#include "BallMachine.hpp"

#include <cstddef>
#include <cstdlib>

void draw_line(canvas_ity::canvas& ctx, float x1, float y1, float x2, float y2)
{
    ctx.begin_path();
    ctx.move_to(x1, y1);
    ctx.line_to(x2, y2);
    ctx.stroke();
}

void BallMachine::render(size_t canvas_width, size_t canvas_height)
{
    if (m_last_canvas_width == canvas_width && m_last_canvas_height == canvas_height) {
        return;
    }

    m_canvas.clear();

    m_last_canvas_width = canvas_width;
    m_last_canvas_height = canvas_height;

    canvas_ity::canvas ctx(static_cast<int>(canvas_width), static_cast<int>(canvas_height));

    for (auto const& surface : m_surfaces) {
        auto const a = pix2pos(surface.a, canvas_width, canvas_height);
        auto const b = pix2pos(surface.b, canvas_width, canvas_height);
        draw_line(ctx, a.x, a.y, b.x, b.y);
    }

    ctx.get_image_data(reinterpret_cast<unsigned char*>(m_canvas.canvas_memory()),
        static_cast<int>(canvas_width), static_cast<int>(canvas_height),
        static_cast<int>(canvas_width * 4),
        0, 0);
}
