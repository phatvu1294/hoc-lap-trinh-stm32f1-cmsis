#include "stm32f1xx.h"
#include "startup.h"

int main(void)
{
  /* Thêm chương trình chính tại đây */

  /* ___LIB___ */

  /* Khởi tạo hệ thống */
  startup_init();

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
  /* ta có Counter_Period (max) = 2^16 - 1 = 65535 => Prescaler (min) = 1099 */
  /* Chọn Prescaler = 7200, t_update = 1/10000 = 0.1us */
  /* Compare1 value = 5000 => t_compare1 = 500ms */
  /* Compare2 value = 10000 => t_compare2 = 1000ms */
  TIM4->CR1 &= ~TIM_CR1_CKD; // 00: tDTS=tCK_INT
  TIM4->CR1 &= ~TIM_CR1_CMS; // 00: Edge-aligned mode. The counter counts up or down depending on the direction bit DIR
  TIM4->CR1 &= ~TIM_CR1_DIR; // 0: Counter used as upcounter
  TIM4->CR1 &= ~TIM_CR1_ARPE; // 0: TIMx_ARR register is not buffered
  TIM4->ARR = 65535; // Auto-reload value (Counter Period)
  TIM4->PSC = 7200 - 1; // Prescaler value
  TIM4->RCR = 0 & 0x00FF; // Repetition counter value
  TIM4->EGR |= TIM_EGR_UG; // 1: Reinitialize the counter and generates an update of the registers

  /* Bật TIM4 */
  TIM4->CR1 |= TIM_CR1_CEN; // 1: Counter enabled

  /* Cấu hình TIM4 PWM Chanel 1 */
  TIM4->CCMR1 &= ~TIM_CCMR1_OC1M; // Clear;
  TIM4->CCMR1 |= TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_0; // 011: Toggle - OC1REF toggles when TIMx_CNT=TIMx_CCR1.
  TIM4->CCMR1 &= ~TIM_CCMR1_CC1S; // 00: CC1 channel is configured as output
  TIM4->CCMR1 |= TIM_CCMR1_OC1FE; // 1: Output Compare 1 fast enabled
  TIM4->CCMR1 &= ~TIM_CCMR1_OC1PE; // 0: Output Compare 1 preload disabled
  TIM4->CCR1 = 5000 - 1; // Capture/Compare 1 value (Duty)
  TIM4->CCER &= ~TIM_CCER_CC1P; // 0: OC1 active high
  TIM4->CCER |= TIM_CCER_CC1E; // 1: Capture/Compare 1 output enabled

  /* Cho phép ngắt TIM4 CC1 */
  TIM4->DIER |= TIM_DIER_CC1IE;

  /* Cấu hình TIM4 PWM Chanel 2 */
  TIM4->CCMR1 &= ~TIM_CCMR1_OC2M; // Clear;
  TIM4->CCMR1 |= TIM_CCMR1_OC2M_1 | TIM_CCMR1_OC2M_0; // 011: Toggle - OC2REF toggles when TIMx_CNT=TIMx_CCR2.
  TIM4->CCMR1 &= ~TIM_CCMR1_CC2S; // 00: CC2 channel is configured as output
  TIM4->CCMR1 |= TIM_CCMR1_OC2FE; // 1: Output Compare 2 fast enabled
  TIM4->CCMR1 &= ~TIM_CCMR1_OC2PE; // 0: Output Compare 2 preload disabled
  TIM4->CCR2 = 10000 - 1; // Capture/Compare 2 value (Duty)
  TIM4->CCER &= ~TIM_CCER_CC2P; // 0: OC2 active high
  TIM4->CCER |= TIM_CCER_CC2E; // 1: Capture/Compare 2 output enabled

  /* Cho phép ngắt TIM4 CC2 */
  TIM4->DIER |= TIM_DIER_CC2IE;

  /* ___NVIC___ */

  /* Cấu hình mức độ ưu tiên ngắt TIM4 */
  NVIC_SetPriority(TIM4_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 0, 0));

  /* Cho phép ngắt toàn cục TIM4 */
  NVIC_EnableIRQ(TIM4_IRQn);

  /* Hàm lặp vô hạn */
  while (1)
  {

  }
}

/* Trình phục vụ ngắt TIM4 */
void TIM4_IRQHandler(void)
{
  /* Biến lưu giá trị compare */
  uint32_t compare = 0;

  /* Nếu cờ CC1 được set và ngắt được cho phép */
  if (((TIM4->SR & TIM_SR_CC1IF) == TIM_SR_CC1IF) && ((TIM4->DIER & TIM_DIER_CC1IE) == TIM_DIER_CC1IE))
  {
    /* Lưu giá trị cũ */
    compare = TIM4->CCR1;
    
    /* Đặt giá trị Compare mới */
    TIM4->CCR1 = compare + 5000;
    
    /* Xóa cờ CC1 */
    TIM4->SR &= ~TIM_SR_CC1IF;
  }

  /* Nếu cờ CC2 được set và ngắt được cho phép */
  if (((TIM4->SR & TIM_SR_CC2IF) == TIM_SR_CC2IF) && ((TIM4->DIER & TIM_DIER_CC2IE) == TIM_DIER_CC2IE))
  {
    /* Lưu giá trị cũ */
    compare = TIM4->CCR2;
    
    /* Đặt giá trị Compare mới */
    TIM4->CCR2 = compare + 10000;
    
    /* Xóa cờ CC1 */
    TIM4->SR &= ~TIM_SR_CC2IF;
  }
}
