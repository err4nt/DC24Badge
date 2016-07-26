#include "os_type.h"

#ifndef __RM24C_H__
#define __RM24C_H__

#define EEPROM_ADDRESS 0x50

uint8_t eeprom_read(uint16_t addr);
void eeprom_write(uint16_t addr, uint8_t data);
void eeprom_read_settings(void);

#endif
