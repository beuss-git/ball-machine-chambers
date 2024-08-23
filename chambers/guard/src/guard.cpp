#include "guard.hpp"

#include "image_data.hpp"
#include <libchamber/print.hpp>
#include <ranges>

void init(size_t max_num_balls, size_t max_canvas_size)
{
    // print("Initializing guard chamber!\n");
    chamber::init<GuardChamber>(max_num_balls, max_canvas_size);
}

GuardChamber::GuardChamber(size_t max_balls, size_t max_canvas_size)
    : Chamber(max_balls, max_canvas_size)
{
}

float const STEP_LEN_S = 1.666666f / 1300.0f; // Equivalent to step_len_s in your code

void clamp_speed(ball* ball)
{
    auto const max_speed = 2.5;
    auto const max_speed_2 = max_speed * max_speed;
    auto const ball_speed_2 = vec2_length_2(&ball->velocity);
    if (ball_speed_2 > max_speed_2) {
        auto const ball_speed = sqrt(ball_speed_2);
        ball->velocity = vec2_mul(&ball->velocity, max_speed / ball_speed);
    }
}

void clamp_velocity(vec2* vel)
{
    auto const max_speed = 2.5;
    auto const max_speed_2 = max_speed * max_speed;
    auto const ball_speed_2 = vec2_length_2(vel);
    if (ball_speed_2 > max_speed_2) {
        auto const ball_speed = sqrt(ball_speed_2);
        *vel = vec2_mul(vel, max_speed / ball_speed);
    }
}
void apply_velocity(ball* ball, float delta)
{
    ball->pos = pos2_add(&ball->pos, &ball->velocity);
}
pos2 predict_position(ball const& ball, float prediction_time)
{
    pos2 predicted_pos = ball.pos;
    vec2 velocity = ball.velocity;
    float const G = -9.832f; // Acceleration due to gravity (m/s^2)

    int num_steps = static_cast<int>(prediction_time / STEP_LEN_S);
    float remaining_time = prediction_time - (num_steps * STEP_LEN_S);

    // struct ball ball_copy = ball;
    //  Simulate full steps
    for (int i = 0; i < num_steps; i++) {
        velocity.y += G * STEP_LEN_S;
        clamp_velocity(&velocity);

        predicted_pos.x += velocity.x * STEP_LEN_S;
        predicted_pos.y += velocity.y * STEP_LEN_S;
    }

    // Handle remaining time (less than one full step)
    if (remaining_time > 0) {
        // pos2 before = predicted_pos;
        predicted_pos.x += velocity.x * remaining_time;
        predicted_pos.y += velocity.y * remaining_time;
        // vec2 delta = pos2_sub(&predicted_pos, &before);
        // print("Delta: (%.2f, %.2f)\n", delta.x, delta.y);
        //  We don't update velocity here as it won't be used again
    }

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

    return dot_product < 0.F;
}

GuardChamber::BallResult GuardChamber::find_ball(size_t num_balls, float time_to_target)
{
    if (time_to_target < 0.05F) {
        return BallResult { .ball = nullptr, .state = BallResultState::NOT_FOUND };
    }
    float min_distance = std::numeric_limits<float>::max();
    ball* closest_ball = nullptr;
    BallResultState state = BallResultState::NOT_FOUND;

    for (auto& ball : std::ranges::views::take(m_balls, num_balls)) {
        pos2 predicted_pos
            = predict_position(ball, time_to_target);
        vec2 const delta_pos = pos2_sub(&m_guard.pos, &predicted_pos);

        float const dist = vec2_length(&delta_pos);
        if (dist < min_distance && is_moving_towards(ball, m_guard)) {
            // Ignore if it's outside the bounds
            if (predicted_pos.x < 0.F + (m_guard.radius * 2.F) || predicted_pos.x > 1.F - (m_guard.radius * 2.F)
                || predicted_pos.y < 0.F + (m_guard.radius * 2.F) || predicted_pos.y > 7.F - (m_guard.radius * 2.F)) {
                if (state != BallResultState::FOUND) {
                    state = BallResultState::OUT_OF_BOUNDS;
                }
                continue;
            }
            min_distance = dist;
            closest_ball = &ball;
            state = BallResultState::FOUND;
        }
    }

    return BallResult { .ball = closest_ball, .state = state };
}

