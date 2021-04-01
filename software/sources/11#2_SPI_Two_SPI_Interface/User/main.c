#include "stm32f1xx.h"
#include "startup.h"
#include "delay.h"
#include "usart1.h"
#include <string.h>

/* SPI1 TX Only  ->  SPI2 RX Only Interrupt */
/* MOSI (PA7)   <->  MISO (PB14) */
/* SCK (PA5)    <->  SCK (PB13) */
/* PB0          <->  NSS (PB12) */

#define BUFFER_SIZE 14

uint8_t txBuffer[BUFFER_SIZE] = {'H', 'e', 'l', 'l', 'o', ' ', 'W', 'o', 'r', 'l', 'd', '!', '\r', '\n'};
uint8_t rxBuffer[BUFFER_SIZE];
uint8_t rxIndex = 0;
uint8_t rxComplete = 0;
uint32_t prevMillis = 0;

void spi1_transmit(uint8_t *pData, uint8_t dataSize);
void SPI2_IRQHandler(void);

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

  /* Bật Clock PortA và PortB */
  RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN; // 1: Enable

  /* Cấu hình PA5 (SPI1 CLK), PA7 (SPI1 MOSI) */
  GPIOA->CRL &= ~GPIO_CRL_MODE5 & ~GPIO_CRL_MODE7; // Clear;
  GPIOA->CRL |= GPIO_CRL_MODE5_1 | GPIO_CRL_MODE7_1; // 10: Output mode, max speed 2 MHz
  GPIOA->CRL &= ~GPIO_CRL_CNF5 & ~GPIO_CRL_CNF7; // Clear
  GPIOA->CRL |= GPIO_CRL_CNF5_1 | GPIO_CRL_CNF7_1; // 10: Alternate function output Push-pull

  /* Cấu hình PB0 (SPI1 NSS) */
  GPIOB->BSRR |= GPIO_BSRR_BS0; // Set PB0
  GPIOB->CRL &= ~GPIO_CRL_MODE0; // Clear;
  GPIOB->CRL |= GPIO_CRL_MODE0_1; // 10: Output mode, max speed 2 MHz
  GPIOB->CRL &= ~GPIO_CRL_CNF0; // 00: General purpose output push-pull

  /* Cấu hình PB14 (SPI2 MISO), PB12 (SPI2 NSS) */
  GPIOB->CRH &= ~GPIO_CRH_MODE14 & ~GPIO_CRH_MODE12; // 00: Input mode
  GPIOB->CRH &= ~GPIO_CRH_CNF14 & ~GPIO_CRH_CNF12; // Clear
  GPIOB->CRH |= GPIO_CRH_CNF14_0 | GPIO_CRH_CNF12_0; // 01: Floating input

  /* Cấu hình PB13 (SPI2 CLK) */
  GPIOB->CRH &= ~GPIO_CRH_MODE13; // Clear;
  GPIOB->CRH |= GPIO_CRH_MODE13_1; // 10: Output mode, max speed 2 MHz
  GPIOB->CRH &= ~GPIO_CRH_CNF13; // Clear;
  GPIOB->CRH |= GPIO_CRH_CNF13_1; // 10: Alternate function output Push-pull

  /* ___SPI___ */

  /* ___SPI1 TX Only___ */

  /* Bật Clock SPI1 */
  RCC->APB2ENR |= RCC_APB2ENR_SPI1EN; // 1: Enable

  /* Cấu hình SPI1 */
  SPI1->CR1 &= ~SPI_CR1_CPOL; // 0: CK to 0 when idle
  SPI1->CR1 &= ~SPI_CR1_CPHA; // 0: The first clock transition is the first data capture edge
  SPI1->CR1 |= SPI_CR1_MSTR; // 1: Master configuration
  SPI1->CR1 |= SPI_CR1_BR; // 111: fPCLK/256
  SPI1->CR1 &= ~SPI_CR1_LSBFIRST; // 0: MSB transmitted first

  /* Software slave management */
  /* Trong chế độ Master thì SSI bit phải được set bằng 1 */
  /* Trong chế độ Slave thì SSI đượng dùng thay thế chân NSS nếu SSM được sử dụng */
  SPI1->CR1 |= SPI_CR1_SSM; // 1: Software slave management enabled (NSS Pin replace with value of SSI bit)
  SPI1->CR1 |= SPI_CR1_SSI; // In "Master mode SSI forced set 1"

  /* Các chế độ SPI */
  /* BIDIMODE = 1 (1 Line)   | BIDIMODE = 0 (2 Line)   | */
  /* --------------------------------------------------- */
  /* BIDIOE = 0 | BIDIOE = 1 | RXONLY = 0 | RXONLY = 1 | */
  /* --------------------------------------------------- */
  /*  RX Only   |  TX Only   | Fullduplex |   RX Only  | */

  /* Cấu hình chế độ SPI1 1 Line TX Only */
  SPI1->CR1 |= SPI_CR1_BIDIMODE; // 1: 1-line bidirectional data mode selected
  SPI1->CR1 |= SPI_CR1_BIDIOE; // 1: Output enabled (transmit-only mode)
  SPI1->CR1 &= ~SPI_CR1_DFF; // 0: 8-bit data frame format is selected for transmission/reception
  SPI1->CR1 &= ~SPI_CR1_CRCEN; // 0: CRC calculation disabled
  SPI1->CRCPR = 0; // CRC polynomial register
  SPI1->I2SCFGR &= ~SPI_I2SCFGR_I2SMOD; // 0: SPI mode is selected

  /* Bật SPI1 */
  SPI1->CR1 |= SPI_CR1_SPE; // 1: Peripheral enabled

  /* ___SPI2 RX Only Interrupt___ */

  /* Bật Clock SPI2 */
  RCC->APB1ENR |= RCC_APB1ENR_SPI2EN; // 1: Enable

  /* Cấu hình SPI2 */
  SPI2->CR1 &= ~SPI_CR1_CPOL; // 0: CK to 0 when idle
  SPI2->CR1 &= ~SPI_CR1_CPHA; // 0: The first clock transition is the first data capture edge
  SPI2->CR1 &= ~SPI_CR1_MSTR; // 0: Slave configuration
  SPI2->CR1 |= SPI_CR1_BR; // 111: fPCLK/256
  SPI2->CR1 &= ~SPI_CR1_LSBFIRST; // 0: MSB transmitted first

  /* Software slave management */
  /* Trong chế độ Master thì SSI bit phải được set bằng 1 */
  /* Trong chế độ Slave thì SSI đượng dùng thay thế chân NSS nếu SSM được sử dụng */
  SPI2->CR1 &= ~SPI_CR1_SSM; // 0: Software slave management disabled
  //SPI2->CR1 |= SPI_CR1_SSI; // This bit has an effect only when the SSM bit is set

  /* Cấu hình chế độ SPI2 1 Line RX Only */
  SPI2->CR1 |= SPI_CR1_BIDIMODE; // 1: 1-line bidirectional data mode selected
  SPI2->CR1 &= ~SPI_CR1_BIDIOE; // 0: Output disabled (receive-only mode)
  SPI2->CR1 &= ~SPI_CR1_DFF; // 0: 8-bit data frame format is selected for transmission/reception
  SPI2->CR1 &= ~SPI_CR1_CRCEN; // 0: CRC calculation disabled
  SPI2->CRCPR = 0; // CRC polynomial register
  SPI2->I2SCFGR &= ~SPI_I2SCFGR_I2SMOD; // 0: SPI mode is selected

  /* Cho phép ngắt SPI2 RXNE */
  SPI2->CR2 |= SPI_CR2_RXNEIE; // 1: RXNE interrupt not masked. Used to generate an interrupt request when the RXNE flag is set

  /* Bật SPI2 */
  SPI2->CR1 |= SPI_CR1_SPE; // 1: Peripheral enabled

  /* ___NVIC___ */

  /* Cấu hình mức độ ưu tiên ngắt SPI2 */
  NVIC_SetPriority(SPI2_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 0, 0));

  /* Cho phép ngắt toàn cục SPI2 */
  NVIC_EnableIRQ(SPI2_IRQn);

  /* Hàm lặp vô hạn */
  while (1)
  {
    /* Định thời 100ms */
    if (millis() - prevMillis >= 1000)
    {
      /* Lưu thời điểm hiện tại */
      prevMillis = millis();

      /* Kéo chân NSS xuống mức thấp */
      GPIOB->BSRR |= GPIO_BSRR_BR0;

      /* Truyền dữ liệu ra SPI1 */
      spi1_transmit(txBuffer, BUFFER_SIZE);

      /* Kéo chân NSS lên mức cao */
      GPIOB->BSRR |= GPIO_BSRR_BS0; // Set PB0
    }

    /* Nếu nhận dữ liệu thành công */
    if (rxComplete == 1)
    {
      /* Gửi từng ký tự Buffer ra usart1 */
      for (int i = 0; i < BUFFER_SIZE; i++)
      {
        /* Gửi ký tự ra usart 1 */
        usart1_putChar(rxBuffer[i]);
      }

      /* Xóa Buffer, reset biến đếm và cho phép ngắt SPI1 RXNE */
      memset(rxBuffer, 0, BUFFER_SIZE);
      rxComplete = 0;
      rxIndex = 0;
      SPI2->CR2 |= SPI_CR2_RXNEIE;
    }
  }
}

