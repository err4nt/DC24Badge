#ifndef __USER_MAIN_H__
#define __USER_MAIN_H__

#define DISPLAY_DATA_SIZE 96

#define MODE_NONE 0
#define MODE_BLING 1
#define MODE_ENTRY 2
#define MODE_DELAY 3
#define MODE_MENU 4

#define MENU_ITEM_BRIGHTNESS 0
#define MENU_ITEM_RESET 1

typedef struct {
    uint8_t display_dirty:1;
    uint8_t mode:7;
    uint8_t delay_count;
} system_flags_s;

typedef struct {
    uint8_t header;
    char nick[17];
    uint8_t brightness;
} settings_s;

typedef uint8_t (*display_function_f)(void *);
typedef void (*button_handler_f)(void);

extern uint8_t display_data[DISPLAY_DATA_SIZE];
extern char display_text[17];
extern system_flags_s system_flags;
extern settings_s settings;
extern display_function_f current_display_function;
extern button_handler_f button_up_handler;
extern button_handler_f button_down_handler;
extern button_handler_f button_back_handler;
extern button_handler_f button_fwd_handler;
extern button_handler_f button_long_up_handler;
extern button_handler_f button_long_down_handler;
extern button_handler_f button_long_back_handler;
extern button_handler_f button_long_fwd_handler;

#endif
