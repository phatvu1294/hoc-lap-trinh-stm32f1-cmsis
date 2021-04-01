#ifndef __DELAY_H
#define __DELAY_H

#include "stm32f1xx.h"

/* Nguyên mẫu hàm public */
void delay_init(void);
uint32_t millis(void);
void delay_ms(uint32_t ms);
void delay_us(uint32_t us);

#endif
