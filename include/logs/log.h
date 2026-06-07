#pragma once
#include <cstdio>
#include <cstdarg>
#include <ctime>

enum LogLevel {
    LVL_DEBUG = 0,
    LVL_INFO  = 1,
    LVL_WARN  = 2,
    LVL_ERROR = 3
};

void log_msg(LogLevel level, const char* file, int line, const char* fmt, ...);

#define LOG_DEBUG(...) do { log_msg(LVL_DEBUG, __FILE__, __LINE__, __VA_ARGS__); } while(0)
#define LOG_INFO(...)  do { log_msg(LVL_INFO,  __FILE__, __LINE__, __VA_ARGS__); } while(0)
#define LOG_WARN(...)  do { log_msg(LVL_WARN,  __FILE__, __LINE__, __VA_ARGS__); } while(0)
#define LOG_ERROR(...) do { log_msg(LVL_ERROR, __FILE__, __LINE__, __VA_ARGS__); } while(0)