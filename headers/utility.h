#ifndef UTILITY_H
#define UTILITY_H

#include <string>
#include <cstdarg>

inline std::string format(const char* format, ...){
    char buffer[256];
    va_list args;
    va_start (args, format);
    vsnprintf (buffer, 255, format, args);
    va_end(args);
    return buffer;
}

#endif //UTILITY_H
