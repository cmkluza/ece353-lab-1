#include "debug.h"
#include <stdarg.h>
#include <stdio.h>

void debug(const char *file, const char *function, int line,
           const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    fprintf(stderr, "[DEBUG - %s#%s(l%d)] ", file, function, line);
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");
}
