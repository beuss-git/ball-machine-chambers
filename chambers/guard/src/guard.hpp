#include <libchamber/chamber.hpp>
#include <libchamber/exports.h>
#ifdef __cplusplus
extern "C" {
#endif
#include <libphysics/physics.h>
#ifdef __cplusplus
}
#endif

struct Guard {
    pos2 start_pos { 0.5, 0.5 };
    pos2 pos { 0.5, 0.5 };
    // vec2 velocity;
    float radius { 0.035 };

    float cooldown_time = 0.F;

    // targeting
    bool has_target = false;
    pos2 target_pos;
    float time = 0.F;
};

class GuardChamber : public chamber::Chamber {
public:
    ~GuardChamber() override = default;
    GuardChamber(size_t max_balls, size_t max_canvas_size);

    void step(size_t num_balls, float delta) override;

    void render(size_t canvas_width, size_t canvas_height) override;

private:
    void draw_image(uint32_t const* data, int image_width, int image_height, int x, int y);

private:
    Guard m_guard;
    size_t m_canvas_width;
    size_t m_canvas_height;
};
