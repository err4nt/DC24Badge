#include "os_type.h"
#include "user_main.h"
#include "ht16k33.h"
#include "osapi.h"
#include "debug.h"
#include "bling.h"

display_function_f display_handlers[] = {&display_text_sneakers};

uint8_t ICACHE_FLASH_ATTR
display_text_sneakers(void *data)
{
    if(data == 0)
    {
        strncpy(((sneakers_data_s *)display_data)->target_text, settings.nick, 8);
        ((sneakers_data_s *)display_data)->steps = 0;
        ((sneakers_data_s *)display_data)->random_or_not = 0xff;
        data = (void *)display_data;
    }
    sneakers_data_s *s_data = (sneakers_data_s *)data;
    s_data->steps++;
    uint8_t scan = 0;

    if((s_data->steps % 10 == 9)){
        uint8_t reveal;
        //find a character to reveal
        os_get_random(&reveal, 1);
        reveal = reveal % 8;
        while(((s_data->random_or_not >> reveal) & 1) == 0)
        {
            //already did that one, pick again
            os_get_random(&reveal, 1);
            reveal = reveal % 8;
        }
        //mark the character for reveal
        s_data->random_or_not &= ~(1 << reveal);
    }

    while(scan < 8)
    {
        if(((s_data->random_or_not >> scan) & 1) == 1)
        {
            //put random char there
            char c;
            os_get_random(&c, 1);
            display_buffer[scan] = c;
        }
        else
        {
            //put real char there
            display_buffer[scan] = s_data->target_text[scan]; 
        }
        scan++;
    }
    system_flags.display_dirty = 1;

    if(s_data->random_or_not == 0)
    {
       //Done
        return 1;
    }
    else
    {
        return 0;
    }
}

void ICACHE_FLASH_ATTR
display_text_build(void *data)
{
}

void ICACHE_FLASH_ATTR
display_text_scroll(void *data)
{
}

void ICACHE_FLASH_ATTR
random_bling_select(void)
{
    //uint8_t display_count = 1;
    //uint8_t random;

    //os_get_random(&random, 1);
    //random %= display_count-1;
    //current_display_function = display_handlers[random];
    current_display_function = &display_text_sneakers;
    current_display_function(0);
}
