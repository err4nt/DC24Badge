#include "os_type.h"
#include "user_main.h"
#include "ht16k33.h"
#include "osapi.h"
#include "stdio.h"
#include "numeric.h"
    
void ICACHE_FLASH_ATTR 
numeric_up_handler(void)
{
    numeric_data_s *data = (numeric_data_s *)display_data;

    if(data->current_value < data->max_value)
    {
        data->current_value++;
    } 
}

void ICACHE_FLASH_ATTR
numeric_down_handler(void)
{
    numeric_data_s *data = (numeric_data_s *)display_data;

    if(data->current_value > data->min_value)
    {
        data->current_value--;
    }
}

void ICACHE_FLASH_ATTR
numeric_left_handler(void)
{
    numeric_data_s *data = (numeric_data_s *)display_data;
    
    data->result = NUMERIC_CANCEL;
}

uint8_t ICACHE_FLASH_ATTR 
numeric_display(void *data)
{
    numeric_data_s *s_data = (numeric_data_s *)data;
    char buffer[8] = "        ";

    os_sprintf((char *)buffer, "%d", s_data->current_value);
    memcpy(display_buffer, buffer, 8);
    system_flags.display_dirty = 1;

    if(s_data->result == NUMERIC_CANCEL)
        return 1;

    return 0;
}
    
void ICACHE_FLASH_ATTR
numeric_setup(void)
{
    memset(display_data, 0, DISPLAY_DATA_SIZE);
    ((numeric_data_s *)display_data)->result = 0;
    button_up_handler = &numeric_up_handler;
    button_down_handler = &numeric_down_handler;
    button_back_handler = &numeric_left_handler;
    current_display_function = &numeric_display;
    update_display_output_buffer();
    send_display_buffer();
    system_flags.mode = MODE_NONE;
}

void ICACHE_FLASH_ATTR
numeric_teardown(void)
{
    memset(display_data, 0, DISPLAY_DATA_SIZE);
    button_up_handler = 0;
    button_down_handler = 0;
    current_display_function = 0;
    system_flags.mode = MODE_NONE;
}
