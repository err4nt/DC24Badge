#include "os_type.h"
#include "user_main.h"
#include "ht16k33.h"
#include "osapi.h"
#include "entry.h"
    
void 
entry_up_handler(void)
{
    char *buffer = ((entry_data_s *)display_data)->current_text;
    char current = buffer[((entry_data_s *)display_data)->cursor_pos];

    current++;
    if(current > 126)
        current = 32;
    buffer[((entry_data_s *)display_data)->cursor_pos] = current;

    memcpy(display_buffer, &buffer[((entry_data_s *)display_data)->offset], DISPLAY_SIZE);
    send_display_buffer();

    ((entry_data_s *)display_data)->resume_blink = ((entry_data_s *)display_data)->steps + 5;
}

void 
entry_down_handler(void)
{
    char *buffer = ((entry_data_s *)display_data)->current_text;
    char current = buffer[((entry_data_s *)display_data)->cursor_pos];

    current--;
    if(current < 32)
        current = 126;
    buffer[((entry_data_s *)display_data)->cursor_pos] = current;

    memcpy(display_buffer, &buffer[((entry_data_s *)display_data)->offset], DISPLAY_SIZE);
    send_display_buffer();

    ((entry_data_s *)display_data)->resume_blink = ((entry_data_s *)display_data)->steps + 5;
}

void 
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
        send_display_buffer();

        ((entry_data_s *)display_data)->resume_blink = ((entry_data_s *)display_data)->steps + 5;
    }
}

void 
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
        send_display_buffer();

        ((entry_data_s *)display_data)->resume_blink = ((entry_data_s *)display_data)->steps + 5;
    }
}

uint8_t 
entry_display(void *data)
{
    entry_data_s *s_data = (entry_data_s *)data;
    char *buffer = ((entry_data_s *)display_data)->current_text;
    uint8_t cursor_phy_offset = (s_data->cursor_pos)-(s_data->offset);

    s_data->steps++;

    if((s_data->steps % 5) == 0 && (s_data->steps > s_data->resume_blink))
    {
        if(s_data->current_text[s_data->cursor_pos] == ' ')
        {
            if(display_buffer[cursor_phy_offset] == ' ')
                display_buffer[cursor_phy_offset] = '_';
            else
                display_buffer[cursor_phy_offset] = ' ';
        }
        else
        {
            if(display_buffer[cursor_phy_offset] == s_data->current_text[s_data->cursor_pos])
                display_buffer[cursor_phy_offset] = '_';
            else
                display_buffer[cursor_phy_offset] = s_data->current_text[s_data->cursor_pos];
        }
    }
    system_flags.display_dirty = 1;

    return 0;
}
    
void ICACHE_FLASH_ATTR
entry_setup(void)
{
    //Put up the enter nic prompt
    ((entry_data_s *)display_data)->steps = 0;
    ((entry_data_s *)display_data)->cursor_pos = 0;
    ((entry_data_s *)display_data)->offset = 0;
    ((entry_data_s *)display_data)->resume_blink = 0;
    //memset(((entry_data_s *)display_data)->current_text, ' ', MAX_NICK_SIZE);
    memcpy(((entry_data_s *)display_data)->current_text, "ABCDEFGHIJKLMNOP", MAX_NICK_SIZE);
    ((entry_data_s *)display_data)->current_text[MAX_NICK_SIZE] = 0;
    button_up_handler = &entry_up_handler;
    button_down_handler = &entry_down_handler;
    button_back_handler = &entry_left_handler;
    button_fwd_handler = &entry_right_handler;
    current_display_function = &entry_display;
    memcpy(display_buffer, ((entry_data_s *)display_data)->current_text, DISPLAY_SIZE);
}
