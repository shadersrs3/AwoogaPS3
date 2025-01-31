#ifndef _COMMON_LOGGER_H
#define _COMMON_LOGGER_H

#include <cstdio>

void logOutput(FILE *out, const char *code, const char *fmt, ...);

#define LOG_INFO(...) logOutput(stdout, "I", __VA_ARGS__)
#define LOG_WARN(...) logOutput(stdout, "W", __VA_ARGS__)
#define LOG_ERROR(...) logOutput(stdout, "E", __VA_ARGS__)

#endif