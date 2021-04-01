#include "stm32f1xx.h"
#include "startup.h"
#include "delay.h"
#include "usart1.h"
#include <stdio.h>

uint32_t icValue1 = 0;
uint32_t icValue2 = 0;
uint32_t diffCapture = 0;
uint16_t captureIndex = 0;
double frequency = 0.0;
char strBuffer[32];

void TIM3_IRQHandler(void);

int main(void)
{
  /* Thêm chương trình chính tại đây */

  /* ___LIB___ */

  /* Khởi tạo hệ thống */
  startup_init();

  /* Khởi tạo delay */
  delay_init();

  /* Khởi tạo usart1 */
  usart1_init();

  /* ___GPIO___ */

  /* Bật Clock PortB */
  RCC->APB2ENR |= RCC_APB2ENR_IOPBEN; // 1: Enable

  /* Reset PB6 */
  GPIOB->BSRR |= GPIO_BSRR_BR6;

  /* Cấu hình PB6 (TIM4 PWM Channel 1) */
  GPIOB->CRL &= ~GPIO_CRL_MODE6; // Clear
  GPIOB->CRL |= GPIO_CRL_MODE6_1; // 10: Output mode, max speed 2 MHz
  GPIOB->CRL &= ~GPIO_CRL_CNF6; // Clear;
  GPIOB->CRL |= GPIO_CRL_CNF6_1; // 10: Alternate function output Push-pull

  /* Bật Clock PortA */
  RCC->APB2ENR |= RCC_APB2ENR_IOPAEN; // 1: Enable

  /* Cấu hình PA6 (TIM3 Input Capture Channel 1) */
  GPIOA->CRL &= ~GPIO_CRL_MODE6; // 00: Input mode
  GPIOA->CRL &= ~GPIO_CRL_CNF6; // Clear;
  GPIOA->CRL |= GPIO_CRL_CNF6_0; //01: Floating input

  /* ___TIM___ */

  /* Bật Clock TIM4 */
  RCC->APB1ENR |= RCC_APB1ENR_TIM4EN; // 1: Enable;

  /* Cấu hình TIM4 Time Base */
  /* f_clock_source/f_update = Prescaler * (Counter_Period + 1) */
  /* có f_clock_source = 72MHz */
  /* Với f_update = 1kHz => 72000 = Prescaler * (Counter_Period + 1) */
  /* ta có Counter_Period (max) = 2^16 - 1 = 65535 => Prescaler (min) = 2 */
  /* Chọn Prescaler = 72 => Counter_Period = 1000-1 */
  TIM4->CR1 &= ~TIM_CR1_CKD; // 00: tDTS=tCK_INT
  TIM4->CR1 &= ~TIM_CR1_CMS; // 00: Edge-aligned mode. The counter counts up or down depending on the direction bit DIR
  TIM4->CR1 &= ~TIM_CR1_DIR; // 0: Counter used as upcounter
  TIM4->CR1 &= ~TIM_CR1_ARPE; // 0: TIMx_ARR register is not buffered
  TIM4->ARR = 1000 - 1; // Auto-reload value (Counter Period)
  TIM4->PSC = 72 - 1; // Prescaler value
  TIM4->RCR = 0 & 0x00FF; // Repetition counter value
  TIM4->EGR |= TIM_EGR_UG; // 1: Reinitialize the counter and generates an update of the registers

  /* Bật TIM4 */
  TIM4->CR1 |= TIM_CR1_CEN; // 1: Counter enabled

  /* Cấu hình TIM4 PWM Chanel 1 */
  TIM4->CCMR1 &= ~TIM_CCMR1_OC1M; // Clear;
  TIM4->CCMR1 |= TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1; // 110: PWM mode 1
  TIM4->CCMR1 &= ~TIM_CCMR1_CC1S; // 00: CC1 channel is configured as output
  TIM4->CCMR1 |= TIM_CCMR1_OC1FE; // 1: Output Compare 1 fast enabled
  TIM4->CCMR1 &= ~TIM_CCMR1_OC1PE; // 0: Output Compare 1 preload disabled
  TIM4->CCR1 = 500 - 1; // Capture/Compare 1 value (Duty)
  TIM4->CCER &= ~TIM_CCER_CC1P; // 0: OC1 active high
  TIM4->CCER |= TIM_CCER_CC1E; // 1: Capture/Compare 1 output enabled

  /* Bật Clock TIM3 */
  RCC->APB1ENR |= RCC_APB1ENR_TIM3EN; // 1: Enable

  /* Cấu hình TIM3 Time Base */
  /* f_clock_source = f_input_signal * Prescaler * diff */
  /* có f_clock_source = 72MHz */
  /* Để đo được f_input_signal = 1kHz thì: 72000 = Prescaler * diff */
  /* ta có diff (max) = 2^16 - 1 = 65535 => Prescaler (min) = 1.099 */
  /* Chọn Prescaler = 2 */
  TIM3->CR1 &= ~TIM_CR1_CKD; // 00: tDTS=tCK_INT
  TIM3->CR1 &= ~TIM_CR1_CMS; // 00: Edge-aligned mode. The counter counts up or down depending on the direction bit DIR
  TIM3->CR1 &= ~TIM_CR1_DIR; // 0: Counter used as upcounter
  TIM3->CR1 &= ~TIM_CR1_ARPE; // 0: TIMx_ARR register is not buffered
  TIM3->ARR = 65535; // Auto-reload value (Counter Period)
  TIM3->PSC = 2 - 1; // Prescaler value
  TIM3->RCR = 0 & 0x00FF; // Repetition counter value
  TIM3->EGR |= TIM_EGR_UG; // 1: Reinitialize the counter and generates an update of the registers

  /* Bật TIM3 */
  TIM3->CR1 |= TIM_CR1_CEN; // 1: Counter enabled

  /* Cấu hình TIM3 Input Capture Channel 1 */
  TIM3->CCMR1 &= ~TIM_CCMR1_IC1F; // 0000: No filter, sampling is done at fDTS
  TIM3->CCMR1 &= ~TIM_CCMR1_IC1PSC; // 00: no prescaler, capture is done each time an edge is detected on the capture input
  TIM3->CCMR1 &= ~TIM_CCMR1_CC1S; // Clear
  TIM3->CCMR1 |= TIM_CCMR1_CC1S_0; //01: CC1 channel is configured as input, IC1 is mapped on TI1
  TIM3->CCER &= ~TIM_CCER_CC1P; // 0: non-inverted: capture is done on a rising edge of IC1. When used as external trigger, IC is non-inverted.
  TIM3->CCER |= TIM_CCER_CC1E; // 1: Capture/Compare 1 output enabled

  /* Cho phép ngắt TIM3 Capture/Compare */
  TIM3->DIER |= TIM_DIER_CC1IE; // 1: Capture enabled

  /* ___NVIC___ */

  /* Cấu hình mức độ ưu tiên ngắt TIM3 */
  NVIC_SetPriority(TIM3_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 0, 0));

  /* Cho phép ngắt toàn cục TIM3 */
  NVIC_EnableIRQ(TIM3_IRQn);

  /* Hàm lặp vô hạn */
  while (1)
  {
    /* Gửi giá trị tần số ra usart1 */
    sprintf(strBuffer, "Frequency=%.2fHz\r\n", frequency);
    usart1_putString((uint8_t *)strBuffer);
    delay_ms(1000);
  }
}

