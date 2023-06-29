#ifndef DEBUG_H
#define DEBUG_H

#include <stdio.h>

#define DEBUG 1

#if defined(DEBUG) && DEBUG
#define debug_print(fmt, ...) \
    fprintf(stderr, fmt, ##__VA_ARGS__)
#else
#define debug_print(fmt, ...)
#endif

#endif // DEBUG_H
