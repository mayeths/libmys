#pragma once

#include <memory>
#include <string>
#include <stdexcept>
#include <stdarg.h>
#include <string.h>

/* https://stackoverflow.com/a/26221725 */
template<typename ... Args>
std::string strformat(const std::string& format, Args ...args)
{
    int size_s = std::snprintf(nullptr, 0, format.c_str(), args...) + 1; // Extra space for '\0'
    if (size_s <= 0)
        throw std::runtime_error( "Error during formatting.");
    size_t size = static_cast<size_t>(size_s);
    char *buffer = new char[size];
    std::snprintf(buffer, size, format.c_str(), args...);
    std::string result(buffer, buffer + size - 1); // We don't want the '\0' inside
    delete[] buffer;
    return result;
}

/* https://stackoverflow.com/a/26221725 */
static inline char *cstrnformat(char *buffer, int bufsize, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    int size_s = vsnprintf(buffer, bufsize, format, args);
    va_end(args);
    if (size_s <= 0 || size_s > bufsize)
        return NULL;
    return buffer;
}

static inline char *cstrformat(char *buffer, const char *format, ...) {
    va_list args;
    va_start(args, format);
    if (buffer == NULL)
        buffer = cstrnformat(NULL, 0, format, args);
    else {
        int bufsize = strlen(buffer);
        buffer = cstrnformat(buffer, bufsize, format, args);
    }
    va_end(args);
    return buffer;
}
