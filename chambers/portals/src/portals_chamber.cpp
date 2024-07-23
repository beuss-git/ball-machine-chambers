#include "portals_chamber.hpp"
#include "images.hpp"
#include <cstddef>
#include <libchamber/exports.h>
#include <libchamber/print.hpp>
#include <ranges>

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
    vec2 rotated_position = vec2_rotate(relative_position, to.rotation() - from.rotation() + deg2rad(180.F));

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
    ball.velocity = vec2_rotate(ball.velocity, exit.rotation() - entrance.rotation() + deg2rad(180.F));

    vec2 exit_normal = exit.normal();

    // Move the ball away from the portal to avoid immediate re-intersection
    ball.pos.x += exit_normal.x * ball.r * 2;
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
            if (vec2 res {}; surface_collision_resolution(&surface_in, &ball->pos, &ball->velocity, &res)) {
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

    draw_image(
        blue_portal_data.data(),
        140, 56,
        (int)pix2pos_x(m_blue_portal.pos().x), (int)pix2pos_y(m_blue_portal.pos().y));

    draw_image(
        orange_portal_data.data(),
        140, 56,
        (int)pix2pos_x(m_orange_portal.pos().x), (int)pix2pos_y(m_orange_portal.pos().y));
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
