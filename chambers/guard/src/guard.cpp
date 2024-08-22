#include "guard.hpp"

#include "image_data.hpp"
#include <libchamber/print.hpp>
#include <ranges>

void init(size_t max_num_balls, size_t max_canvas_size)
{
    chamber::init<GuardChamber>(max_num_balls, max_canvas_size);
}

GuardChamber::GuardChamber(size_t max_balls, size_t max_canvas_size)
    : Chamber(max_balls, max_canvas_size)
{
}

pos2 predict_position(ball const& ball, float seconds)
{
    pos2 predicted_pos {};

    // Initial position
    predicted_pos.x = ball.pos.x + ball.velocity.x * seconds;

    // Gravity's effect on vertical position
    predicted_pos.y = ball.pos.y + (ball.velocity.y) * seconds + 0.5 * (-9.832) * seconds * seconds;

    return predicted_pos;
}

float lerp(float a, float b, float t)
{
    return a + (b - a) * t;
}

vec2 reflect(vec2 const* v, vec2 const* n)
{
    float dot = v->x * n->x + v->y * n->y;
    return {
        v->x - 2 * dot * n->x,
        v->y - 2 * dot * n->y
    };
}

void handle_collision(ball& ball, Guard& guard, vec2 guard_velocity)
{
    vec2 delta_pos = pos2_sub(&guard.pos, &ball.pos);
    float dist = vec2_length(&delta_pos);
    if (dist < guard.radius + ball.r) {
        vec2 norm = vec2_normalized(&delta_pos);
        vec2 relative_velocity = vec2_sub(&ball.velocity, &guard_velocity);
        vec2 reflected_velocity = reflect(&relative_velocity, &norm);
        ball.velocity.x = reflected_velocity.x + guard_velocity.x;
        ball.velocity.y = reflected_velocity.y + guard_velocity.y;
    }
}

bool resolve_collision(ball& ball, Guard const& guard)
{
    vec2 delta_pos = pos2_sub(&guard.pos, &ball.pos);
    vec2 n = vec2_normalized(&delta_pos); // Normal of collision
    float dist = vec2_length(&delta_pos);

    // Check for collision
    if (dist < guard.radius + ball.r) {
        // Reflect ball's velocity
        float v_dot_n = vec2_dot(&ball.velocity, &n);
        vec2 vn = vec2_mul(&n, v_dot_n);
        vec2 vt = vec2_sub(&ball.velocity, &vn);
        vec2 new_velocity = vec2_sub(&vt, &vn);

        ball.velocity = vec2_mul(&new_velocity, 4.F); // Updating velocity to the reflected velocity
        // Move ball out of guard
        ball.pos.x = guard.pos.x + -n.x * (guard.radius + ball.r);
        ball.pos.y = guard.pos.y + -n.y * (guard.radius + ball.r);
        return true;
    }
    return false;
}

vec2 direction_vector(pos2 const& from, pos2 const& to)
{
    return { to.x - from.x, to.y - from.y };
}
bool is_moving_towards(ball const& ball, Guard const& guard)
{
    vec2 relative_velocity = ball.velocity; // Assuming guard is stationary
    vec2 direction_to_ball = direction_vector(guard.pos, ball.pos);

    float dot_product = vec2_dot(&relative_velocity, &direction_to_ball);

    return dot_product < 0;
}

