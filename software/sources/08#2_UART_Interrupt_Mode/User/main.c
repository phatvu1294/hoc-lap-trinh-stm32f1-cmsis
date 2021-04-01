#include "stm32f1xx.h"
#include "startup.h"
#include "string.h"

uint8_t rxBuffer[128];
uint8_t rxIndex = 0;
uint8_t rxComplete = 0;

void usart1_putChar(uint8_t c);
void usart1_putString(uint8_t *s);
void USART1_IRQHandler(void);

int main(void)
{
  /* Thêm chương trình chính tại đây */

  /* ___LIB___ */

  /* Khởi tạo hệ thống */
  startup_init();

  /* ___GPIO___ */

  /* Bật Clock PortA */
  RCC->APB2ENR |= RCC_APB2ENR_IOPAEN; // 1: Enable

  /* Cấu hình PA10 (RX) */
  GPIOA->CRH &= ~GPIO_CRH_MODE10; // 00: Input mode
  GPIOA->CRH &= ~GPIO_CRH_CNF10; // Clear;
  GPIOA->CRH |= GPIO_CRH_CNF10_0; // 01: Floating input

  /* Cấu hình PA9 (TX) */
  GPIOA->CRH &= ~GPIO_CRH_MODE9; // Clear;
  GPIOA->CRH |= GPIO_CRH_MODE9_1; // 10: Output mode, max speed 2 MHz
  GPIOA->CRH &= ~GPIO_CRH_CNF9; // Clear;
  GPIOA->CRH |= GPIO_CRH_CNF9_1; // 10: Alternate function output Push-pull

  /* ___AFIO___ */

  /* Bật Clock AFIO để remap USART1, xem tài liệu TX:PA9/PB6(remap), RX:PA10/PB7(remap) */
  /* Phải cấu hình lại chân GPIO về PB6 và PB7 */
  //RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;
  //AFIO->MAPR |= AFIO_MAPR_USART1_REMAP;

  /* ___USART___ */

  /* Bật Clock USART1 */
  RCC->APB2ENR |= RCC_APB2ENR_USART1EN; // 1: Enable

  /* Cấu hình USART1 */
  USART1->CR1 &= ~USART_CR1_M; // 0: 1 Start bit, 8 Data bits, n Stop bit
  USART1->CR1 &= ~USART_CR1_PCE; // 0: Parity control disabled
  USART1->CR1 |= USART_CR1_TE; // 1: Transmitter is enabled
  USART1->CR1 |= USART_CR1_RE; // 1: Receiver is enabled and begins searching for a start bit
  USART1->CR2 &= ~USART_CR2_STOP; // 00: 1 Stop bit
  USART1->CR3 &= ~USART_CR3_CTSE; // 0: CTS hardware flow control disabled

  /* Tx/Rx Baud = fCK/(8*(2-OVER8)*USARTDIV) */
  /* Khi OVER8 = 0, phần fractional được lập trình bởi 4 bit DIV_fraction[3:0] trong thanh ghi USART_BRR */
  /* Khi OVER8 = 1, phần fractional được lập trình bởi 3 bit DIV_fraction[2:0] trong thanh ghi USART_BRR, bit DIV_fraction[3] phải được clear (=0) */

  /* Ví dụ 1: fCK = 72000000, Baudrate = 9600, OVER8=0 */
  /* USARTDIV = (fCK/Baudrate)/16 */
  /* USARTDIV = (72000000/9600)/16 = 468.75 */
  /* DIV_Mantissa = 468, DIV_Fraction = 0.75 * (2^4) = 12 */
  /* USART1->BRR = (468 << 4) | 12; */

  /* Ví dụ 2: fCK = 72000000, Baudrate = 57600, OVER8=1 */
  /* USARTDIV = (fCK/Baudrate)/8 */
  /* USARTDIV = (72000000/57600)/8 = 156.25 */
  /* DIV_Mantissa = 156, DIV_Fraction = 0.25 * (2^3) = 2 */
  /* USART1->BRR = (156 << 3) | 2; */

  /* STM32F103C8T6, không có bit OVER8 trong USART1->CR1, nên OVER8=0 */
  /* =>Tx/Rx Baud = fCK/(16*USARTDIV) */
  /* fCK - Input clock to the peripheral (PCLK1 for USART2, 3, 4, 5 or PCLK2 for USART1) */
  /* PCLK2 = HCLK = SYSCLK = 72000000 (stm32f1xx.h), theo ví dụ 1 thì */
  USART1->BRR = (468 << 4) | 12;
  
  /* Cho phép ngắt USART1 RXNE */
  USART1->CR1 |= USART_CR1_RXNEIE; // 1: A USART interrupt is generated whenever ORE=1 or RXNE=1 in the USART_SR register

  /* Bật USART1 */
  USART1->CR1 |= USART_CR1_UE; // 1: USART enabled
  
  /* ___NVIC___ */
  
  /* Cấu hình mức độ ưu tiên ngắt USART1 */
  NVIC_SetPriority(USART1_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 0, 0));
  
  /* Cho phép ngắt toàn cục USART1 */
  NVIC_EnableIRQ(USART1_IRQn);

  /* Hàm lặp vô hạn */
  while (1)
  {
    /* Nếu nhận dữ liệu thành công */
    if (rxComplete == 1)
    {
      /* Gửi chuỗi (line) nhận được ra usart1 */
      usart1_putString(rxBuffer);

      /* Xóa Buffer, reset biến đếm, cho phép ngắt USART RXNE */
      memset(rxBuffer, 0, sizeof(rxBuffer));
      rxComplete = 0;
      rxIndex = 0;
      USART1->CR1 |= USART_CR1_RXNEIE;
    }
  }
}

/* Hàm gửi ký tự ra USART1 */
void usart1_putChar(uint8_t c)
{
  /* Gửi ký tự nhận được ra USART1 */
  USART1->DR = c & 0x01FF; // Thanh ghi USART_DR 9 bit

  /* Chờ cho đến khi thanh ghi USART1 data rỗng */
  /* Cờ TXE = 1: USART1 truyền thành công và USART1 data lúc này rỗng */
  while ((USART1->SR & USART_SR_TXE) != USART_SR_TXE);
}

/* Hàm gửi chuỗi ra USART1 */
void usart1_putString(uint8_t *s)
{
  uint8_t i = 0;

  /* Nếu không phải ký tự null */
  while (s[i] != 0)
  {
    /* Gửi ký tự */
    usart1_putChar(s[i++]);
  }
}

/* Trình phục vụ ngắt USART1 */
void USART1_IRQHandler(void)
{
  /* Nếu cờ RXNE = 1 và ngắt RXNE được cho phép */
  if (((USART1->SR & USART_SR_RXNE) == USART_SR_RXNE) && ((USART1->CR1 & USART_CR1_RXNEIE) == USART_CR1_RXNEIE))
  {
    /* Lưu dữ liệu USART1 vào rxChar */
    uint8_t rxChar = USART1->DR & 0x1FF; // USART_DR 9 bit

    /* Thêm dồn rxChar vào rxBuffer */
    rxBuffer[rxIndex++] = rxChar;

    /* Nếu rxChar là ký tự xuống dòng */
    if (rxChar == '\n')
    {
      /* Nhận thành công */
      rxComplete = 1;

      /* Cấm ngắt USART1 RXNE */
      USART1->CR1 &= ~USART_CR1_RXNEIE;
    }

    /* Xóa cờ RXNE */
    USART1->SR &= ~USART_SR_RXNE;
  }
}
