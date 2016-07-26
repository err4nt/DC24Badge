#include "os_type.h"

#ifndef __HT16K33_H__
#define __HT16K33_H__

#define HT16K33_ADDR 0x70
#define HT16K33_SYSTEM_SETUP_REG 0x20
#define HT16K33_ROW_INT_REG 0xA0
#define HT16K33_BRIGHTNESS_REG 0xE0
#define HT16K33_DISPLAY_SETUP_REG 0x80
#define HT16K33_DISPLAY_MEM_BEGIN 0x00

#define DISPLAY_SIZE 8
#define DISPLAY_BUFFER_SIZE DISPLAY_SIZE*3

void display_state(uint8_t state);
void display_brightness(uint8_t brightness);
void display_init(void); 
void display_write(char *text);
void display_clear(void);
void display_update(void);
void send_display_buffer(void);

#endif
