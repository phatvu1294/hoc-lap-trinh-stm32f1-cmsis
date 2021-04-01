#include "delay.h"

/* Biến private */
uint32_t __msTick;

/* Nguyên mẫu hàm private */
void SysTick_Handler(void);

/* Hàm khởi tạo delay */
void delay_init(void)
{
  /* Cấu hình Clock SysTick cho delay ms */
  SysTick_Config(SystemCoreClock / 1000);

  /* Cấu hình mức độ ưu tiên ngắt SysTick */
  NVIC_SetPriority(SysTick_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 0, 0));

  /* Cấu hình DWT cho delay us */
  CoreDebug->DEMCR &= ~CoreDebug_DEMCR_TRCENA_Msk;
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  /* Nếu sử dụng Cortex-M7 uncomment dòng dưới */
  /* DWT->LAR = 0xC5ACCE55; */
  DWT->CYCCNT = 0;
  DWT->CTRL &= ~DWT_CTRL_CYCCNTENA_Msk;
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

/* Hàm lấy giá trị hiện hành của SysTick */
uint32_t millis(void)
{
  return __msTick;
}

/* Hàm delay ms sử dụng SysTick */
void delay_ms(uint32_t ms)
{
  uint32_t startTick = __msTick;
  while (__msTick - startTick < ms);
}

/* Hàm delay us sử dụng DWT */
void delay_us(uint32_t us)
{
  uint32_t startTick = DWT->CYCCNT;
  us *= (SystemCoreClock / 1000000);
  while (DWT->CYCCNT - startTick < us);
}

/* Trình phục vụ ngắt SysTick */
void SysTick_Handler(void)
{
  /* Tăng biến tick */
  __msTick++;
}
