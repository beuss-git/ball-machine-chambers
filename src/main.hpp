#include <cstddef>

#ifdef __cplusplus
extern "C" {
#endif
void init(size_t max_num_balls, size_t max_canvas_size);
void* ballsMemory(void);
void* canvasMemory(void);
void* saveMemory(void);
size_t saveSize(void);
void save(void);
void load(void);
void step(size_t num_balls, float delta);
void render(size_t canvas_width, size_t canvas_height);
#ifdef __cplusplus
}
#endif
