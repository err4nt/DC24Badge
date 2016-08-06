#include "ets_sys.h"
#include "osapi.h"
#include "gpio.h"
#include "os_type.h"
#include "mem.h"
#include "user_config.h"
#include "user_interface.h"
#include "driver/key.h"
#include "driver/uart.h"
#include "driver/i2c_master.h"
#include "ht16k33.h"
#include "entry.h"
#include "numeric.h"
#include "debug.h"
#include "bling.h"
#include "menu.h"
#include "user_main.h"

system_flags_s system_flags;
settings_s settings;

uint8_t display_data[DISPLAY_DATA_SIZE];
char display_text[33];
char heard_nicks[8][17];
uint8_t heard_nick_index;
display_function_f current_display_function = 0;

//Button handlers
button_handler_f button_up_handler = 0;
button_handler_f button_down_handler = 0;
button_handler_f button_back_handler = 0;
button_handler_f button_fwd_handler = 0;
button_handler_f button_long_up_handler = 0;
button_handler_f button_long_down_handler = 0;
button_handler_f button_long_back_handler = 0;
button_handler_f button_long_fwd_handler = 0;

#define user_procTaskPrio        1 
#define user_procTaskQueueLen    1
os_event_t    user_procTaskQueue[user_procTaskQueueLen];
static volatile os_timer_t deauth_timer;

// Channel to perform deauth
uint8_t channel = 1;

// Access point MAC to deauth
uint8_t ap[6] = {0x00,0x01,0x02,0x03,0x04,0x05};

// Client MAC to deauth
uint8_t client[6] = {0x06,0x07,0x08,0x09,0x0A,0x0B};

// Sequence number of a packet from AP to client
uint16_t seq_n = 0;

// Packet buffer
uint8_t packet_buffer[64];

LOCAL struct keys_param keys;
LOCAL struct single_key_param *single_key[4];

int button_event;

/* ==============================================
 * Promiscous callback structures, see ESP manual
 * ============================================== */
 
struct RxControl {
    signed rssi:8;
    unsigned rate:4;
    unsigned is_group:1;
    unsigned:1;
    unsigned sig_mode:2;
    unsigned legacy_length:12;
    unsigned damatch0:1;
    unsigned damatch1:1;
    unsigned bssidmatch0:1;
    unsigned bssidmatch1:1;
    unsigned MCS:7;
    unsigned CWB:1;
    unsigned HT_length:16;
    unsigned Smoothing:1;
    unsigned Not_Sounding:1;
    unsigned:1;
    unsigned Aggregation:1;
    unsigned STBC:2;
    unsigned FEC_CODING:1;
    unsigned SGI:1;
    unsigned rxend_state:8;
    unsigned ampdu_cnt:8;
    unsigned channel:4;
    unsigned:12;
};
 
struct LenSeq {
    uint16_t length;
    uint16_t seq;
    uint8_t  address3[6];
};

struct sniffer_buf {
    struct RxControl rx_ctrl;
    uint8_t buf[36];
    uint16_t cnt;
    struct LenSeq lenseq[1];
};

struct sniffer_buf2{
    struct RxControl rx_ctrl;
    uint8_t buf[112];
    uint16_t cnt;
    uint16_t len;
};

/* Creates a deauth packet.
 * 
 * buf - reference to the data array to write packet to;
 * client - MAC address of the client;
 * ap - MAC address of the acces point;
 * seq - sequence number of 802.11 packet;
 * 
 * Returns: size of the packet
 */
uint16_t deauth_packet(uint8_t *buf, uint8_t *client, uint8_t *ap, uint16_t seq)
{
    int i=0;
    
    // Type: deauth
    buf[0] = 0xC0;
    buf[1] = 0x00;
    // Duration 0 msec, will be re-written by ESP
    buf[2] = 0x00;
    buf[3] = 0x00;
    // Destination
    for (i=0; i<6; i++) buf[i+4] = client[i];
    // Sender
    for (i=0; i<6; i++) buf[i+10] = ap[i];
    for (i=0; i<6; i++) buf[i+16] = ap[i];
    // Seq_n
    buf[22] = seq % 0xFF;
    buf[23] = seq / 0xFF;
    // Deauth reason
    buf[24] = 1;
    buf[25] = 0;
    return 26;
}

