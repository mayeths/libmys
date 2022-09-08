#pragma once

#include <memory>
#include <string>
#include <stdexcept>

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
