#pragma once
#include <cstdio>
#include <cstdarg>
#include <ctime>

enum LogLevel {
    LOG_DEBUG,
    LOG_INFO, 
    LOG_WARN, 
    LOG_ERROR
};

void log_msg(LogLevel level, const char* file, int line, const char* fmt, ...);

#define LOG_DEBUG(...) log_msg(LOG_DEBUG, __FILE__, __LINE__, __VA_ARGS__);
#define LOG_INFO(...) log_msg(LOG_INFO, __FILE__, __LINE__, __VA_ARGS__);
#define LOG_WARN(...) log_msg(LOG_WARN, __FILE__, __LINE__, __VA_ARGS__);
#define LOG_ERROR(...) log_msg(LOG_ERROR, __FILE__, __LINE__, __VA_ARGS__);