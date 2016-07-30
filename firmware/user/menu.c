#include "os_type.h"
#include "user_main.h"
#include "ht16k33.h"
#include "osapi.h"
#include "debug.h"
#include "menu.h"

void ICACHE_FLASH_ATTR 
menu_up_handler(void)
{
     menu_data_s *s_data = (menu_data_s *)display_data;

     if(s_data->index == 0)
     {
        s_data->index = MAX_MENU_ITEMS-1;
     }
     else
     {
        s_data->index--;
     }
     s_data->steps = 0;
     system_flags.display_dirty = 1;
}

void ICACHE_FLASH_ATTR
menu_down_handler(void)
{
     menu_data_s *s_data = (menu_data_s *)display_data;
   
     if(s_data->index == MAX_MENU_ITEMS-1 || s_data->items[s_data->index+1][0] == 0)
     {
        s_data->index = 0;
     }
     else
     {
        s_data->index++;
     }
     s_data->steps = 0;
     system_flags.display_dirty = 1;
}

void ICACHE_FLASH_ATTR
menu_right_handler(void)
{
    menu_data_s *s_data = (menu_data_s *)display_data;

    s_data->result = s_data->index;
}

void ICACHE_FLASH_ATTR 
menu_left_handler(void)
{
    menu_data_s *s_data = (menu_data_s *)display_data;

    s_data->result = MENU_CANCEL;
}

void ICACHE_FLASH_ATTR
menu_setup(menu_data_s *data)
{
    debug_print("Menu setup enter\r\n");
    memset(data, 0, sizeof(menu_data_s));
    button_up_handler = &menu_up_handler;
    button_down_handler = &menu_down_handler;
    button_back_handler = &menu_left_handler;
    button_fwd_handler = &menu_right_handler;
    current_display_function = &menu_display;
    system_flags.mode = MODE_MENU;
    data->cancel_steps = 200;
    data->result = MENU_RUNNING;
    debug_print("Menu setup done\r\n");
}

void ICACHE_FLASH_ATTR
menu_add_item(menu_data_s *data, uint8_t index, char *text)
{
    strncpy(data->items[index], text, DISPLAY_SIZE);
}

uint8_t 
menu_display(void *data)
{
    menu_data_s *s_data = (menu_data_s *)data;

    s_data->steps++;

    strncpy(display_buffer, s_data->items[s_data->index], DISPLAY_SIZE);
    system_flags.display_dirty = 1;

    if(s_data->steps > s_data->cancel_steps)
    {
        memset(display_buffer, 0, DISPLAY_SIZE);
        system_flags.display_dirty = 1;
        return 1;
    }

    if(s_data->result == MENU_RUNNING)
    {
        return 0;
    }
    else
    {
        memset(display_buffer, 0, DISPLAY_SIZE);
        system_flags.display_dirty = 1;
        return 1;
    }
}

void ICACHE_FLASH_ATTR
menu_teardown(menu_data_s *data)
{
    debug_print("menu teardown enter\r\n");
    //memset(data, 0, sizeof(menu_data_s));
    button_up_handler = 0;
    button_down_handler = 0;
    button_back_handler = 0;
    button_fwd_handler = 0;
    current_display_function = 0;
    system_flags.mode = MODE_NONE;
    debug_print("menu teardown done\r\n");
}
