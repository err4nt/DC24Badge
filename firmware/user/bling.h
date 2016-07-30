#include "user_main.h"

#ifndef __BLING_H__
#define __BLING_H__

typedef struct {
    uint8_t random_or_not;
    char target_text[8];
    uint8_t steps;
    uint8_t end_delay;
} sneakers_data_s;

typedef struct {
    uint8_t steps;
    char text[16];
    uint8_t offset;
} scroll_data_s;

uint8_t display_text_sneakers(void *data);
uint8_t display_text_scroll(void *data);

extern  display_function_f display_handlers[];

void random_bling_select(void);

#endif
