#include "stm32f1xx.h"
#include "startup.h"
#include "usart1.h"
#include <stdio.h>

/* ADC1 Channel 6 -> Đọc ADC chế độ DMA Interrupt, kích hoạt mỗi khi có sofware */
/* ADC1 Channel 7 -> Đọc ADC chế độ Interrupt, kích hoạt mỗi khi TIM4 TRGO Update */
/* ADC1 Channel 7 (Injected) có mức ưu tiên ngắt cao hơn, nên sẽ làm gián đoạn ADC Channel 6 (Regular) */
/* Khi ngắt ADC1 Channel 7 kết thúc, ADC1 Channel 6 sẽ tiếp tục */
/* Chế độ này giống với Interrupt nhưng chỉ dành cho ADC */

uint16_t regularADCValue;
uint16_t injectedADCValue;
char strBuffer[32];

void DMA1_Channel1_IRQHandler(void);
void ADC1_2_IRQHandler(void);

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

  /* Cấu hình PA6 (ADC1 Channel6), PA7 (ADC1 Channel7) */
  GPIOA->CRL &= ~GPIO_CRL_MODE6 & ~GPIO_CRL_MODE7; // 00: Input mode
  GPIOA->CRL &= ~GPIO_CRL_CNF6 & ~GPIO_CRL_CNF7; // 00: Analog mode

  /* ___DMA___ */

  /* Bật Clock DMA1 */
  RCC->AHBENR |= RCC_AHBENR_DMA1EN; // 1: Enable

  /* Cấu hình DMA1 Channel 1 (Xem channel trong bảng 78) */
  DMA1_Channel1->CCR &= ~DMA_CCR_DIR; // 0: Read from peripheral
  DMA1_Channel1->CCR &= ~DMA_CCR_MEM2MEM; // 0: Memory to memory mode disabled
  DMA1_Channel1->CCR &= ~DMA_CCR_PL; // 00: Low priority
  DMA1_Channel1->CCR &= ~DMA_CCR_MSIZE; // Clear
  DMA1_Channel1->CCR |= DMA_CCR_MSIZE_0; //01: Memory size 16-bits
  DMA1_Channel1->CCR &= ~DMA_CCR_PSIZE; // Clear
  DMA1_Channel1->CCR |= DMA_CCR_PSIZE_0; //01: Periph size 16-bits
  DMA1_Channel1->CCR |= DMA_CCR_MINC; // 1: Memory increment mode enabled
  DMA1_Channel1->CCR &= ~DMA_CCR_PINC; // 0: Peripheral increment mode disabled
  DMA1_Channel1->CCR |= DMA_CCR_CIRC; // 1: Circular mode enabled
  DMA1_Channel1->CNDTR = (uint32_t)1; // Buffer size
  DMA1_Channel1->CPAR = (uint32_t)0x40012400 + (uint32_t)0x4C; // periph address, (ADC_BASE_ADDR + OFFSET của thanh ghi ADC1_DR)
  DMA1_Channel1->CMAR = (uint32_t)&regularADCValue; // memory address

  /* Cho phép ngắt DMA1 Channel1 TC */
  DMA1_Channel1->CCR |= DMA_CCR_TCIE; // 1: TC interrupt enabled

  /* Bật DMA1 Channel1 */
  DMA1_Channel1->CCR |= DMA_CCR_EN; // 1: Channel enabled

  /* ___TIM___ */

  /* Bật Clock TIM4 */
  RCC->APB1ENR |= RCC_APB1ENR_TIM4EN; // 1: Enable

  /* Cấu hình TIM4 Time Base */
  TIM4->CR1 &= ~TIM_CR1_CKD; // 00: tDTS=tCK_INT
  TIM4->CR1 &= ~TIM_CR1_CMS; // 00: Edge-aligned mode. The counter counts up or down depending on the direction bit DIR
  TIM4->CR1 &= ~TIM_CR1_DIR; // 0: Counter used as upcounter
  TIM4->CR1 &= ~TIM_CR1_ARPE; // 0: TIMx_ARR register is not buffered
  TIM4->ARR = 30000 - 1; // Auto-reload value (Counter Period)
  TIM4->PSC = 7200 - 1; // Prescaler value
  TIM4->RCR = 0 & 0x00FF; // Repetition counter value
  TIM4->EGR |= TIM_EGR_UG; // 1: Reinitialize the counter and generates an update of the registers

  /* Bật TIM4 */
  TIM4->CR1 |= TIM_CR1_CEN; // 1: Counter enabled

  /* Cấu hình TIM4 Trigger Output (TRGO) */
  TIM4->SMCR &= ~TIM_SMCR_MSM;  // Master/Slave mode, 0: No action
  TIM4->CR2 &= ~TIM_CR2_MMS; // Clear
  TIM4->CR2 |= TIM_CR2_MMS_1; // 010: Update - The update event is selected as trigger output (TRGO). For instance a master timer can then be used as a prescaler for a slave timer.

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
  ADC1->CR2 |= ADC_CR2_EXTSEL; // 111: SWSTART
  uint32_t regularADCLength = 1;
  uint32_t injectedADCLength = 1;
  ADC1->SQR1 |= ((regularADCLength - 1) << 20); // 0000: 1 conversion (Regular channel sequence length)
  ADC1->JSQR |= ((injectedADCLength - 1) << 20); // 00: 1 conversion

  /* Cấu hình Regular ADC1 Rank 1 */
  uint32_t adcChannel = 6;
  uint32_t adcRank = 1;
  ADC1->SQR3 |= (adcChannel << (5 * (adcRank - 1))); // Rank = 1th conversion in regular sequence
  ADC1->SMPR2 |= ADC_SMPR2_SMP6; // 111: 239.5 cycles

  /* Bật ADC1 DMA */
  ADC1->CR2 |= ADC_CR2_DMA; // 1: DMA mode enabled

  /* Cấu hình Injected ADC1 Rank 1 */
  adcChannel = 7;
  adcRank = 1;

  /* Lưu ý: */
  /* When JL=3 ( 4 injected conversions in the sequencer), the ADC converts the channels in this order: */
  /* JSQ1[4:0] >> JSQ2[4:0] >> JSQ3[4:0] >> JSQ4[4:0] */
  /* When JL=2 ( 3 injected conversions in the sequencer), the ADC converts the channels in this order: */
  /* JSQ2[4:0] >> JSQ3[4:0] >> JSQ4[4:0] */
  /* When JL=1 ( 2 injected conversions in the sequencer), the ADC converts the channels in this order: */
  /* JSQ3[4:0] >> JSQ4[4:0] */
  /* When JL=0 (1 injected conversion in the sequencer), the ADC converts only JSQ4[4:0] channel */
  ADC1->JSQR |= (adcChannel << (5 * (4 - adcRank))); // fourth conversion in injected sequence
  ADC1->SMPR2 |= ADC_SMPR2_SMP7; // 111: 239.5 cycles

  /* Chọn đầu vào Trigger kênh injecter */
  ADC1->CR2 &= ~ADC_CR2_JEXTSEL; // Clear
  ADC1->CR2 |= ADC_CR2_JEXTSEL_2 | ADC_CR2_JEXTSEL_0; // 101: Timer 4 TRGO event

  /* Cho phép ngắt ADC1 JEOC */
  ADC1->CR1 |= ADC_CR1_JEOCIE;

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

  /* Cấu hình mức độ ưu tiên ngắt DMA1 Channel1 và ADC1 */
  NVIC_SetPriority(DMA1_Channel1_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 0, 0));
  NVIC_SetPriority(ADC1_2_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 0, 0));

  /* Cho phép ngắt toàn cục DMA1 Channel1 và ADC1 */
  NVIC_EnableIRQ(DMA1_Channel1_IRQn);
  NVIC_EnableIRQ(ADC1_2_IRQn);

  /* ___MAIN___ */

  /* Bắt đầu chuyển đổi ADC1 Channel 6 Regular software trigger */
  ADC1->CR2 |= ADC_CR2_SWSTART; // 1: Starts conversion of regular channels

  /* Bắt đầu chuyển đổi ADC1 Channel 7 Injected kích ngưỡng bởi TIM4 Trigger Output */
  ADC1->CR2 |= ADC_CR2_JEXTTRIG; // 1: Conversion on external event enabled

  /* Hàm lặp vô hạn */
  while (1)
  {

  }
}