/* Trình phục vụ ngắt TIM3 */
void TIM3_IRQHandler(void)
{
  /* Nếu cờ CC1IF được set và ngắt TIM3 Input Capture Channel 1 được cho phép */
  if (((TIM3->SR & TIM_SR_CC1IF) == TIM_SR_CC1IF) && ((TIM3->DIER & TIM_DIER_CC1IE) == TIM_DIER_CC1IE))
  {
    if (captureIndex == 0)
    {
      /* Đọc giá trị Capture cũ */
      icValue1 = TIM3->CCR1;
      captureIndex = 1;
    }
    else if (captureIndex == 1)
    {
      /* Đọc giá trị Capture mới */
      icValue2 = TIM3->CCR1;

      /* Kiểm tra 2 giá trị Capture */
      if (icValue2 > icValue1)
      {
        diffCapture = icValue2 - icValue1;
      }
      else if (icValue2 < icValue1)
      {
        diffCapture = ((65535 - icValue1) + icValue2) + 1;
      }

      /* f_input_signal = f_clock_source / prescaler / diff */
      frequency = 72000000.0 / 2.0 / diffCapture;
      captureIndex = 0;
    }

    /* Xóa cờ CC1IF */
    TIM3->SR &= ~TIM_SR_CC1IF;
  }
}
