#include "main.hpp"
#include "BallMachine.hpp"
#include "printer.hpp"

std::unique_ptr<BallMachine> ball_machine {}; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

void init(size_t max_num_balls, size_t max_canvas_size)
{
    print("Initializing Ball Machine with %zu balls and canvas size %zu\n", max_num_balls, max_canvas_size);

    // ball_machine.init(max_num_balls, max_canvas_size);
    ball_machine = std::make_unique<BallMachine>(max_num_balls, max_canvas_size);
}

void* ballsMemory(void) { return ball_machine->balls_memory(); }
void* canvasMemory(void) { return ball_machine->canvas_memory(); }
void* saveMemory(void) { return nullptr; }
size_t saveSize(void) { return 0; }
void save(void) { }
void load(void) { }

void step(size_t num_balls, float delta)
{
    ball_machine->step(num_balls, delta);
}

void render(size_t canvas_width, size_t canvas_height)
{
    ball_machine->render(canvas_width, canvas_height);
}