/* Trình phục vụ ngắt DMA1 Channel1 */
void DMA1_Channel1_IRQHandler(void)
{
  /* Nếu cờ DMA1 TC được set và ngắt DMA1 TC được cho phép */
  if (((DMA1->ISR & DMA_ISR_TCIF1) == DMA_ISR_TCIF1) && ((DMA1_Channel1->CCR & DMA_CCR_TCIE) == DMA_CCR_TCIE))
  {
    /* Gửi giá trị ADC1 Channel 6 ra usart1 */
    sprintf(strBuffer, "ADC1 (CH6 Regular)=%d\r\n", regularADCValue);
    usart1_putString((uint8_t *)strBuffer);

    /* Xóa cờ GL, TE, HT, TC */
    DMA1->IFCR |= DMA_IFCR_CGIF1; // 1: Clears the GIF, TEIF, HTIF and TCIF flags in the DMA_ISR register
  }
}

/* Trình phục vụ ngăt ADC1 */
void ADC1_2_IRQHandler(void)
{
  /* Nếu cờ JEOC được set và ngắt JEOC được cho phép */
  if (((ADC1->SR & ADC_SR_JEOC) == ADC_SR_JEOC) && ((ADC1->CR1 & ADC_CR1_JEOCIE) == ADC_CR1_JEOCIE))
  {
    /* Đọc giá trị Injeted ADC channel 1*/
    injectedADCValue = ADC1->JDR1;

    /* Gửi giá trị ADC1 Channel 7 ra usart1 */
    sprintf(strBuffer, "ADC1 (CH7 Injected)=%d\r\n", injectedADCValue);
    usart1_putString((uint8_t *)strBuffer);

    /* Xóa cờ JEOC */
    ADC1->SR &= ~ADC_SR_JEOC;
  }
}
