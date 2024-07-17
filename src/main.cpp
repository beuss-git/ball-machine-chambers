#include "main.hpp"
#include "BallMachine.hpp"
#include <cstddef>
#include <cstdint>

// Manual memory management, https://github.com/wingo/walloc is a reasonable
// alternative
enum {
    kPageSize = 65536,
};
extern unsigned char __data_end;  // NOLINT
extern unsigned char __heap_base; // NOLINT

namespace {

BallMachine ball_machine; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

size_t memorySizeBytes()
{
    return __builtin_wasm_memory_size(0) * kPageSize;
}
constexpr size_t roundUp(size_t val, size_t step)
{
    return val + ((step - val % step) % step);
}
}

void init(size_t max_num_balls, size_t max_canvas_size)
{
    unsigned char* alloc_ptr = &__heap_base;
    auto* balls_memory = reinterpret_cast<ball*>(roundUp((size_t)alloc_ptr, alignof(struct ball))); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
    alloc_ptr = reinterpret_cast<unsigned char*>(balls_memory + max_num_balls);                     // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)

    auto* canvas_memory = reinterpret_cast<int32_t*>(roundUp((size_t)alloc_ptr, alignof(int32_t))); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
    alloc_ptr = reinterpret_cast<unsigned char*>(canvas_memory + max_canvas_size);                  // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)

    if ((size_t)alloc_ptr >= memorySizeBytes()) {
        size_t const required_pages = roundUp((size_t)alloc_ptr, kPageSize) / kPageSize;
        __builtin_wasm_memory_grow(0, required_pages);
    }

    ball_machine.init(balls_memory, max_num_balls, canvas_memory, max_canvas_size);
}

void* ballsMemory(void) { return ball_machine.balls_memory(); }
void* canvasMemory(void) { return ball_machine.canvas_memory(); }
void* saveMemory(void) { return nullptr; }
size_t saveSize(void) { return 0; }
void save(void) { }
void load(void) { }

void step(size_t num_balls, float delta)
{
    ball_machine.step(num_balls, delta);
}

void render(size_t canvas_width, size_t canvas_height)
{
    ball_machine.render(canvas_width, canvas_height);
}