uint16_t badge_announce_packet(uint8_t *buf)
{
    //uint8_t nick_end = 5+(strlen(settings.nick)-2); //Two header bytes, length byte, plus length of nick
    //buf[0] = 0x80;
    //buf[1] = 0xAD;
    //buf[2] = 0x00;
    //buf[3] = 0x00;
    //buf[4] = strlen(settings.nick)-2;
    //memcpy(&buf[5], settings.nick, strlen(settings.nick)-2);
    //return nick_end;
    buf[0] = 0x80;
    buf[1] = 0x00;
    buf[2] = 0x00;
    buf[3] = 0x00;
    buf[4] = 0xBE;
    buf[5] = 0xEF;
    buf[6] = 0xDE;
    buf[7] = 0xAD;
    buf[8] = 0xFF;
    buf[9] = 0xFF;
    buf[10] = 0xDE;
    buf[11] = 0xAD;
    buf[12] = 0xBE;
    buf[13] = 0xEF;
    buf[14] = 0xFF;
    buf[15] = 0xFF;
    buf[16] = 0xBE;
    buf[17] = 0xEE;
    buf[18] = 0xEE;
    buf[19] = 0xEF;
    buf[20] = 0xFF;
    buf[21] = 0xFF;
    buf[22] = 0xC0;
    buf[23] = 0x6C;
    buf[24] = 0x83;
    buf[25] = 0x51;
    buf[26] = 0xF7;
    buf[27] = 0x8F;
    buf[28] = 0x0F;
    buf[29] = 0x00;
    buf[30] = 0x00;
    buf[31] = 0x00;
    buf[32] = 0x64;
    buf[33] = 0x00;
    buf[34] = 0x01;
    buf[35] = 0x04;
    strncpy(&buf[36], settings.nick, 17);
    buf[56] = 0x04;

    return 56; 
}

/*
 * Nick cache management
 */
void ICACHE_FLASH_ATTR
add_heard_nick(char *nick)
{
    uint8_t scan = 0;
    
    for(scan = 0; scan < 8; scan++)
    {
        if(strcmp(heard_nicks[scan], nick) == 0)
        {
            //Nick isin't new
            return;
        }
    }    

    heard_nick_index++;
    if(heard_nick_index == 8)
        heard_nick_index = 0;
    strncpy(heard_nicks[heard_nick_index], nick, 17);
}

/*
 * Send badge announce packet
 */
void badge_announce(void)
{
    uint16_t size = badge_announce_packet(packet_buffer);
    uint8_t result = wifi_send_pkt_freedom(packet_buffer, size, 0);
}

/* Sends deauth packets. */
void deauth(void *arg)
{
    os_printf("\nSending deauth seq_n = %d ...\n", seq_n/0x10);
    // Sequence number is increased by 16, see 802.11
    uint16_t size = deauth_packet(packet_buffer, client, ap, seq_n+0x10);
    wifi_send_pkt_freedom(packet_buffer, size, 0);
}

/* Listens communication between AP and client */
static void 
promisc_cb(uint8_t *buf, uint16_t len)
{
    if (len == 12){
        struct RxControl *sniffer = (struct RxControl*) buf;
    } else if (len == 128) {
        struct sniffer_buf2 *sniffer = (struct sniffer_buf2*) buf;
        if(sniffer->buf[4] == 0xBE && sniffer->buf[5] == 0xEF && sniffer->buf[6] == 0xDE && sniffer->buf[7] == 0xAD)
        {
            char nick_buffer[17];
            strncpy(nick_buffer, &sniffer->buf[36], 17);
            add_heard_nick(nick_buffer);
            system_flags.have_heard_nick = 1;
            debug_print("Got badge packet: %s\r\n", nick_buffer);
        }
    } else {
/*        struct sniffer_buf *sniffer = (struct sniffer_buf*) buf;
        int i=0;
        // Check MACs
        for (i=0; i<6; i++) if (sniffer->buf[i+4] != client[i]) return;
        for (i=0; i<6; i++) if (sniffer->buf[i+10] != ap[i]) return;
        // Update sequence number
        seq_n = sniffer->buf[23] * 0xFF + sniffer->buf[22];*/
    }
}

