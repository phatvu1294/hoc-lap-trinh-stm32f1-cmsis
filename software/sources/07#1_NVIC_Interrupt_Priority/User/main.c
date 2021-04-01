#include "stm32f1xx.h"
#include "startup.h"
#include "delay.h"

void EXTI0_IRQHandler(void);
void EXTI1_IRQHandler(void);
void EXTI2_IRQHandler(void);

int main(void)
{
  /* Thêm chương trình chính tại đây */

  /* ___LIB___ */

  /* Khởi tạo hệ thống */
  startup_init();

  /* Khởi tạo delay */
  delay_init();

  /* ___GPIO___ */

  /* Bật Clock PortB */
  RCC->APB2ENR |= RCC_APB2ENR_IOPBEN; // 1: Enable

  /* Reset PB5, PB6, PB7 */
  GPIOB->BSRR |= GPIO_BSRR_BR5 | GPIO_BSRR_BR6 | GPIO_BSRR_BR7;

  /* Cấu hình PB5, PB6, PB7 */
  GPIOB->CRL &= ~GPIO_CRL_MODE5 & ~GPIO_CRL_MODE6 & ~GPIO_CRL_MODE7; // Clear
  GPIOB->CRL |= GPIO_CRL_MODE5_1 | GPIO_CRL_MODE6_1 | GPIO_CRL_MODE7_1; // 10: Output mode, max speed 2 MHz
  GPIOB->CRL &= ~GPIO_CRL_CNF5 & ~GPIO_CRL_CNF6 & ~GPIO_CRL_CNF7; // 00: General purpose output push-pull

  /* Bật Clock PortA */
  RCC->APB2ENR |= RCC_APB2ENR_IOPAEN; // 1: Enable

  /* Cấu hình PA0, PA1, PA2 */
  GPIOA->CRL &= ~GPIO_CRL_MODE0 & ~GPIO_CRL_MODE1 & ~GPIO_CRL_MODE2; // 00: Input mode
  GPIOA->CRL &= ~GPIO_CRL_CNF0 & ~GPIO_CRL_CNF1 & ~GPIO_CRL_CNF2; // Clear
  GPIOA->CRL |= GPIO_CRL_CNF0_0 | GPIO_CRL_CNF1_0 | GPIO_CRL_CNF2_0; // 01: Floating input

  /* ___AFIO___ */

  /* Bật Clock AFIO */
  RCC->APB2ENR |= RCC_APB2ENR_AFIOEN; // 1: Enable

  /* Kết nối EXTI line0, line1, line2 với PortA */
  /* AFIO->EXTICR[0]: EXTI line3 -> EXTI line0 */
  /* AFIO->EXTICR[1]: EXTI line7 -> EXTI line4 */
  /* AFIO->EXTICR[2]: EXTI line11 -> EXTI line8 */
  /* AFIO->EXTICR[3]: EXTI line15 -> EXTI line12 */
  AFIO->EXTICR[0] |= AFIO_EXTICR1_EXTI0_PA; // 0000: PA[x] pin
  AFIO->EXTICR[0] |= AFIO_EXTICR1_EXTI1_PA; // 0000: PA[x] pin
  AFIO->EXTICR[0] |= AFIO_EXTICR1_EXTI2_PA; // 0000: PA[x] pin

  /* ___EXTI___ */

  /* Cấu hình EXTI line0, line1, line2 */
  EXTI->IMR |= EXTI_IMR_MR0 | EXTI_IMR_MR1 | EXTI_IMR_MR2; // 1: Interrupt request from Line x is not masked
  EXTI->EMR &= ~EXTI_EMR_MR0 & ~EXTI_EMR_MR1 & ~EXTI_EMR_MR2; // 0: Event request from Line x is masked
  EXTI->RTSR &= ~EXTI_RTSR_TR0 & ~EXTI_RTSR_TR1 & ~EXTI_RTSR_TR2; // 0: Rising trigger disabled (for Event and Interrupt) for input line
  EXTI->FTSR |= EXTI_FTSR_FT0 | EXTI_FTSR_FT1 | EXTI_FTSR_FT2; // 1: Falling trigger enabled (for Event and Interrupt) for input line.

  /* ___NVIC___ */

  /* Đặt mức độ ưu tiên ngắt */
  NVIC_SetPriority(EXTI0_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 1, 0));
  NVIC_SetPriority(EXTI1_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 2, 1));
  NVIC_SetPriority(EXTI2_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 2, 0));

  /* Cho phép ngắt toàn cục EXTI line0, line1, line2 */
  NVIC_EnableIRQ(EXTI0_IRQn);
  NVIC_EnableIRQ(EXTI1_IRQn);
  NVIC_EnableIRQ(EXTI2_IRQn);

  /* Hàm lặp vô hạn */
  while (1)
  {

  }
}

/* Trình phục vụ ngắt EXTI line0 */
void EXTI0_IRQHandler(void)
{
  /* Chớp tắt LED PB5 */
  for (uint8_t i = 0; i < 6; i++)
  {
    GPIOB->BSRR |= GPIO_BSRR_BS5;
    delay_ms(250);
    GPIOB->BSRR |= GPIO_BSRR_BR5;
    delay_ms(250);
  }

  /* Nếu EXTI line0 đã được ngắt và ngắt EXTI line0 được cho phép */
  if (((EXTI->PR & EXTI_PR_PIF0) == EXTI_PR_PIF0) && ((EXTI->IMR & EXTI_IMR_MR0) == EXTI_IMR_MR0))
  {
    /* Xóa cờ ngắt EXTI line0 bằng phần mềm */
    EXTI->PR |= EXTI_PR_PIF0;
  }
}

/* Trình phục vụ ngắt EXTI line1 */
void EXTI1_IRQHandler(void)
{
  /* Chớp tắt LED PB6 */
  for (uint8_t i = 0; i < 6; i++)
  {
    GPIOB->BSRR |= GPIO_BSRR_BS6;
    delay_ms(250);
    GPIOB->BSRR |= GPIO_BSRR_BR6;
    delay_ms(250);
  }

  /* Nếu EXTI line1 đã được ngắt và ngắt EXTI line1 được cho phép */
  if (((EXTI->PR & EXTI_PR_PIF1) == EXTI_PR_PIF1) && ((EXTI->IMR & EXTI_IMR_MR1) == EXTI_IMR_MR1))
  {
    /* Xóa cờ ngắt EXTI line1 bằng phần mềm */
    EXTI->PR |= EXTI_PR_PIF1;
  }
}

/* Trình phục vụ ngắt EXTI line2 */
void EXTI2_IRQHandler(void)
{
  /* Chớp tắt LED PB7 */
  for (uint8_t i = 0; i < 6; i++)
  {
    GPIOB->BSRR |= GPIO_BSRR_BS7;
    delay_ms(250);
    GPIOB->BSRR |= GPIO_BSRR_BR7;
    delay_ms(250);
  }

  /* Nếu EXTI line2 đã được ngắt và ngắt EXTI line2 được cho phép */
  if (((EXTI->PR & EXTI_PR_PIF2) == EXTI_PR_PIF2) && ((EXTI->IMR & EXTI_IMR_MR2) == EXTI_IMR_MR2))
  {
    /* Xóa cờ ngắt EXTI line2 bằng phần mềm */
    EXTI->PR |= EXTI_PR_PIF2;
  }
}
