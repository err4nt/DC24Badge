#ifndef __I2C_HELPER_H__
#define __I2C_HELPER_H__

#include "os_type.h"

uint8_t i2c_send(uint8_t addr, uint8_t *bytes, uint8_t len);

#endif
