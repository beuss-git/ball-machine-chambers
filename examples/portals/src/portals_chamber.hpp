#ifndef PORTALS_HPP
#define PORTALS_HPP
#include "portal.hpp"
#include <canvas_ity/canvas_ity.hpp>
#include <libchamber/chamber.hpp>
#include <libchamber/print.hpp>

class Portals : public chamber::Chamber {
public:
    Portals(Portals const&) = delete;
    Portals(Portals&&) = delete;
    Portals& operator=(Portals const&) = delete;
    Portals& operator=(Portals&&) = delete;
    ~Portals() override = default;

    static vec2 compute_width_height(size_t max_canvas_size)
    {
        auto const aspect_ratio = 1.F / 0.7F;
        auto const max_canvas_height = std::sqrt(static_cast<float>(max_canvas_size) / aspect_ratio);
        auto const max_canvas_width = max_canvas_height * aspect_ratio;
        return { max_canvas_width, max_canvas_height };
    }

    Portals(size_t max_balls, size_t max_canvas_size)
        : Chamber(max_balls, max_canvas_size)
    //, m_ctx(compute_width_height(max_canvas_size).x, compute_width_height(max_canvas_size).y)
    {
        // m_blue_portal = Portal { { 0.805F, 0.25F }, { 0.805F, 0.25F }, { 0.0F, 0.7F, 1.0F }, 0.15F, 0.05F, deg2rad(30.F), 0.7F };
        // m_orange_portal = Portal { { 0.20F, 0.1F }, { 0.20F, 0.15F }, { 1.0F, 0.5F, 0.0F }, 0.15F, 0.05F, deg2rad(0), 0.5F };

        // m_blue_portal = Portal { { 0.55F, 0.595F }, { 0.0F, 0.7F, 1.0F }, 0.15F, 0.05F, deg2rad(180.F), 0.7F };
        // m_orange_portal = Portal { { 0.45F, 0.1F }, { 1.0F, 0.5F, 0.0F }, 0.15F, 0.05F, deg2rad(0.F), 0.5F };

        m_blue_portal = Portal { { 0.5F + 0.002F, 0.595F }, { 0.0F, 0.7F, 1.0F }, 0.15F, 0.05F, deg2rad(0.F), 0.7F };
        m_orange_portal = Portal { { 0.5F + 0.002F, 0.1F }, { 1.0F, 0.5F, 0.0F }, 0.15F, 0.05F, deg2rad(0.F), 0.5F };

        auto const [max_canvas_width, max_canvas_height] = compute_width_height(max_canvas_size);

#ifdef RENDER_LIVE
        m_blue_portal_texture = render_portal_to_texture(m_ctx, max_canvas_width, max_canvas_height, m_blue_portal);
        m_orange_portal_texture = render_portal_to_texture(m_ctx, max_canvas_width, max_canvas_height, m_orange_portal);
#endif
    }

    void step(size_t num_balls, float delta) override;

    void render(size_t canvas_width, size_t canvas_height) override;

private:
    [[nodiscard]] float pix2pos_x(float x_norm) const
    {
        return x_norm * static_cast<float>(m_canvas_width);
    }
    [[nodiscard]] float pix2pos_y(float y_norm) const
    {
        return static_cast<float>(m_canvas_height) - y_norm * static_cast<float>(m_canvas_width);
    }
    [[nodiscard]] pos2 pix2pos(pos2 pos_norm) const
    {
        return {
            pix2pos_x(pos_norm.x),
            pix2pos_y(pos_norm.y)
        };
    }

    void draw_image(uint32_t const* data, int image_width, int image_height, int x, int y);

    // canvas_ity::canvas m_ctx;
    Portal m_blue_portal;
    Portal m_orange_portal;
#ifdef RENDER_LIVE
    Image m_blue_portal_texture;
    Image m_orange_portal_texture;
#endif
    size_t m_canvas_width {};
    size_t m_canvas_height {};
};

#endif // PORTALS_HPP