void GuardChamber::step(size_t num_balls, float delta)
{
    for (auto& ball : std::ranges::views::take(m_balls, num_balls)) {
        apply_gravity(&ball, delta);

        if (ball.velocity.y > -10.F) {
            ball.velocity.y = -10.F;
        }
    }

    static constexpr float TIME_TO_TARGET = 0.07F;
    m_guard.cooldown_time += delta;
    if (!m_guard.has_target && m_guard.cooldown_time >= 0.02F) {
        // Find the closest ball to the guard
        float min_distance = std::numeric_limits<float>::min();
        ball* closest_ball = nullptr;
        for (auto& ball : std::ranges::views::take(m_balls, num_balls)) {

            pos2 predicted_pos
                = predict_position(ball, TIME_TO_TARGET + 0.01F);
            vec2 const delta_pos = pos2_sub(&m_guard.pos, &predicted_pos);

            float const dist = vec2_length(&delta_pos);
            if (dist > min_distance && is_moving_towards(ball, m_guard)) {
                print("Predicted position: (%.2f, %.2f)\n", predicted_pos.x, predicted_pos.y);
                // Ignore if it's outside the bounds
                if (predicted_pos.x < 0.F || predicted_pos.x > 1.F
                    || predicted_pos.y < 0.F || predicted_pos.y > 7.F) {
                    continue;
                }
                min_distance = dist;
                closest_ball = &ball;
            }
        }

        if (closest_ball != nullptr) {
            pos2 predicted_pos
                = predict_position(*closest_ball, TIME_TO_TARGET + 0.01F);
            // Subtract radius from predicted_pos in normal direciton
            // vec2 delta_pos = pos2_sub(&m_guard.pos, &predicted_pos);
            // vec2 norm = vec2_normalized(&delta_pos);
            // predicted_pos.x = predicted_pos.x + norm.x * (closest_ball->r + m_guard.radius);
            // predicted_pos.y = predicted_pos.y + norm.y * (closest_ball->r + m_guard.radius);

            m_guard.start_pos = m_guard.pos;
            m_guard.target_pos = predicted_pos;
            m_guard.has_target = true;
            m_guard.time = 0.F;
            // print("Guard has target at (%.2f, %.2f)\n", m_guard.target_pos.x, m_guard.target_pos.y);
        }
    }

    pos2 old_guard_pos = m_guard.pos;
    if (m_guard.has_target) {
        // Lerp towards target.
        m_guard.pos = {
            .x = lerp(m_guard.start_pos.x, m_guard.target_pos.x, m_guard.time / TIME_TO_TARGET),
            .y = lerp(m_guard.start_pos.y, m_guard.target_pos.y, m_guard.time / TIME_TO_TARGET),
        };

        m_guard.time += delta;
        if (m_guard.time / TIME_TO_TARGET >= 1.F) {
            m_guard.has_target = false;
            m_guard.cooldown_time = 0.F;
        }
        // print("Guard time: %.2f\n", m_guard.time / TIME_TO_TARGET);
    }

    // If we are intersecting with the target, collide with it and bounce the target ball away. The guard is immovable.
    for (auto& ball : std::ranges::views::take(m_balls, num_balls)) {
        if (resolve_collision(ball, m_guard)) {
            m_guard.has_target = false;
            m_guard.cooldown_time = 0.F;
        }
    }
}

void GuardChamber::render(size_t canvas_width, size_t canvas_height)
{
    // if (m_prev_canvas_width == canvas_width && m_prev_canvas_height == canvas_height) {
    //     return;
    // }
    m_canvas_width = canvas_width;
    m_canvas_height = canvas_height;
    //
    // Do rendering..
    //
    auto const draw_horizontal_line = [&](int x1, int x2, int y) {
        for (int i = x1; i < x2; i++) {
            m_canvas[i + y * canvas_width] = 0xFF000000;
        }
    };
    auto const draw_circle = [&](int x, int y, int radius) {
        for (int i = -radius; i < radius; i++) {
            for (int j = -radius; j < radius; j++) {
                if (i * i + j * j < radius * radius) {
                    auto const index = (x + i) + (y + j) * canvas_width;
                    if (index >= 0 && index < canvas_width * canvas_height) {
                        m_canvas[index] = 0xFF00FFFF;
                    }
                }
            }
        }
    };

    auto const fill_screen = [this](uint32_t color) {
        for (size_t i = 0; i < m_canvas_width * m_canvas_height; ++i) {
            m_canvas[i] = color;
        }
    };
    fill_screen(0xFFFFFFFF);

    auto const pos2pix_x = [&](float x_norm) {
        return x_norm * static_cast<float>(canvas_width);
    };
    auto const pos2pix_y = [&](float y_norm) {
        return static_cast<float>(canvas_height) - y_norm * static_cast<float>(canvas_width);
    };

    // print("Drawing Guard position: (%.2f, %.2f)\n", m_guard.pos.x, m_guard.pos.y);
    // draw_circle(pos2pix_x(m_guard.pos.x), pos2pix_y(m_guard.pos.y), pos2pix_x(m_guard.radius));
    draw_circle(pos2pix_x(0.9), pos2pix_y(0.0), pos2pix_x(m_guard.radius));

    draw_image(
        orb_data.data(),
        20, 22,
        (int)pos2pix_x(m_guard.pos.x), (int)pos2pix_y(m_guard.pos.y));
}

void GuardChamber::draw_image(uint32_t const* data, int image_width, int image_height, int x, int y)
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
