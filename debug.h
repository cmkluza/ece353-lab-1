#ifndef DEBUG_H
#define DEBUG_H

#include <stdlib.h>
#include <stdarg.h>

/**
 * Logs a debugging statement.
 */
void debug(const char *file, const char *function, int line,
        const char *fmt, ...);

/*
 * If DEBUG is defined when compiling (using -DDEBUG on gcc), will print debugging info
 */
#ifdef DEBUG
#define PRINT_DEBUG(fmt, ...) do { \
    debug(__FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__); \
} while (0)
#else
#define PRINT_DEBUG(fmt, ...)
#endif
#endif
