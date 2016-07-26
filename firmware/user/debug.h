#include "osapi.h"

#define debug_print(fmt, ...) \
    do { if (DEBUG) os_printf(fmt, ##__VA_ARGS__); } while (0)
