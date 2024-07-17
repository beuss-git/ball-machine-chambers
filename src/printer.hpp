#ifndef PRINTER_HPP
#define PRINTER_HPP

#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <print>
#ifdef __cplusplus
extern "C" {
#endif

#ifdef EMSCRIPTEN
extern void logWasm(char*, size_t);
#else
inline void logWasm(char* fmt, size_t)
{
    std::println("{}", fmt);
}

#endif

void print(char const* fmt, ...);

#ifdef __cplusplus
}
#endif
#endif // PRINTER_HPP
