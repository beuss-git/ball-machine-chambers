#include "BallMachine.hpp"

#include "printer.hpp"
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <numbers>

void draw_line(canvas_ity::canvas& ctx, float x1, float y1, float x2, float y2)
{
    ctx.begin_path();
    ctx.move_to(x1, y1);
    ctx.line_to(x2, y2);
    ctx.stroke();
}

void BallMachine::draw_portal(canvas_ity::canvas& context, float cx, float cy, float rx, float ry, Color const& portal_color, float rotation)
{
    print("Drawing portal at cx=%f, cy=%f, rx=%f, ry=%f, color=(%f, %f, %f)\n", cx, cy, rx, ry, portal_color.r, portal_color.g, portal_color.b);
    int const num_segments = 100;                                          // Increase for smoother ellipse
    float const angle_step = 2 * std::numbers::pi_v<float> / num_segments; // Full circle divided by the number of segments

    // Start the path for the inner portal
    context.begin_path();
    for (int i = 0; i <= num_segments; ++i) {
        float angle = static_cast<float>(i) * angle_step;
        float x = rx * cos(angle);
        float y = ry * sin(angle);

        float rotated_x = cos(rotation) * x - sin(rotation) * y + cx;
        float rotated_y = sin(rotation) * x + cos(rotation) * y + cy;

        if (i == 0) {
            context.move_to(pix2pos_x(rotated_x), pix2pos_y(rotated_y));
        } else {
            context.line_to(pix2pos_x(rotated_x), pix2pos_y(rotated_y));
        }
    }
    context.close_path();

    // Fill the inner portal with solid color
    context.set_color(canvas_ity::fill_style, portal_color.r, portal_color.g, portal_color.b, 1.0F);
    context.fill();

    // Set up shadow for glow effect
    context.set_shadow_blur(15.0F);                                                  // Increase or decrease for stronger or weaker glow
    context.set_shadow_color(portal_color.r, portal_color.g, portal_color.b, 0.75F); // Glow color and intensity

    // Draw the glowing edge around the portal
    float glow_factor = 1.1F;
    context.begin_path();
    for (int i = 0; i <= num_segments; ++i) {
        float angle = static_cast<float>(i) * angle_step;
        float x = rx * glow_factor * cos(angle);
        float y = ry * glow_factor * sin(angle);

        float rotated_x = cos(rotation) * x - sin(rotation) * y + cx;
        float rotated_y = sin(rotation) * x + cos(rotation) * y + cy;

        if (i == 0) {
            context.move_to(pix2pos_x(rotated_x), pix2pos_y(rotated_y));
        } else {
            context.line_to(pix2pos_x(rotated_x), pix2pos_y(rotated_y));
        }
    }
    context.close_path();

    // Fill the edge to activate the shadow glow effect
    context.fill(); // This fill is for the shadow to take effect

    // Turn off shadow for other drawings
    context.set_shadow_color(0.0f, 0.0f, 0.0f, 0.0f);
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
        auto const a = pix2pos(surface.a);
        auto const b = pix2pos(surface.b);
        draw_line(ctx, a.x, a.y, b.x, b.y);
    }

    // Color blue_portal_color = { 0.0F, 0.7F, 1.0F }; // A typical blue portal color
    //  draw_portal(ctx, 0.5F, 0.5F, 0.15F, 0.05F, blue_portal_color);

    for (auto const& portal : m_portals) {
        draw_portal(ctx, portal.pos().x, portal.pos().y, portal.rad_x(), portal.rad_y(), portal.color(), portal.rotation());
        // draw_portal_2(ctx, portal.pos().x, portal.pos().y, portal.rad_x(), portal.rad_y(), portal.rotation());
    }

    ctx.get_image_data(
        reinterpret_cast<unsigned char*>(m_canvas.canvas_memory()),
        static_cast<int>(canvas_width), static_cast<int>(canvas_height),
        static_cast<int>(canvas_width * 4),
        0, 0);
}

// Function to rotate a vector by a given angle
vec2 vec2_rotate(vec2 const& vec, float angle)
{
    float cos_a = cos(angle);
    float sin_a = sin(angle);
    return {
        vec.x * cos_a - vec.y * sin_a,
        vec.x * sin_a + vec.y * cos_a
    };
}
vec2 portal_normal(Portal const& portal)
{
    // Calculate normal vector from portal's rotation (rotate by 90 degrees to get the normal)
    float normal_rotation = portal.rotation() + std::numbers::pi_v<float> / 2.0F; // 90 degrees in radians
    return {
        cos(normal_rotation),
        sin(normal_rotation)
    };
}
vec2 reflect_vector(vec2 v, vec2 n)
{
    float dot_product = v.x * n.x + v.y * n.y;
    return {
        v.x - 2 * dot_product * n.x,
        v.y - 2 * dot_product * n.y
    };
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
    vec2 exit_normal = portal_normal(exit);
    ball.velocity = reflect_vector(rotated_velocity, exit_normal);

    // Move the ball away from the portal to avoid immediate re-intersection
    ball.pos.x += exit_normal.x * ball.r;
    ball.pos.y += exit_normal.y * ball.r;
}
void BallMachine::step(size_t num_balls, float delta)
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

    for (size_t i = 0; i < num_balls; ++i) {
        ball* ball = &m_balls[i];
        for (size_t j = 0; j < m_portals.size(); ++j) {
            auto const& entry_portal = m_portals.at(j);
            auto const& exit_portal = m_portals.at((j + 1) % m_portals.size());

            auto surface_in = entry_portal.calculate_surface();
            if (vec2 res {}; surface_collision_resolution(&surface_in, &ball->pos, &ball->velocity, &res)) {
                teleport_ball(*ball, entry_portal, exit_portal);
            }
        }
    }
}
