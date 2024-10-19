#include "libchamber/chamber.hpp"
#include "exports.h"

namespace chamber {
std::unique_ptr<Chamber> g_chamber = nullptr;
}

void* ballsMemory(void) { return chamber::g_chamber->balls_memory(); }
void* canvasMemory(void) { return chamber::g_chamber->canvas_memory(); }
void* saveMemory(void) { return chamber::g_chamber->save_memory(); }
size_t saveSize(void) { return chamber::g_chamber->save_size(); }
void save(void) { chamber::g_chamber->save(); }
void load(void) { chamber::g_chamber->load(); }
void step(size_t num_balls, float delta) { chamber::g_chamber->step(num_balls, delta); }
void render(size_t canvas_width, size_t canvas_height) { chamber::g_chamber->render(canvas_width, canvas_height); }
