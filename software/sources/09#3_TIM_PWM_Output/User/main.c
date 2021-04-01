#include "stm32f1xx.h"
#include "startup.h"
#include "delay.h"

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

  /* Reset PB6, PB7 */
  GPIOB->BSRR |= GPIO_BSRR_BR6;
  GPIOB->BSRR |= GPIO_BSRR_BR7;

  /* Cấu hình PB6 (TIM4 PWM Channel 1), PB7 (TIM4 PWM Channel 2) */
  GPIOB->CRL &= ~GPIO_CRL_MODE6 & ~GPIO_CRL_MODE7; // Clear
  GPIOB->CRL |= GPIO_CRL_MODE6_1 | GPIO_CRL_MODE7_1; // 10: Output mode, max speed 2 MHz
  GPIOB->CRL &= ~GPIO_CRL_CNF6 & ~GPIO_CRL_CNF7; // Clear;
  GPIOB->CRL |= GPIO_CRL_CNF6_1 | GPIO_CRL_CNF7_1; // 10: Alternate function output Push-pull

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

  /* Cấu hình TIM4 PWM Chanel 2 */
  TIM4->CCMR1 &= ~TIM_CCMR1_OC2M; // Clear;
  TIM4->CCMR1 |= TIM_CCMR1_OC2M_2 | TIM_CCMR1_OC2M_1; // 110: PWM mode 1
  TIM4->CCMR1 &= ~TIM_CCMR1_CC2S; // 00: CC2 channel is configured as output
  TIM4->CCMR1 |= TIM_CCMR1_OC2FE; // 1: Output Compare 2 fast enabled
  TIM4->CCMR1 &= ~TIM_CCMR1_OC2PE; // 0: Output Compare 2 preload disabled
  TIM4->CCR2 = 500 - 1; // Capture/Compare 2 value (Duty)
  TIM4->CCER &= ~TIM_CCER_CC2P; // 0: OC2 active high
  TIM4->CCER |= TIM_CCER_CC2E; // 1: Capture/Compare 2 output enabled

  /* Hàm lặp vô hạn */
  while (1)
  {
    /* Đặt giá trị Compare (Duty) */
    for (uint16_t i = 0; i < 1000; i++)
    {
      TIM4->CCR1 = i;
      TIM4->CCR2 = 999 - i;
      delay_ms(2);
    }
    delay_ms(1000);

    /* Đặt giá trị Compare (Duty) */
    for (uint16_t i = 0; i < 1000; i++)
    {
      TIM4->CCR1 = 999 - i;
      TIM4->CCR2 = i;
      delay_ms(2);
    }
    delay_ms(1000);
  }
}
