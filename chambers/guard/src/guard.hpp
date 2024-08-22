#include <libchamber/chamber.hpp>
#include <libchamber/exports.h>
#ifdef __cplusplus
extern "C" {
#endif
#include <libphysics/physics.h>
#ifdef __cplusplus
}
#endif

struct Target {
    pos2 pos;
    float time_acc;
    float time_to_target; // Calculated on the fly
};
struct Guard {
    pos2 start_pos { 0.5, 0.5 };
    pos2 pos { 0.5, 0.5 };
    // vec2 velocity;
    float radius { 0.035 };

    float cooldown_time = 0.F;

    // targeting
    bool has_target = false;

    Target target;
};

class GuardChamber : public chamber::Chamber {
public:
    ~GuardChamber() override = default;
    GuardChamber(size_t max_balls, size_t max_canvas_size);

    void step(size_t num_balls, float delta) override;

    void render(size_t canvas_width, size_t canvas_height) override;

    pos2 m_save_guard_pos;

    void* save_memory() override { return &m_save_guard_pos; }
    size_t save_size() override { return sizeof(m_save_guard_pos); }
    void save() override { m_save_guard_pos = m_guard.pos; }
    void load() override { m_guard.pos = m_save_guard_pos; }

private:
    enum class BallResultState {
        NOT_FOUND,
        OUT_OF_BOUNDS,
        FOUND,
    };
    struct BallResult {
        ball* ball;
        BallResultState state;
    };

    BallResult find_ball(size_t num_balls, float time_to_target);
    void draw_image(uint32_t const* data, int image_width, int image_height, int x, int y);

private:
    Guard m_guard;
    size_t m_canvas_width;
    size_t m_canvas_height;
};
