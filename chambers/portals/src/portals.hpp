#ifndef PORTALS_HPP
#define PORTALS_HPP
#include "portal.hpp"
#include "utils/image.hpp"
#include <libchamber/chamber.hpp>
#include <libchamber/print.hpp>

class Portals : public chamber::Chamber {
public:
    Portals(Portals const&) = delete;
    Portals(Portals&&) = delete;
    Portals& operator=(Portals const&) = delete;
    Portals& operator=(Portals&&) = delete;
    ~Portals() override = default;

    Portals(size_t max_balls, size_t max_canvas_size)
        : Chamber(max_balls, max_canvas_size)
        , m_ctx(300, 209)
    {
        m_blue_portal = Portal { { 0.805F, 0.3F }, { 0.805F, 0.0F }, { 0.0F, 0.7F, 1.0F }, 0.15F, 0.05F, deg2rad(45.F) };
        m_orange_portal = Portal { { 0.2F, 0.095F }, { 1.F - 0.2F, 0.095F }, { 1.0F, 0.5F, 0.0F }, 0.15F, 0.05F, deg2rad(0) };

        m_blue_portal_texture = render_portal_to_texture(m_ctx, 300, 209, m_blue_portal);
        m_orange_portal_texture = render_portal_to_texture(m_ctx, 300, 209, m_orange_portal);
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

    void draw_portal(canvas_ity::canvas& context, float cx, float cy, float rx, float ry, Color const& portal_color, float rotation);
    Image render_portal_to_texture(canvas_ity::canvas& context, size_t canvas_width, size_t canvas_height, Portal const& portal);

    canvas_ity::canvas m_ctx;
    Portal m_blue_portal;
    Portal m_orange_portal;
    Image m_blue_portal_texture;
    Image m_orange_portal_texture;
    size_t m_canvas_width {};
    size_t m_canvas_height {};
};

#endif // PORTALS_HPP
