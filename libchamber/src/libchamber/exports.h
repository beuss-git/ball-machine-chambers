#ifndef EXPORTS_HPP
#define EXPORTS_HPP
#include <cstddef>
#include <cstdint>

#ifdef __cplusplus
extern "C" {
#endif
void* ballsMemory(void);
void* canvasMemory(void);
void* saveMemory(void);
size_t saveSize(void);
void save(void);
void load(void);
void step(size_t num_balls, float delta);
void render(size_t canvas_width, size_t canvas_height);

/*NOTE: WASI API bypass when using std::vector*/
typedef uint16_t __wasi_errno_t;
typedef uint32_t __wasi_fd_t;
__wasi_errno_t __wasi_fd_close(__wasi_fd_t)
{
    __builtin_trap();
}

typedef int64_t __wasi_filedelta_t;
typedef uint8_t __wasi_whence_t;
typedef uint64_t __wasi_filesize_t;

__wasi_errno_t __wasi_fd_seek(
    __wasi_fd_t,
    __wasi_filedelta_t,
    __wasi_whence_t,
    __wasi_filesize_t*)
{
    __builtin_trap();
}

typedef __SIZE_TYPE__ __wasi_size_t;
typedef struct __wasi_ciovec_t __wasi_ciovec_t;

__wasi_errno_t __wasi_fd_write(
    __wasi_fd_t,
    __wasi_ciovec_t const*,
    size_t,
    __wasi_size_t*)
{
    __builtin_trap();
}

__wasi_errno_t __wasi_environ_get(
    uint8_t**,
    uint8_t*)
{
    __builtin_trap();
}

__wasi_errno_t __wasi_environ_sizes_get(
    __wasi_size_t*,
    __wasi_size_t*)
{
    __builtin_trap();
}

#ifdef __cplusplus
}
#endif
#endif // EXPORTS_HPP
