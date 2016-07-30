#ifndef __MENU_H__
#define __MENU_H__

#define MAX_MENU_ITEMS 8

#define MENU_RUNNING MAX_MENU_ITEMS+1
#define MENU_CANCEL MAX_MENU_ITEMS+2

typedef struct
{
    char items[MAX_MENU_ITEMS][DISPLAY_SIZE];
    uint8_t index;
    uint8_t result;
    uint8_t steps;
    uint8_t cancel_steps;
} menu_data_s;

void menu_setup(menu_data_s *data);
void menu_add_item(menu_data_s *data, uint8_t index, char *text);
uint8_t menu_display(void *data);
void menu_teardown(menu_data_s *data);

#endif