void ICACHE_FLASH_ATTR
sniffer_system_init_done(void)
{
    // Set up promiscuous callback
    wifi_set_channel(settings.channel);
    wifi_promiscuous_enable(0);
    wifi_set_promiscuous_rx_cb(promisc_cb);
    wifi_promiscuous_enable(1);
}

/*
 * Button Handler functions
 */

void ICACHE_FLASH_ATTR
button_up_long_press(void)
{
    debug_print("Up long press\r\n");
    if(button_long_up_handler != 0)
        button_long_up_handler();
}

void ICACHE_FLASH_ATTR
button_up_short_press(void)
{
    debug_print("Up short press\r\n");
    if(button_up_handler != 0)
        button_up_handler();
}

void ICACHE_FLASH_ATTR
button_down_long_press(void)
{
    debug_print("Down long press\r\n");
    if(button_long_down_handler != 0)
        button_long_down_handler();
}

void ICACHE_FLASH_ATTR
button_down_short_press(void)
{
    debug_print("Down short press\r\n");
    if(button_down_handler != 0)
        button_down_handler();
}

void ICACHE_FLASH_ATTR
button_left_long_press(void)
{
    debug_print("Left long press\r\n");
    if(button_long_back_handler != 0)
        button_long_back_handler();
}

void ICACHE_FLASH_ATTR
button_left_short_press(void)
{
    debug_print("Left short press\r\n");
    if(button_back_handler != 0)
        button_back_handler();
}

void ICACHE_FLASH_ATTR
button_right_long_press(void)
{
    debug_print("Right long press\r\n");
    if(button_long_fwd_handler != 0)
        button_long_fwd_handler();
}

void ICACHE_FLASH_ATTR
button_right_short_press(void)
{
    debug_print("Right short press\r\n");
    debug_print("system_flags.mode = %d\r\n", system_flags.mode);
    if(system_flags.mode == MODE_BLING)
    {
        system_flags.mode = MODE_MENU;
        current_display_function = 0;
    }
    else
    {
        if(button_fwd_handler != 0)
            button_fwd_handler();
    }
}

/* 
 * Initialization functions
 */

void ICACHE_FLASH_ATTR
buttons_init()
{
    debug_print("Enter buttons_init\r\n");

    single_key[0] = key_init_single(BUTTON_UP_IO_NUM, BUTTON_UP_IO_MUX, BUTTON_UP_IO_FUNC, button_up_long_press, button_up_short_press);
    single_key[1] = key_init_single(BUTTON_DOWN_IO_NUM, BUTTON_DOWN_IO_MUX, BUTTON_DOWN_IO_FUNC, button_down_long_press, button_down_short_press);
    single_key[2] = key_init_single(BUTTON_LEFT_IO_NUM, BUTTON_LEFT_IO_MUX, BUTTON_LEFT_IO_FUNC, button_left_long_press, button_left_short_press);
    single_key[3] = key_init_single(BUTTON_RIGHT_IO_NUM, BUTTON_RIGHT_IO_MUX, BUTTON_RIGHT_IO_FUNC, button_right_long_press, button_right_short_press);

    keys.key_num = 4;
    keys.single_key = single_key;
    key_init(&keys);
    debug_print("Leave buttons_init\r\n");
}

void ICACHE_FLASH_ATTR
i2c_init()
{
    debug_print("Enter i2c_init\r\n");
    i2c_master_gpio_init();
    i2c_master_init();
    debug_print("Leave i2c_init\r\n");
}

