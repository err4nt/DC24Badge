#ifndef __USER_MAIN_H__
#define __USER_MAIN_H__

typedef struct {
    uint8_t display_dirty:1;
} system_flags_s;

extern system_flags_s system_flags;

#endif
