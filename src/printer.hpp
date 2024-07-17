#ifndef PRINTER_HPP
#define PRINTER_HPP

#include <cstdarg>
#include <cstddef>
#include <cstdio>
#include <print>
#ifdef __cplusplus
extern "C" {
#endif

#ifdef EMSCRIPTEN
extern void logWasm(char*, size_t);
#else
inline void logWasm(char* fmt, size_t len)
{
    (void)len;
    std::println("{}", fmt);
}
#endif

#ifdef __cplusplus
}
template<typename... Args>
void print(char const* fmt, Args... args)
{
    std::array<char, 256> buffer {};
    int n = snprintf(buffer.data(), sizeof(buffer), fmt, args...);
    if (n < 0) {
        return;
    }
    logWasm(buffer.data(), n);
}
#endif
#endif // PRINTER_HPP
