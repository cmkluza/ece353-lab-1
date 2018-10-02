#include "debug.h"

#ifdef DEBUG // if DEBUG is defined at compile time, log debug statements
void debug(const char *file, const char *function, int line,
        const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    fprintf(stderr, "[DEBUG - %s#%s(l%d)] ", file, function, line);
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");
}
#else // otherwise, do nothing
void debug(const char *file, const char *function, int line,
       const char *fmt, ...) {}
#endif