void ICACHE_FLASH_ATTR
nick_entry_done_handler(void)
{
    //Activated on long press of right button
    entry_data_s *s_data = (entry_data_s *)display_data;

    uint8_t index = 0;
    char c = s_data->current_text[index];
    while(c != ' ' && c != 0) 
    {
        index++;
        c = s_data->current_text[index];
    }
    if(c == ' ')
        s_data->current_text[index] = 0;

    settings.header = 0xDE;
    settings.channel = 1;
    settings.mode = 0;

    strncpy(settings.nick, s_data->current_text, 17);

    eeprom_write_block(&settings, 0, sizeof(settings_s));

    memcpy(display_buffer, "SAVED   ", 8);
    update_display_output_buffer();
    send_display_buffer();
    entry_teardown();
    button_long_fwd_handler = 0;
    system_flags.mode = MODE_BLING;
}

void ICACHE_FLASH_ATTR
enter_nick_instruction_end_handler(void)
{
    instructions_teardown();
    memset(display_text, 0, 17);
    entry_setup();
    button_long_fwd_handler = &nick_entry_done_handler;
}

void ICACHE_FLASH_ATTR
led_msg_back_handler(void)
{
    system_flags.mode = MODE_BLING;
    current_display_function = 0;
}

void ICACHE_FLASH_ATTR
led_msg_entry_done_handler(void)
{
    memcpy(display_text, ((entry_data_s *)display_data)->current_text, 17);
    entry_teardown();
    current_display_function = &display_text_scroll;
    display_text_scroll(0);
    button_back_handler = &led_msg_back_handler;
    system_flags.mode = MODE_LED_MSG;
}

void ICACHE_FLASH_ATTR
brightness_numeric_done_handler(void)
{
    numeric_data_s *data = (numeric_data_s *)display_data;
    settings.brightness = data->current_value;
    display_brightness(settings.brightness);
    eeprom_write_block(&settings, 0, sizeof(settings_s));       
    system_flags.mode = MODE_BLING;
    current_display_function = 0;
    button_long_fwd_handler = 0;
}

void ICACHE_FLASH_ATTR
channel_numeric_done_handler(void)
{
    numeric_data_s *data = (numeric_data_s *)display_data;
    settings.channel = data->current_value;
    wifi_set_channel(settings.channel);
    eeprom_write_block(&settings, 0, sizeof(settings_s));       
    system_flags.mode = MODE_BLING;
    current_display_function = 0;
    button_long_fwd_handler = 0;
}

