#include "stm32f1xx.h"
#include "startup.h"
#include "usart1.h"
#include "delay.h"
#include <stdio.h>

uint16_t adcValue = 0;
uint8_t adcLevelOutTrigger = 0;
char strBuffer[32];

void ADC1_2_IRQHandler(void);

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

  /* Bật Clock PortA */
  RCC->APB2ENR |= RCC_APB2ENR_IOPAEN; // 1: Enable

  /* Cấu hình PA6 (ADC1 Channel6) */
  GPIOA->CRL &= ~GPIO_CRL_MODE6; // 00: Input mode
  GPIOA->CRL &= ~GPIO_CRL_CNF6; // 00: Analog mode

  /* Bật Clock PortC */
  RCC->APB2ENR |= RCC_APB2ENR_IOPCEN; // 1: Enable

  /* Cấu hình PC13 */
  GPIOC->CRH &= ~GPIO_CRH_MODE13; // Clear
  GPIOC->CRH |= GPIO_CRH_MODE13_1; // 10: Output mode, max speed 2 MHz
  GPIOC->CRH &= ~GPIO_CRH_CNF13; // 00: General purpose output push-pull

  /* ___ADC___ */

  /* Bật Clock ADC1 */
  RCC->APB2ENR |= RCC_APB2ENR_ADC1EN; // 1: Enable

  /* Cấu hình Clock ADC1 */
  RCC->CFGR &= ~RCC_CFGR_ADCPRE; // Clear
  RCC->CFGR |= RCC_CFGR_ADCPRE_0; // 01: PCLK2 divided by 4

  /* Cấu hình ADC1 */
  ADC1->CR1 &= ~ADC_CR1_DUALMOD; // 0000: Independent mode
  ADC1->CR1 &= ~ADC_CR1_SCAN; // 0: Scan mode disabled
  ADC1->CR2 |= ADC_CR2_CONT; // 1: Continuous conversion mode
  ADC1->CR2 &= ~ADC_CR2_ALIGN; // 0: Right Alignment
  ADC1->CR2 |= ADC_CR2_EXTSEL; // 111: SWSTART (Software Trigger)
  ADC1->SQR1 &= ~ADC_SQR1_L; // 0000: 1 conversion (Regular channel sequence length)

  /* Cấu hình ADC1 Rank 1 */
  uint32_t adcChannel = 6;
  uint32_t adcRank = 1;
  ADC1->SQR3 |= (adcChannel << (5 * (adcRank - 1))); // Rank = 1th conversion in regular sequence
  ADC1->SMPR2 |= ADC_SMPR2_SMP6; // 111: 239.5 cycles

  /* Cấu hình Analog Watchdog Channel 6 */
  //ADC1->CR1 &= ~ ADC_CR1_AWDCH; // Clear
  //ADC1->CR1 |= ADC_CR1_AWDCH_1 | ADC_CR1_AWDCH_2; // 00110: ADC analog Channel6
  // Hoặc sử dụng
  ADC1->CR1 |= (adcChannel << 0);
  ADC1->CR1 |= ADC_CR1_AWDSGL; // 1: Analog watchdog enabled on a single channel
  ADC1->HTR = 2500; // Analog watchdog high threshold
  ADC1->LTR = 1500; // Analog watchdog low threshold

  /* Cho phép ngắt ADC1 EOC */
  ADC1->CR1 |= ADC_CR1_EOCIE; // 1: EOC interrupt enabled. An interrupt is generated when the EOC bit is set.

  /* Cho phép ngắt ADC1 AWD */
  ADC1->CR1 |= ADC_CR1_AWDIE; // 1: Analog watchdog interrupt enabled

  /* Bật Analog Watchdog Channel6 */
  ADC1->CR1 |= ADC_CR1_AWDEN; // 1: Analog watchdog enabled on regular channels

  /* Bật ADC1 */
  /* If this bit holds a value of zero and a 1 is written to it then it wakes up the ADC from Power Down state */
  /* Conversion starts when this bit holds a value of 1 and a 1 is written to it. The application should allow a delay of "tSTAB" between power up and start of conversion */
  ADC1->CR2 |= ADC_CR2_ADON; // hold_0->write_1: Wakes up the ADC from Power Down state
  __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); // tSTAB
  ADC1->CR2 |= ADC_CR2_ADON; // hold_1->write_1: Conversion starts

  /* Hiệu chuẩn điện áp tham chiếu (Vref) */
  ADC1->CR2 |= ADC_CR2_RSTCAL; // 1: Initialize calibration register
  while ((ADC1->CR2 & ADC_CR2_RSTCAL) == ADC_CR2_RSTCAL); // This bit is set by software and cleared by hardware. It is cleared after the calibration registersare initialized
  ADC1->CR2 |= ADC_CR2_CAL; // 1: Enable calibration
  while ((ADC1->CR2 & ADC_CR2_CAL) == ADC_CR2_CAL); // This bit is set by software to start the calibration. It is reset by hardware after calibration is complete

  /* ___NVIC___ */

  /* Cấu hình mức độ ưu tiên ngắt ADC1 */
  NVIC_SetPriority(ADC1_2_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 0, 0));

  /* Cho phép ngắt toàn cục ADC1 */
  NVIC_EnableIRQ(ADC1_2_IRQn);

  /* ___MAIN___ */

  /* Bắt đầu chuyển đổi */
  ADC1->CR2 |= ADC_CR2_SWSTART; // 1: Starts conversion of regular channels

  /* Hàm lặp vô hạn */
  while (1)
  {
    /* Gửi giá trị ADC1 Channel 6 ra usart1 */
    sprintf(strBuffer, "ADC1 (CH6)=%d\r\n", adcValue);
    usart1_putString((uint8_t *)strBuffer);

    /* Nếu có cảnh báo ngưỡng */
    if (adcLevelOutTrigger == 1)
    {
      /* Bật LED cảnh báo ngưỡng */
      GPIOC->BSRR |= GPIO_BSRR_BR13; // Reset PC13

      /* Đặt lại ngưỡng cảnh báo */
      adcLevelOutTrigger = 0;
    }
    else
    {
      /* Tắt LED cảnh báo ngưỡng */
      GPIOC->BSRR |= GPIO_BSRR_BS13; // Set PC13
    }

    delay_ms(100);
  }
}

/* Trình phục vụ ngắt ADC1 */
void ADC1_2_IRQHandler(void)
{
  /* Nếu cờ EOC được set và ngắt EOC được cho phép */
  if (((ADC1->SR & ADC_SR_EOC) == ADC_SR_EOC) && ((ADC1->CR1 & ADC_CR1_EOCIE) == ADC_CR1_EOCIE))
  {
    /* Đọc giá trị ADC1 */
    adcValue = ADC1->DR;

    /* Xóa cờ EOC */
    ADC1->SR &= ~ADC_SR_EOC;
  }

  /* Nếu cờ AWD được set và ngắt AWD được cho phép */
  if (((ADC1->SR & ADC_SR_AWD) == ADC_SR_AWD) && ((ADC1->CR1 & ADC_CR1_AWDIE) == ADC_CR1_AWDIE))
  {
    /* Cảnh báo ngưỡng (giá trị nằm ngoài mức đặt) */
    adcLevelOutTrigger = 1;

    /* Xóa cờ AWD */
    ADC1->SR &= ~ADC_SR_AWD;
  }
}
