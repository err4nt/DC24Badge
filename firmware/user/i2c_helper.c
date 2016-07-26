#include "i2c_helper.h"
#include "osapi.h"

uint8_t i2c_send(uint8_t addr, uint8_t *bytes, uint8_t len)
{
    i2c_master_start();
    i2c_master_writeByte((uint8_t)addr<<1);
    if(!i2c_master_checkAck())
    {
        i2c_master_stop();
        os_printf("I2C Error 1\r\n");
        return -1;
    }

    int c = 0;
    for(c = 0; c < len; c++)
    {
        i2c_master_writeByte(bytes[c]);
        if(!i2c_master_checkAck()){
            i2c_master_stop();
            os_printf("I2C Error 2\r\n");
            return c;
        }
    }
}

