#ifndef __USER_MAIN_H__
#define __USER_MAIN_H__

typedef struct {
    uint8_t display_dirty:1;
} system_flags_s;

typedef struct {
    uint8_t random_or_not;
    char target_text[8];
    uint8_t steps;
} sneakers_data_s;

typedef uint8_t (*display_function_f)(void *);

extern system_flags_s system_flags;

#endif
