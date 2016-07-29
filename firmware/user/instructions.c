#include "os_type.h"
#include "user_main.h"
#include "ht16k33.h"
#include "osapi.h"
#include "instructions.h"

uint8_t ICACHE_FLASH_ATTR
instructions_display(void *data)
{
    instruction_data_s *s_data = (instruction_data_s *)data;

    s_data->steps++;

    if(s_data->steps % (s_data->delay-1) == 0)
    {
        if(s_data->instruction_text[s_data->instruction][0] == 0 || s_data->instruction == 6){
            if(s_data->loop == 1)
            {
                s_data->instruction = 0;
            }
            else
            {
                s_data->end_handler();
                return 1;
            }
        }
        else
        {
            memcpy(display_buffer, s_data->instruction_text[s_data->instruction], 8);
            update_display_output_buffer();
            send_display_buffer();
        }
        s_data->instruction++;
    }

    return 0;
}

void ICACHE_FLASH_ATTR
instruction_set(uint8_t index, char *text)
{
    instruction_data_s *s_data = (instruction_data_s *)display_data;

    strncpy(s_data->instruction_text[index], text, 8);
}

void ICACHE_FLASH_ATTR
instructions_set_end_handler(instruction_end_handler_f handler)
{
    instruction_data_s *s_data = (instruction_data_s *)display_data;

    s_data->end_handler = handler;
}

void ICACHE_FLASH_ATTR
instructions_setup(uint8_t delay)
{
    instruction_data_s *s_data = (instruction_data_s *)display_data;

    memset(s_data, 0, sizeof(instruction_data_s));
    s_data->delay = delay;
    current_display_function = &instructions_display;

    memcpy(display_buffer, s_data->instruction_text[0], 8);
}

void ICACHE_FLASH_ATTR
instructions_teardown(void)
{
    memset(display_data, 0, sizeof(instruction_data_s));
    current_display_function = 0;
}
