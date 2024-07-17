#include "printer.hpp"
#include <cstring>

void print(char const* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    char buffer[256];
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    logWasm(reinterpret_cast<char*>(buffer), strlen(buffer));
}