void GuardChamber::step(size_t num_balls, float delta)
{
    for (auto& ball : std::ranges::views::take(m_balls, num_balls)) {
        apply_gravity(&ball, delta);
    }

    static constexpr float WANTED_TIME_TO_TARGET = 0.15F;
    m_guard.cooldown_time += delta;
    if (!m_guard.has_target && m_guard.cooldown_time >= 0.10F) {
        float calculated_time_to_target = WANTED_TIME_TO_TARGET;
        BallResult result = find_ball(num_balls, calculated_time_to_target);

        while (result.state == BallResultState::OUT_OF_BOUNDS) {
            calculated_time_to_target -= 0.01F;
            result = find_ball(num_balls, calculated_time_to_target);
        }
        if (result.state == BallResultState::FOUND) {
            m_guard.start_pos = m_guard.pos;
            m_guard.target.ball = result.ball;
            m_guard.has_target = true;
            m_guard.target.time_acc = 0.F;
            m_guard.target.initial_time_to_target = calculated_time_to_target;
        }
    }

    if (m_guard.has_target) {
        float remaining_time = m_guard.target.initial_time_to_target - m_guard.target.time_acc;
        pos2 predicted_pos = predict_position(*m_guard.target.ball, remaining_time);

        // Calculate the offset interception point
        vec2 ball_velocity = m_guard.target.ball->velocity;
        float velocity_magnitude = std::sqrt(ball_velocity.x * ball_velocity.x + ball_velocity.y * ball_velocity.y);

        if (velocity_magnitude > 0) {
            vec2 normalized_velocity = { ball_velocity.x / velocity_magnitude, ball_velocity.y / velocity_magnitude };
            // This one is correct, but not as interesting
            float offset = m_guard.target.ball->r + m_guard.radius - 0.02F /* Add small offset to make it more interesting */;
            // float offset = m_guard.radius;

            // Adjust the predicted position to be just ahead of the ball
            predicted_pos.x += normalized_velocity.x * offset;
            predicted_pos.y += normalized_velocity.y * offset;
        }

        // Lerp towards target.
        float t = std::clamp<float>(m_guard.target.time_acc / m_guard.target.initial_time_to_target, 0.F, 1.F);
        m_guard.pos = {
            .x = lerp(m_guard.start_pos.x, predicted_pos.x, t),
            .y = lerp(m_guard.start_pos.y, predicted_pos.y, t),
        };

        if (t >= 1.F) {
            m_guard.has_target = false;
            m_guard.cooldown_time = 0.F;
            m_guard.start_pos = m_guard.pos;
        }
        m_guard.target.time_acc += delta;
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
                        m_canvas[index] = 0xFFFF00FF;
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
    // draw_circle(pos2pix_x(0.9), pos2pix_y(0.0), pos2pix_x(m_guard.radius));

    // print("Guard position: (%.2f, %.2f)\n", m_guard.pos.x, m_guard.pos.y);
    draw_image(
        orb_data.data(),
        20, 22,
        (int)pos2pix_x(m_guard.pos.x), (int)pos2pix_y(m_guard.pos.y));

    // if (m_guard.has_target) {
    //
    //     float remaining_time = m_guard.target.initial_time_to_target - m_guard.target.time_acc;
    //     pos2 predicted_pos = predict_position(*m_guard.target.ball, remaining_time);
    //     draw_circle(pos2pix_x(predicted_pos.x), pos2pix_y(predicted_pos.y), 10);
    // }
    // draw_circle(pos2pix_x(m_guard.target.pos.x), pos2pix_y(m_guard.target.ball.pos.y), 10);
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
