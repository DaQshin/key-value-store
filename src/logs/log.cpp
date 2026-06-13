#include "logs/log.h"
#include <cstdarg>
#include <cstdio>

static const char* level_to_string(LogLevel level) {
    switch(level) {
        case LVL_DEBUG: return "DEBUG";
        case LVL_INFO:  return "INFO";
        case LVL_WARN:  return "WARN";
        case LVL_ERROR: return "ERROR";
        default:        return "UNKNOWN";
    }
}

void log_msg(LogLevel level,
             const char* file,
             int line,
             const char* fmt,
             ...) {

    std::fprintf(stderr,
                 "[%s] %s:%d: ",
                 level_to_string(level),
                 file,
                 line);

    va_list args;
    va_start(args, fmt);
    std::vfprintf(stderr, fmt, args);
    va_end(args);

    std::fprintf(stderr, "\n");
}