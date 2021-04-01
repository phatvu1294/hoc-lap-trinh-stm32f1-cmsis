#ifndef __USART1_H
#define __USART1_H

#include "stm32f1xx.h"

/* Nguyên mẫu hàm public */
void usart1_init(void);
void usart1_putChar(uint8_t c);
void usart1_putString(uint8_t *s);
uint8_t usart1_getChar(void);

#endif
