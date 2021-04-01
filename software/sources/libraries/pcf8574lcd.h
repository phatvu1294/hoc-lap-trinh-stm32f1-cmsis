#ifndef __PCF8574LCD_H
#define __PCF8574LCD_H

#include "stm32f1xx.h"

/* Địa chỉ PCF8574LCD */
#define PCF8574LCD_ADDR 0x27

/* Nguyên mẫu hàm public */
void pcf8574lcd_command(uint8_t cmd);
void pcf8574lcd_data(uint8_t dt);
void pcf8574lcd_char(char chr);
void pcf8574lcd_string(char * str);
void pcf8574lcd_setPos(uint8_t x, uint8_t y);
void pcf8574lcd_clear(void);
void pcf8574lcd_createChar(uint8_t loc, uint8_t charmap[]);
void pcf8574lcd_init(void);
void pcf8574lcd_backlight(uint8_t enable);

#endif
