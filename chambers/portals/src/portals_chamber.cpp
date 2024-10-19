#include "portals_chamber.hpp"
#include "images.hpp"
#include <cstddef>
#include <libchamber/exports.h>
#include <libchamber/print.hpp>
#include <ranges>

void draw_line(canvas_ity::canvas& context, float x1, float y1, float x2, float y2)
{
    context.begin_path();
    context.move_to(x1, y1);
    context.line_to(x2, y2);
    context.close_path();
    context.set_color(canvas_ity::stroke_style, 0, 0, 0, 1.0F);
    context.stroke();
}

void init(size_t max_num_balls, size_t max_canvas_size)
{
    chamber::init<Portals>(max_num_balls, max_canvas_size);
}

pos2 transform_point_portal_to_portal(pos2 point, Portal const& from, Portal const& to)
{
    // Transform the point relative to the center of the entrance portal to the exit portal
    vec2 relative_position = {
        point.x - from.pos().x,
        point.y - from.pos().y
    };

    // Rotate relative_position to align with the exit portal
    vec2 rotated_position = vec2_rotate(relative_position, to.rotation() - from.rotation());

    // Translate to the exit portal's position
    return {
        to.pos().x + rotated_position.x,
        to.pos().y + rotated_position.y
    };
}

void teleport_ball(ball& ball, Portal const& entrance, Portal const& exit)
{
    // Transform the ball's position
    ball.pos = transform_point_portal_to_portal(ball.pos, entrance, exit);

    // Rotate the velocity to align with the exit portal's context
    float angle_diff = exit.rotation() - entrance.rotation();

    float vx = ball.velocity.x * std::cos(angle_diff) - ball.velocity.y * std::sin(angle_diff);
    float vy = ball.velocity.x * std::sin(angle_diff) + ball.velocity.y * std::cos(angle_diff);
    ball.velocity = { vx, vy };

    vec2 exit_normal = exit.normal();

    // Move the ball away from the portal to avoid immediate re-intersection
    ball.pos.x += exit_normal.x * ball.r * 2;
}

pos2 closest_point_on_segment(pos2 c, pos2 a, pos2 b)
{
    vec2 ab = { b.x - a.x, b.y - a.y };
    vec2 ac = { c.x - a.x, c.y - a.y };
    float ab_ab = ab.x * ab.x + ab.y * ab.y; // Dot product of ab with itself
    float ab_ac = ab.x * ac.x + ab.y * ac.y; // Dot product of ab with ac
    float t = ab_ac / ab_ab;

    // Clamp t from 0.0 to 1.0 to keep the projection within the segment
    t = fmax(0.0, fmin(1.0, t));

    // Compute the closest point
    pos2 closest {};
    closest.x = a.x + t * ab.x;
    closest.y = a.y + t * ab.y;
    return closest;
}

bool check_collision(ball& b, surface& s)
{
    // Calculate normal
    vec2 normal = { s.b.y - s.a.y, s.a.x - s.b.x };

    // Check if the ball is moving towards the normal
    if (vec2_dot(&b.velocity, &normal) < 0) {
        return false; // Not moving towards the surface from the correct side
    }

    // Find the closest point on the segment to the ball's center
    pos2 closest = closest_point_on_segment(b.pos, s.a, s.b);

    // Calculate distance from closest point on segment to ball center
    float dx = closest.x - b.pos.x;
    float dy = closest.y - b.pos.y;
    float distance = sqrt(dx * dx + dy * dy);

    // Check if the distance is less than or equal to the ball's radius
    if (distance <= (b.r / 10.f)) {
        return true;
    }

    return false;
}

void Portals::step(size_t num_balls, float delta)
{
    for (auto& ball : std::ranges::views::take(m_balls, num_balls)) {
        apply_gravity(&ball, delta);
    }

    std::array<Portal, 2> portals = { m_blue_portal, m_orange_portal };
    for (size_t i = 0; i < num_balls; ++i) {
        ball* ball = &m_balls[i];
        for (size_t j = 0; j < portals.size(); ++j) {
            auto const& entry_portal = portals.at(j);
            auto const& exit_portal = portals.at((j + 1) % portals.size());

            auto surface_in = entry_portal.calculate_surface();
            if (check_collision(*ball, surface_in)) {
                teleport_ball(*ball, entry_portal, exit_portal);
            }
        }
    }

    // m_blue_portal.update_position(delta);
    // m_orange_portal.update_position(delta);
}

