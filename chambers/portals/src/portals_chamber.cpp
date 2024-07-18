#include "portals_chamber.hpp"
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
    vec2 rotated_velocity = vec2_rotate(ball.velocity, exit.rotation() - entrance.rotation());

    // Reflect the velocity based on the exit portal's normal
    vec2 exit_normal = exit.normal();
    ball.velocity = vec2_reflect(rotated_velocity, exit_normal);

    // Move the ball away from the portal to avoid immediate re-intersection
    ball.pos.x += exit_normal.x * ball.r * 2;
}

void Portals::step(size_t num_balls, float delta)
{
    for (auto& ball : std::ranges::views::take(m_balls, num_balls)) {
        apply_gravity(&ball, delta);
    }
    print("Stepping Ball Machine with %zu balls and delta %f\n", num_balls, delta);

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

    m_orange_portal.update_position(delta);
}

void Portals::render(size_t canvas_width, size_t canvas_height)
{
    m_canvas_width = canvas_width;
    m_canvas_height = canvas_height;

    // Draw white background
    m_ctx.set_color(canvas_ity::fill_style, 1.0F, 1.0F, 1.0F, 1.0F);
    m_ctx.fill_rectangle(0, 0, canvas_width, canvas_height);

    draw_image(m_ctx, m_orange_portal_texture, pix2pos_x(m_orange_portal.pos().x), pix2pos_y(m_orange_portal.pos().y));
    draw_image(m_ctx, m_blue_portal_texture, pix2pos_x(m_blue_portal.pos().x), pix2pos_y(m_blue_portal.pos().y));

    m_ctx.get_image_data(
        reinterpret_cast<unsigned char*>(m_canvas.data()),
        static_cast<int>(canvas_width), static_cast<int>(canvas_height),
        static_cast<int>(canvas_width * 4),
        0, 0);
}

Image Portals::render_portal_to_texture(canvas_ity::canvas& context, size_t canvas_width, size_t canvas_height, Portal const& portal)
{
    m_ctx.clear_rectangle(0, 0, canvas_width, canvas_height);

    draw_portal(context, canvas_width / 2.f, canvas_height / 2.f, portal.rad_x() * canvas_width, portal.rad_y() * canvas_height, portal.color(), portal.rotation());

    Image render_texture;
    render_texture.data.clear();
    render_texture.data.resize(canvas_width * canvas_height * 4);
    render_texture.width = canvas_width;
    render_texture.height = canvas_height;

    context.get_image_data(
        reinterpret_cast<unsigned char*>(render_texture.data.data()),
        static_cast<int>(canvas_width), static_cast<int>(canvas_height),
        static_cast<int>(canvas_width * 4),
        0, 0);

    auto bounds = calculate_image_bounds(render_texture);
    print("Rendered portal texture with bounds left=%f, right=%f, top=%f, bottom=%f\n", bounds.left, bounds.right, bounds.top, bounds.bottom);
    auto new_image = crop_and_resize_image(render_texture, bounds);
    print("Cropped portal texture with width=%d, height=%d\n", new_image.width, new_image.height);
    return new_image;
}

void Portals::draw_portal(canvas_ity::canvas& context, float cx, float cy, float rx, float ry, Color const& portal_color, float rotation)
{
    int const num_segments = 100;                                          // Increase for smoother ellipse
    float const angle_step = 2 * std::numbers::pi_v<float> / num_segments; // Full circle divided by the number of segments

    rotation = -rotation; // Rotate the portal by 180 degrees to align with the canvas
    // Rotation adjustments
    float cos_rot = std::cos(rotation);
    float sin_rot = std::sin(rotation);

    // Start the path for the portal
    context.begin_path();
    for (int i = 0; i <= num_segments; ++i) {
        float angle = static_cast<float>(i) * angle_step;
        float x = rx * std::cos(angle); // Initial x, y calculations without rotation
        float y = ry * std::sin(angle);

        // Apply rotation
        float rotated_x = cos_rot * x - sin_rot * y + cx;
        float rotated_y = sin_rot * x + cos_rot * y + cy;

        if (i == 0) {
            context.move_to(rotated_x, rotated_y);
        } else {
            context.line_to(rotated_x, rotated_y);
        }
    }
    context.close_path();

    // Fill the inner portal with solid color
    context.set_color(canvas_ity::fill_style, portal_color.r, portal_color.g, portal_color.b, 1.0F);
    context.fill();

    // Set up shadow for glow effect
    context.set_shadow_blur(15.0F);                                                  // Increase or decrease for stronger or weaker glow
    context.set_shadow_color(portal_color.r, portal_color.g, portal_color.b, 0.95F); // Glow color and intensity

    // Draw the glowing edge around the portal
    float glow_factor = 1.1F;
    context.begin_path();
    for (int i = 0; i <= num_segments; ++i) {
        float angle = static_cast<float>(i) * angle_step;
        float x = rx * glow_factor * std::cos(angle); // Initial x, y calculations without rotation
        float y = ry * glow_factor * std::sin(angle);

        // Apply rotation
        float rotated_x = cos_rot * x - sin_rot * y + cx;
        float rotated_y = sin_rot * x + cos_rot * y + cy;

        if (i == 0) {
            context.move_to(rotated_x, rotated_y);
        } else {
            context.line_to(rotated_x, rotated_y);
        }
    }
    context.close_path();

    // // Fill the edge to activate the shadow glow effect
    context.fill(); // This fill is for the shadow to take effect

    // Turn off shadow for other drawings
    context.set_shadow_color(0.0F, 0.0F, 0.0F, 0.0F);

    context.set_color(canvas_ity::stroke_style, 0, 0, 0, 1.0F); // Set stroke color to black
    context.set_line_width(2.0F);                               // Set the line width for the border
    context.stroke();
}
