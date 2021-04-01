#ifndef __AT24CXX_H
#define __AT24CXX_H

#include "stm32f1xx.h"

/* Địa chỉ các Page */
#define AT24C08_ADDR_PAGE1 0x50
#define AT24C08_ADDR_PAGE2 0x51
#define AT24C08_ADDR_PAGE3 0x52
#define AT24C08_ADDR_PAGE4 0x53 // AT24C08 có 4 page
#define AT24C08_ADDR_PAGE5 0x54
#define AT24C08_ADDR_PAGE6 0x55
#define AT24C08_ADDR_PAGE7 0x56
#define AT24C08_ADDR_PAGE8 0x57 // AT24C16 có 8 page

/* Nguyên mẫu hàm public */
void at24cxx_init(void);
void at24cxx_writeByte(uint8_t address, uint8_t wordAddress, uint8_t data);
void at24cxx_readByte(uint8_t address, uint8_t wordAddress, uint8_t *data);
void at24cxx_writePage(uint8_t address, uint8_t wordAddress, uint8_t *data, uint8_t size);
void at24cxx_readPage(uint8_t address, uint8_t wordAddress, uint8_t *data, uint8_t size);

#endif