void Portals::render(size_t canvas_width, size_t canvas_height)
{
    if (m_canvas_width == canvas_width && m_canvas_height == canvas_height) {
        return;
    }
    m_canvas_width = canvas_width;
    m_canvas_height = canvas_height;

    auto const fill_screen = [this](uint32_t color) {
        for (size_t i = 0; i < m_canvas_width * m_canvas_height; ++i) {
            m_canvas[i] = color;
        }
    };
    fill_screen(0xFFFFFFFF);

    // m_ctx.set_color(canvas_ity::fill_style, 1, 1, 1, 1.0F);
    // m_ctx.fill_rectangle(0, 0, canvas_width, canvas_height);

    draw_image(
        blue_portal_data.data(),
        140, 56,
        (int)pix2pos_x(m_blue_portal.pos().x), (int)pix2pos_y(m_blue_portal.pos().y));

    draw_image(
        orange_portal_data.data(),
        140, 56,
        (int)pix2pos_x(m_orange_portal.pos().x), (int)pix2pos_y(m_orange_portal.pos().y));

    //     std::array<Portal, 2> portals = { m_blue_portal, m_orange_portal };
    //     for (auto const& portal : portals) {
    //         auto const surf = portal.calculate_surface();
    //         draw_line(m_ctx,
    //             pix2pos_x(surf.a.x),
    //             pix2pos_y(surf.a.y),
    //             pix2pos_x(surf.b.x),
    //             pix2pos_y(surf.b.y));
    //     }
    //
    //     m_ctx.get_image_data(
    //         reinterpret_cast<unsigned char*>(m_canvas.data()),
    //         static_cast<int>(canvas_width), static_cast<int>(canvas_height),
    //         static_cast<int>(canvas_width * 4),
    //         0, 0);
}

void Portals::draw_image(uint32_t const* data, int image_width, int image_height, int x, int y)
{
    vec2 middle = { image_width / 2.F, image_height / 2.F };

    for (int i = 0; i < image_width; ++i) {
        for (int j = 0; j < image_height; ++j) {
            int index = i + j * image_width;

            int x_pos = x - middle.x + i;
            int y_pos = y - middle.y + j;
            if (x_pos < 0 || x_pos >= m_canvas_width || y_pos < 0 || y_pos >= m_canvas_height) {
                continue;
            }

            uint32_t src = data[index];
            uint32_t dst = m_canvas.at(x_pos + y_pos * m_canvas_width);

            uint8_t srcAlpha = (src >> 24) & 0xFF;
            uint8_t srcRed = (src >> 16) & 0xFF;
            uint8_t srcGreen = (src >> 8) & 0xFF;
            uint8_t srcBlue = src & 0xFF;

            uint8_t dstAlpha = (dst >> 24) & 0xFF;
            uint8_t dstRed = (dst >> 16) & 0xFF;
            uint8_t dstGreen = (dst >> 8) & 0xFF;
            uint8_t dstBlue = dst & 0xFF;

            // Perform alpha blending for each component
            float srcAlphaNormalized = srcAlpha / 255.0f;
            uint8_t newRed = srcRed * srcAlphaNormalized + dstRed * (1 - srcAlphaNormalized);
            uint8_t newGreen = srcGreen * srcAlphaNormalized + dstGreen * (1 - srcAlphaNormalized);
            uint8_t newBlue = srcBlue * srcAlphaNormalized + dstBlue * (1 - srcAlphaNormalized);
            uint8_t newAlpha = dstAlpha; // Optionally, calculate the new alpha as well if needed

            // Combine into a single uint32_t
            uint32_t newColor = static_cast<uint32_t>((newAlpha << 24) | (newRed << 16) | (newGreen << 8) | newBlue);
            if (x_pos + y_pos * m_canvas_width >= (m_canvas_width * m_canvas_height) * 4) {
                continue;
            }
            // continue;
            // m_canvas.at(x_pos + y_pos * m_canvas_width) = 0xFFFFFFFF;
            // m_canvas.at(x_pos + y_pos * m_canvas_width) = 0x000000FF;
            m_canvas.at(x_pos + y_pos * m_canvas_width) = newColor;
        }
    }
}