/* Hàm gửi dữ liệu ra SPI1 */
void spi1_transmit(uint8_t *pData, uint8_t dataSize)
{
  /* Biến đếm Tx */
  uint8_t txIndex = 0;

  /* Nếu kích thước dữ liệu gửi khác 0 */
  while (dataSize > 0)
  {
    /* Chờ cho đến khi SPI1 TX data rỗng (TXE) */
    while ((SPI1->SR & SPI_SR_TXE) != SPI_SR_TXE);

    /* Gửi dữ liệu ra SPI1 */
    SPI1->DR = pData[txIndex];

    /* Tăng giảm biến đếm */
    txIndex++;
    dataSize--;
  }

  /* Chờ cho đến khi SPI1 không bận */
  while ((SPI1->SR & SPI_SR_BSY) == SPI_SR_BSY);
}

/* Trình phục vụ ngắt SPI2 */
void SPI2_IRQHandler(void)
{
  /* Nếu cờ SPI2 RXNE được set và ngắt SPI2 RXNE được cho phép */
  if (((SPI2->SR & SPI_SR_RXNE) == SPI_SR_RXNE) && ((SPI2->CR2 & SPI_CR2_RXNEIE) == SPI_CR2_RXNEIE))
  {
    /* Đọc dữ liệu nhận được từ SPI2 */
    rxBuffer[rxIndex] = SPI2->DR;

    /* Tăng biến đệm RX */
    rxIndex++;

    /* Nếu biến đếm bằng kích thước bộ đệm */
    if (rxIndex == BUFFER_SIZE)
    {
      /* Nhận dữ liệu thành công */
      rxComplete = 1;

      /* Cấm ngắt SPI2 RXNE */
      SPI2->CR2 &= ~SPI_CR2_RXNEIE;
    }

    /* Xóa cờ SPI2 RXNE */
    /* Cờ RXNE được xóa bời hardware */
  }
}
