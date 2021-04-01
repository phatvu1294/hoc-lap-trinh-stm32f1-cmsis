#include "stm32f1xx.h"
#include "startup.h"

#define BUFFER_SIZE 4

uint32_t srcBuffer[BUFFER_SIZE] = {0xAAAAAAAA, 0xBBBBBBBB, 0xCCCCCCCC, 0xDDDDDDDD};
uint32_t dstBuffer[BUFFER_SIZE];
uint8_t tranferComplete = 0;

uint8_t buffer_compare(const uint32_t* pBuffer, uint32_t* pBuffer1, uint16_t bufferLength);
void DMA1_Channel1_IRQHandler(void);

int main(void)
{
  /* Thêm chương trình chính tại đây */

  /* ___LIB___ */

  /* Khởi tạo hệ thống */
  startup_init();

  /* ___GPIO___ */

  /* Bật Clock PortC */
  RCC->APB2ENR |= RCC_APB2ENR_IOPCEN; // 1: Enable

  /* Set PC13 */
  GPIOC->BSRR |= GPIO_BSRR_BS13;

  /* Cấu hình PC13 */
  GPIOC->CRH &= ~GPIO_CRH_MODE13; // Clear
  GPIOC->CRH |= GPIO_CRH_MODE13_1; // 10: Output mode, max speed 2 MHz
  GPIOC->CRH &= ~GPIO_CRH_CNF13; // 00: General purpose output push-pull

  /* ___DMA___ */

  /* Bật Clock DMA */
  RCC->AHBENR |= RCC_AHBENR_DMA1EN; // 1: Enable

  /* Cấu hình DMA1 Channel1 */
  DMA1_Channel1->CCR &= ~DMA_CCR_DIR; // 0: Read from peripheral
  DMA1_Channel1->CCR |= DMA_CCR_MEM2MEM; // 1: Memory to memory mode enabled
  DMA1_Channel1->CCR &= ~DMA_CCR_PL; // 00: Low priority
  DMA1_Channel1->CCR &= ~DMA_CCR_MSIZE; // Clear
  DMA1_Channel1->CCR |= DMA_CCR_MSIZE_1; //10: Memory size 32-bits
  DMA1_Channel1->CCR &= ~DMA_CCR_PSIZE; // Clear
  DMA1_Channel1->CCR |= DMA_CCR_PSIZE_1; //10: Periph size 32-bits
  DMA1_Channel1->CCR |= DMA_CCR_MINC; // 1: Memory increment mode enabled
  DMA1_Channel1->CCR |= DMA_CCR_PINC; // 1: Peripheral increment mode enabled
  DMA1_Channel1->CCR &= ~DMA_CCR_CIRC; // 0: Circular mode disabled
  DMA1_Channel1->CNDTR = (uint32_t)BUFFER_SIZE; // Buffer size
  DMA1_Channel1->CPAR = (uint32_t)&srcBuffer[0]; // &srcBuffer[0] or srcBuffer, periph address
  DMA1_Channel1->CMAR = (uint32_t)&dstBuffer[0]; // &dstBuffer[0] or dstBuffer, memory address

  /* Cho phép ngắt DMA1 Channel TC */
  DMA1_Channel1->CCR |= DMA_CCR_TCIE; // 1: TC interrupt enabled

  /* Bật DMA1 Channel1 */
  DMA1_Channel1->CCR |= DMA_CCR_EN; // 1: Channel enabled

  /* ___NVIC___ */

  /* Cấu hình mức độ ưu tiên ngắt DMA1 Channel1 */
  NVIC_SetPriority(DMA1_Channel1_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 0, 0));

  /* Cho phép ngắt DMA1 Channel1 */
  NVIC_EnableIRQ(DMA1_Channel1_IRQn);

  /* ___MAIN___ */

  /* Chờ cho đến khi cờ TC được set, tức là truyền thành công */
  while (tranferComplete == 0) { }

  /* Nếu Buffer nguồn (Flash) và Buffer đích (SRAM) giống nhau */
  if (buffer_compare((uint32_t *)srcBuffer, (uint32_t *)dstBuffer, BUFFER_SIZE) == 1)
  {
    /* Reset PC13 */
    GPIOC->BSRR |= GPIO_BSRR_BR13;
  }
  else
  {
    /* Set PC13 */
    GPIOC->BSRR |= GPIO_BSRR_BS13;
  }

  /* Hàm lặp vô hạn */
  while (1)
  {

  }
}

/* Hàm so sánh hai Buffer */
uint8_t buffer_compare(const uint32_t* pBuffer, uint32_t* pBuffer1, uint16_t bufferLength)
{
  while (bufferLength--)
  {
    if (*pBuffer != *pBuffer1)
    {
      return 0;
    }
    pBuffer++;
    pBuffer1++;
  }

  return 1;
}

/* Trình phục vụ ngắt DMA1 Channel 1 */
void DMA1_Channel1_IRQHandler(void)
{
  /* Nếu cờ TC được set và ngắt DMA1 Channel1 được cho phép */
  if (((DMA1->ISR & DMA_ISR_TCIF1) == DMA_ISR_TCIF1) && ((DMA1_Channel1->CCR & DMA_CCR_TCIE) == DMA_CCR_TCIE))
  {
    /* Truyền thành công */
    tranferComplete = 1;

    /* Xoá cờ DMA1 Channel1 HT, TC, TE và GL */
    DMA1->IFCR |= DMA_IFCR_CGIF1; // 1: Clears the GIF, TEIF, HTIF and TCIF flags in the DMA_ISR register
  }
}
