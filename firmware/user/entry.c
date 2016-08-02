#include "os_type.h"
#include "user_main.h"
#include "ht16k33.h"
#include "osapi.h"
#include "entry.h"
    
void ICACHE_FLASH_ATTR 
entry_up_handler(void)
{
    char *buffer = ((entry_data_s *)display_data)->current_text;
    char current = buffer[((entry_data_s *)display_data)->cursor_pos];

    current++;
    if(current > 126)
        current = 32;
    buffer[((entry_data_s *)display_data)->cursor_pos] = current;

    memcpy(display_buffer, &buffer[((entry_data_s *)display_data)->offset], DISPLAY_SIZE);
    update_display_output_buffer();
    send_display_buffer();

    ((entry_data_s *)display_data)->resume_blink = ((entry_data_s *)display_data)->steps + 5;
}

void ICACHE_FLASH_ATTR
entry_down_handler(void)
{
    char *buffer = ((entry_data_s *)display_data)->current_text;
    char current = buffer[((entry_data_s *)display_data)->cursor_pos];

    current--;
    if(current < 32)
        current = 126;
    buffer[((entry_data_s *)display_data)->cursor_pos] = current;

    memcpy(display_buffer, &buffer[((entry_data_s *)display_data)->offset], DISPLAY_SIZE);
    update_display_output_buffer();
    send_display_buffer();

    ((entry_data_s *)display_data)->resume_blink = ((entry_data_s *)display_data)->steps + 5;
}

void ICACHE_FLASH_ATTR 
entry_right_handler(void)
{
    char *buffer = ((entry_data_s *)display_data)->current_text;

    if(((entry_data_s *)display_data)->cursor_pos < (MAX_NICK_SIZE-1))
    {
        ((entry_data_s *)display_data)->cursor_pos++;
        if(((entry_data_s *)display_data)->cursor_pos > (((entry_data_s *)display_data)->offset + (DISPLAY_SIZE-1)))
        {
            ((entry_data_s *)display_data)->offset++;
        }
        memcpy(display_buffer, &buffer[((entry_data_s *)display_data)->offset], DISPLAY_SIZE);
        update_display_output_buffer();
        send_display_buffer();

        ((entry_data_s *)display_data)->resume_blink = ((entry_data_s *)display_data)->steps + 5;
    }
}

void ICACHE_FLASH_ATTR 
entry_left_handler(void)
{
    char *buffer = ((entry_data_s *)display_data)->current_text;

    if(((entry_data_s *)display_data)->cursor_pos > 0)
    {
        ((entry_data_s *)display_data)->cursor_pos--;
        if(((entry_data_s *)display_data)->cursor_pos < ((entry_data_s *)display_data)->offset)
        {
            ((entry_data_s *)display_data)->offset--;
        }
        memcpy(display_buffer, &buffer[((entry_data_s *)display_data)->offset], DISPLAY_SIZE);
        update_display_output_buffer();
        send_display_buffer();

        ((entry_data_s *)display_data)->resume_blink = ((entry_data_s *)display_data)->steps + 5;
    }
}

uint8_t ICACHE_FLASH_ATTR 
entry_display(void *data)
{
    entry_data_s *s_data = (entry_data_s *)data;
    char *buffer = ((entry_data_s *)display_data)->current_text;
    uint8_t cursor_phy_offset = (s_data->cursor_pos)-(s_data->offset);

    s_data->steps++;

    if((s_data->steps % 5) == 0 && (s_data->steps > s_data->resume_blink))
    {
        display_set_raw(cursor_phy_offset, display_get_raw(cursor_phy_offset) ^ 1 << 14);
        send_display_buffer();
    }
    system_flags.display_dirty = 1;

    return 0;
}
    
void ICACHE_FLASH_ATTR
entry_setup(void)
{
    memset(display_data, 0, DISPLAY_DATA_SIZE);
    if(strlen(display_text) > 0)
    {
        strcpy(((entry_data_s *)display_data)->current_text, display_text);
    }
    else
    {
        memset(((entry_data_s *)display_data)->current_text, ' ', MAX_NICK_SIZE);
        ((entry_data_s *)display_data)->current_text[MAX_NICK_SIZE] = 0;
    }
    button_up_handler = &entry_up_handler;
    button_down_handler = &entry_down_handler;
    button_back_handler = &entry_left_handler;
    button_fwd_handler = &entry_right_handler;
    current_display_function = &entry_display;
    memcpy(display_buffer, ((entry_data_s *)display_data)->current_text, DISPLAY_SIZE);
    update_display_output_buffer();
    send_display_buffer();
    system_flags.mode = MODE_ENTRY;
}

void ICACHE_FLASH_ATTR
entry_teardown(void)
{
    memset(display_data, 0, DISPLAY_DATA_SIZE);
    button_up_handler = 0;
    button_down_handler = 0;
    button_back_handler = 0;
    button_fwd_handler = 0;
    current_display_function = 0;
    system_flags.mode = MODE_ENTRY;
}
