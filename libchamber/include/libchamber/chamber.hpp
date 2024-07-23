#ifndef CHAMBER_HPP
#define CHAMBER_HPP

#include <cstddef>
#ifdef __cplusplus
extern "C" {
#endif
#include <libphysics/physics.h>
#ifdef __cplusplus
}
#endif
#include <memory>
#include <vector>

namespace chamber {

class Chamber {
public:
    virtual ~Chamber() = default;
    Chamber(Chamber const&) = default;
    Chamber(Chamber&&) = delete;
    Chamber& operator=(Chamber const&) = default;
    Chamber& operator=(Chamber&&) = delete;

    Chamber(size_t max_balls, size_t max_canvas_size)
    {
        m_balls = std::vector<ball>(max_balls);
        m_canvas = std::vector<uint32_t>(max_canvas_size);
    }

    [[nodiscard]] void* balls_memory() { return m_balls.data(); }
    [[nodiscard]] void* canvas_memory() { return m_canvas.data(); }

    virtual void* save_memory() { return nullptr; }
    virtual size_t save_size() { return 0; }
    virtual void save() { }
    virtual void load() { }

    // These are the only required methods for chamber implementation (?)
    virtual void step(size_t num_balls, float delta) = 0;
    virtual void render(size_t canvas_width, size_t canvas_height) = 0;

protected:
    std::vector<ball> m_balls;
    std::vector<uint32_t> m_canvas;
};

extern std::unique_ptr<Chamber> g_chamber;

template<typename T>
void init(size_t num_balls, size_t canvas_size)
{
    g_chamber = std::make_unique<T>(num_balls, canvas_size);
}
};

#endif // CHAMBER_HPP
