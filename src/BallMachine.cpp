#include "BallMachine.hpp"

#include "printer.hpp"
#include <algorithm>
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
    // print("Drawing portal at cx=%f, cy=%f, rx=%f, ry=%f, color=(%f, %f, %f)\n", cx, cy, rx, ry, portal_color.r, portal_color.g, portal_color.b);
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
            context.move_to((rotated_x), (rotated_y));
        } else {
            context.line_to((rotated_x), (rotated_y));
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
            context.move_to((rotated_x), (rotated_y));
        } else {
            context.line_to((rotated_x), (rotated_y));
        }
    }
    context.close_path();

    // Fill the edge to activate the shadow glow effect
    context.fill(); // This fill is for the shadow to take effect

    // Turn off shadow for other drawings
    context.set_shadow_color(0.0F, 0.0F, 0.0F, 0.0F);
}

vec2 vec2_rotate(vec2 const& vec, float angle)
{
    float cos_a = cos(angle);
    float sin_a = sin(angle);
    return {
        vec.x * cos_a - vec.y * sin_a,
        vec.x * sin_a + vec.y * cos_a
    };
}

bounds calculate_image_bounds(Image const& image)
{
    int minX = image.width;
    int maxX = 0;
    int minY = image.height;
    int maxY = 0;

    // Lambda to check if a pixel is non-transparent
    auto const is_pixel_visible = [&](int index) {
        // int alpha = image.data[index] & 0xFF;
        //   Alpha is the last byte of the pixel data
        int alpha = (image.data[index] >> 24) & 0xFF;
        return alpha != 0;
    };

    for (int y = 0; y < image.height; ++y) {
        for (int x = 0; x < image.width; ++x) {
            int index = y * image.width + x;
            if (is_pixel_visible(index)) {
                minX = std::min(minX, x);
                maxX = std::max(maxX, x);
                minY = std::min(minY, y);
                maxY = std::max(maxY, y);
            }
        }
    }

    // Adjust the bounds to ensure the sprite is enclosed
    bounds result;
    result.left = minX;
    result.right = maxX;
    result.top = minY;
    result.bottom = maxY;

    return result;
}

Image crop_and_resize_image(Image const& sourceImage, bounds const& b)
{
    // Calculate the new dimensions
    int new_width = b.right - b.left + 1;
    int new_height = b.bottom - b.top + 1;

    // Create a new image with the new dimensions
    Image cropped_image;
    cropped_image.width = new_width;
    cropped_image.height = new_height;
    cropped_image.data.resize(new_width * new_height);

    // Copy the pixels from the source image to the new image
    for (int y = b.top; y <= b.bottom; ++y) {
        for (int x = b.left; x <= b.right; ++x) {
            int source_index = y * sourceImage.width + x;
            int dest_index = (y - b.top) * new_width + (x - b.left);
            cropped_image.data[dest_index] = sourceImage.data[source_index];
        }
    }

    return cropped_image;
}

Image BallMachine::render_portal_to_texture(canvas_ity::canvas& context, size_t canvas_width, size_t canvas_height, Portal const& portal)
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

void BallMachine::put_image(canvas_ity::canvas& context, size_t x, size_t y, Image const& image)
{
    context.put_image_data(
        (unsigned char*)(image.data.data()),
        static_cast<int>(image.width), static_cast<int>(image.height),
        static_cast<int>(image.width * 4),
        x, y);
}

void draw_image(canvas_ity::canvas& ctx, Image const& img, int x, int y)
{
    ctx.draw_image(reinterpret_cast<unsigned char const*>(img.data.data()), img.width, img.height, img.width * 4, x - (img.width / 2.F), y - (img.height / 2.F), img.width, img.height);
}

void BallMachine::render(size_t canvas_width, size_t canvas_height)
{
    // if (m_last_canvas_width == canvas_width && m_last_canvas_height == canvas_height) {
    // return;
    //}

    print("Rendering Ball Machine with canvas size %zu x %zu\n", canvas_width, canvas_height);
    m_canvas.clear();

    m_last_canvas_width = canvas_width;
    m_last_canvas_height = canvas_height;

    m_ctx.clear_rectangle(0, 0, canvas_width, canvas_height);
    m_ctx.set_color(canvas_ity::fill_style, 1.0F, 1.0F, 1.0F, 1.0F);
    m_ctx.fill_rectangle(0, 0, canvas_width, canvas_height);
    //
    // for (auto const& surface : m_surfaces) {
    //     auto const a = pix2pos(surface.a);
    //     auto const b = pix2pos(surface.b);
    //     draw_line(m_ctx, a.x, a.y, b.x, b.y);
    // }

    // Color blue_portal_color = { 0.0F, 0.7F, 1.0F }; // A typical blue portal color
    //  draw_portal(ctx, 0.5F, 0.5F, 0.15F, 0.05F, blue_portal_color);

    // std::array<Portal, 2> const portals = { m_blue_portal, m_orange_portal };
    // for (auto const& portal : portals) {
    //  draw_portal(m_ctx, portal.pos().x, portal.pos().y, portal.rad_x(), portal.rad_y(), portal.color(), portal.rotation());
    //   draw_portal_2(ctx, portal.pos().x, portal.pos().y, portal.rad_x(), portal.rad_y(), portal.rotation());
    //}
    // m_orange_portal_texture = Image {
    //     .width = 100,
    //     .height = 100,
    //     .data = std::vector<int32_t>(100 * 100),
    // };
    // std::fill(m_orange_portal_texture.data.begin(), m_orange_portal_texture.data.end(), 0xFF0000FF);

    draw_image(m_ctx, m_orange_portal_texture, pix2pos_x(m_orange_portal.pos().x), pix2pos_y(m_orange_portal.pos().y));
    draw_image(m_ctx, m_blue_portal_texture, pix2pos_x(m_blue_portal.pos().x), pix2pos_y(m_blue_portal.pos().y));
    //  put_image(m_ctx, 0, 0, m_orange_portal_texture);
    //   put_image(m_ctx, 0, 0, m_blue_portal_texture);
    //    put_image(m_ctx, 0.1F, 0.1F, m_orange_portal_texture);
    //     put_image(m_ctx, m_orange_portal.pos().x, m_orange_portal.pos().y, m_orange_portal_texture);
    //

    m_ctx.get_image_data(
        reinterpret_cast<unsigned char*>(m_canvas.canvas_memory()),
        static_cast<int>(canvas_width), static_cast<int>(canvas_height),
        static_cast<int>(canvas_width * 4),
        0, 0);
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
    m_orange_portal.update_position(delta);
}
