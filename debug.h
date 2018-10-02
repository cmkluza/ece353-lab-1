#ifndef DEBUG_H
#define DEBUG_H

#include <stdlib.h>
#include <varargs.h>

/**
 * Logs a debugging statement.
 */
void debug(const char *file, const char *function, int line,
        const char *fmt, ...);

/*
 * Macro that makes using the debug statement easier.
 */
#define PRINT_DEBUG(fmt, ...) do { \
    debug(__FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__); \
} while (0)

#endif