void loop(os_event_t *events)
{
    uint8_t result;

    if(system_flags.display_dirty == 1 && system_flags.mode != MODE_ENTRY)
    {
        update_display_output_buffer();
        send_display_buffer();
        system_flags.display_dirty = 0;
    }

    if(current_display_function != 0)
    {
        if(current_display_function((void *)display_data) == 1)
        {
            switch(system_flags.mode)
            {
                case MODE_BLING:
                    random_text_select();
                    random_bling_select();
                    badge_announce();
                    break;
                case MODE_MENU:
                    result = ((menu_data_s *)display_data)->result;
                    menu_teardown((menu_data_s *)display_data);
                    switch(result)
                    {
                        case MENU_ITEM_RESET:
                            eeprom_write_byte(0x0000, 0x00); //Clear the settings header
                            system_restart();
                            break;
                        case MENU_ITEM_BRIGHT:
                            numeric_setup();
                            ((numeric_data_s *)display_data)->current_value = settings.brightness;
                            ((numeric_data_s *)display_data)->max_value = 15;
                            ((numeric_data_s *)display_data)->min_value = 1;
                            button_long_fwd_handler = &brightness_numeric_done_handler;
                            break;
                        case MENU_ITEM_CHANNEL:
                            numeric_setup();
                            ((numeric_data_s *)display_data)->current_value = settings.channel;
                            ((numeric_data_s *)display_data)->max_value = 15;
                            ((numeric_data_s *)display_data)->min_value = 1;
                            button_long_fwd_handler = &channel_numeric_done_handler;
                            break;
                        case MENU_ITEM_NICK:
                            strcpy(display_text, settings.nick);
                            entry_setup();
                            button_long_fwd_handler = &nick_entry_done_handler;
                            break;
                        case MENU_ITEM_LED_MSG:
                            display_text[0] = 0;
                            entry_setup();
                            button_long_fwd_handler = &led_msg_entry_done_handler;
                            break;
                        case MENU_CANCEL:
                            system_flags.mode = MODE_BLING;
                            break;
                    }
                    break;
                case MODE_NONE:
                    system_flags.mode = MODE_BLING;
                    break;
                case MODE_LED_MSG:
                    current_display_function(0);
                    break;
            }
        }
    }
    else
    {
        debug_print("No display function, Mode: %d\r\n", system_flags.mode);
        switch(system_flags.mode)
        {
            case MODE_BLING:
                debug_print("Select display function\r\n");
                random_text_select();
                random_bling_select();
                break;
            case MODE_MENU:
                menu_setup((menu_data_s *)display_data);
                menu_add_item((menu_data_s *)display_data, MENU_ITEM_BRIGHT, "BRIGHT");
                menu_add_item((menu_data_s *)display_data, MENU_ITEM_NICK, "NICK");
                menu_add_item((menu_data_s *)display_data, MENU_ITEM_CHANNEL, "CHANNEL");
                //menu_add_item((menu_data_s *)display_data, MENU_ITEM_WIFI_MSG, "WIFI MSG");
                menu_add_item((menu_data_s *)display_data, MENU_ITEM_LED_MSG, "LED MSG");
                menu_add_item((menu_data_s *)display_data, MENU_ITEM_RESET, "RESET");
                break;
        }
    }

    os_delay_us(50000);
    system_os_post(user_procTaskPrio, 0, 0 );
}


void ICACHE_FLASH_ATTR
user_init()
{
    uart_init(115200, 115200);
    os_printf("\n\nSDK version:%s\n", system_get_sdk_version());
    os_printf("\n\nWelcome to DEFCON 24\n");
    os_printf("\n\nThis badge was a labor of love and coffee by Errant Librarian (hardware and firmware) and nodoze (assembly and bad ideas)\n");
    os_printf("\n\nvoidptr.org/dc24\n");
 
    // Promiscuous works only with station mode
    wifi_set_opmode(STATION_MODE);

    gpio_init();

    i2c_init();
    buttons_init();
    display_init();
    display_clear();
    os_printf("Display setup done\r\n");

    eeprom_read_block(&settings, 0, sizeof(settings_s));
    if(settings.header == 0xDE)
    {
        debug_print("EEPROM settings loaded\r\n");
        if(settings.brightness > 15)
            settings.brightness = 5;
        if(settings.channel > 14)
            settings.channel = 1;
        system_flags.mode = MODE_BLING;
        display_brightness(settings.brightness);
    }
    else
    {
        debug_print("EEPROM settings invalid\r\n");
        instructions_setup(30);
        instruction_set(0, "ENTER");
        instruction_set(1, "NICK");
        instruction_set(2, "HOLD");
        instruction_set(3, "RIGHT");
        instruction_set(4, "TO SAVE");
        instructions_set_end_handler(&enter_nick_instruction_end_handler);
        settings.brightness = 5;
        settings.channel = 1;
    }

    system_os_task(loop, user_procTaskPrio, user_procTaskQueue, user_procTaskQueueLen);
    system_os_post(user_procTaskPrio, 0, 0);

    // Set timer for deauth
    // os_timer_disarm(&deauth_timer);
    // os_timer_setfn(&deauth_timer, (os_timer_func_t *) deauth, NULL);
    // os_timer_arm(&deauth_timer, CHANNEL_HOP_INTERVAL, 1);
    
    // Continue to 'sniffer_system_init_done'
    system_init_done_cb(sniffer_system_init_done);
}
