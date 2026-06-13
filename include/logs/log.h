#pragma once

#include <cstdarg>
#include <cstdio>

enum LogLevel {
    LVL_DEBUG = 0,
    LVL_INFO  = 1,
    LVL_WARN  = 2,
    LVL_ERROR = 3,
};

void log_msg(LogLevel level,
             const char* file,
             int line,
             const char* fmt,
             ...);

#ifndef LOG_LEVEL
#define LOG_LEVEL LVL_INFO
#endif

#if LOG_LEVEL <= LVL_DEBUG
#define LOG_DEBUG(...) \
    do { log_msg(LVL_DEBUG, __FILE__, __LINE__, __VA_ARGS__); } while(0)
#else
#define LOG_DEBUG(...) do {} while(0)
#endif

#if LOG_LEVEL <= LVL_INFO
#define LOG_INFO(...) \
    do { log_msg(LVL_INFO, __FILE__, __LINE__, __VA_ARGS__); } while(0)
#else
#define LOG_INFO(...) do {} while(0)
#endif

#if LOG_LEVEL <= LVL_WARN
#define LOG_WARN(...) \
    do { log_msg(LVL_WARN, __FILE__, __LINE__, __VA_ARGS__); } while(0)
#else
#define LOG_WARN(...) do {} while(0)
#endif

#if LOG_LEVEL <= LVL_ERROR
#define LOG_ERROR(...) \
    do { log_msg(LVL_ERROR, __FILE__, __LINE__, __VA_ARGS__); } while(0)
#else
#define LOG_ERROR(...) do {} while(0)
#endif