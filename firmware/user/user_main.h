#ifndef __USER_MAIN_H__
#define __USER_MAIN_H__

#define MENU_NONE 0
#define MENU_ENTER_NICK 1

#define MAX_NICK_SIZE 16

typedef struct {
    uint8_t display_dirty:1;
} system_flags_s;

typedef struct {
    uint8_t random_or_not;
    char target_text[8];
    uint8_t steps;
} sneakers_data_s;

typedef struct {
    char current_nick[16];
    uint8_t cursor_pos;
    char under_cursor;
    uint8_t offset;
    uint8_t steps;
} enter_nick_data_s;

typedef struct {
    uint8_t header;
    char nick[16];
    uint8_t brightness;
} settings_s;

typedef uint8_t (*display_function_f)(void *);
typedef void (*button_handler_f)(void);

extern system_flags_s system_flags;

#endif
