#include "stm32f1xx.h"
#include "startup.h"
#include "usart1.h"
#include <stdio.h>

uint16_t counter = 0;
uint16_t counterOld = 0;
uint8_t direction = 0;
char strBuffer[32];

void TIM3_IRQHandler(void);

int main(void)
{
  /* Thêm chương trình chính tại đây */

  /* ___LIB___ */

  /* Khởi tạo hệ thống */
  startup_init();

  /* Khởi tạo usart1 */
  usart1_init();

  /* ___GPIO___ */

  /* Bật Clock PortA */
  RCC->APB2ENR |= RCC_APB2ENR_IOPAEN; // 1: Enable

  /* Cấu hình PA6 (TIM3 Input Capture Channel 1), Cấu hình PA7 (TIM3 Input Capture Channel 2) */
  GPIOA->CRL &= ~GPIO_CRL_MODE6 & ~GPIO_CRL_MODE7; // 00: Input mode
  GPIOA->CRL &= ~GPIO_CRL_CNF6 & ~GPIO_CRL_CNF7; // Clear;
  GPIOA->CRL |= GPIO_CRL_CNF6_0 | GPIO_CRL_CNF7_0; //01: Floating input

  /* ___TIM___ */

  /* Bật Clock TIM3 */
  RCC->APB1ENR |= RCC_APB1ENR_TIM3EN; // 1: Enable

  /* Cấu hình TIM3 Time Base */
  TIM3->CR1 &= ~TIM_CR1_CKD; // 00: tDTS=tCK_INT
  TIM3->CR1 &= ~TIM_CR1_CMS; // 00: Edge-aligned mode. The counter counts up or down depending on the direction bit DIR
  TIM3->CR1 &= ~TIM_CR1_DIR; // 0: Counter used as upcounter
  TIM3->CR1 &= ~TIM_CR1_ARPE; // 0: TIMx_ARR register is not buffered
  TIM3->ARR = 65535; // Auto-reload value (Counter Period)
  TIM3->PSC = 3 - 1; // Prescaler value
  TIM3->RCR = 0 & 0x00FF; // Repetition counter value
  TIM3->EGR |= TIM_EGR_UG; // 1: Reinitialize the counter and generates an update of the registers

  /* Bật TIM3 */
  TIM3->CR1 |= TIM_CR1_CEN; // 1: Counter enabled

  /* TIM3 Encoder Mode */
  TIM3->SMCR &= ~TIM_SMCR_SMS; // Clear
  TIM3->SMCR |= TIM_SMCR_SMS_1 | TIM_SMCR_SMS_0; // 011: Encoder mode 3 - Counter counts up/down on both TI1FP1 and TI2FP2 edges depending on the level of the other input

  /* Cấu hình TIM3 Input Capture Channel 1 Direct (Channel A)  */
  TIM3->CCMR1 &= ~TIM_CCMR1_IC1F; // Clear
  TIM3->CCMR1 |= TIM_CCMR1_IC1F_2; // 0100: fSAMPLING=fDTS/2, N=6
  TIM3->CCMR1 &= ~TIM_CCMR1_IC1PSC; // 00: no prescaler, capture is done each time an edge is detected on the capture input
  TIM3->CCMR1 &= ~TIM_CCMR1_CC1S; // Clear
  TIM3->CCMR1 |= TIM_CCMR1_CC1S_0; // 01: CC1 channel is configured as input, IC1 is mapped on TI1
  TIM3->CCER |= TIM_CCER_CC1P; // 1: inverted: capture is done on a "falling edge" of IC1. When used as external trigger, IC1 is inverted
  TIM3->CCER |= TIM_CCER_CC1E; // 1: Capture/Compare 1 output enabled

  /* Cấu hình TIM3 Input Capture Channel 2 Direct (Channel B)  */
  TIM3->CCMR1 &= ~TIM_CCMR1_IC2F; // Clear
  TIM3->CCMR1 |= TIM_CCMR1_IC2F_2; // 0100: fSAMPLING=fDTS/2, N=6
  TIM3->CCMR1 &= ~TIM_CCMR1_IC2PSC; // 00: no prescaler, capture is done each time an edge is detected on the capture input
  TIM3->CCMR1 &= ~TIM_CCMR1_CC2S; // Clear
  TIM3->CCMR1 |= TIM_CCMR1_CC2S_0; // 01: CC2 channel is configured as input, IC2 is mapped on TI2
  TIM3->CCER |= TIM_CCER_CC2P; // 1: inverted: capture is done on a "falling edge" of IC2. When used as external trigger, IC2 is inverted
  TIM3->CCER |= TIM_CCER_CC2E; // 1: Capture/Compare 2 output enabled

  /* Hàm lặp vô hạn */
  while (1)
  {
    /* Đọc giá trị của Encoder */
    counter = TIM3->CNT;

    /* Đọc hướng của Encoder */
    direction = ((TIM3->CR1 & TIM_CR1_DIR) == TIM_CR1_DIR);

    /* Nếu giá trị thay đổi */
    if (counter != counterOld)
    {
      /* Gửi giá trị ra usart1 */
      sprintf(strBuffer, "Counter=%d, Direction=%d\r\n", counter, direction);
      usart1_putString((uint8_t *)strBuffer);

      /* Lưu lại giá trị cũ */
      counterOld = counter;
    }
  }
}
