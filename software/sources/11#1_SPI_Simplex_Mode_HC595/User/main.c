#include "stm32f1xx.h"
#include "startup.h"
#include "delay.h"

/* SPI1 TX Only  ->  HC595 */
/* MOSI (PA7)   <->  DS (14) */
/* SCK (PA5)    <->  SHCP (11) */
/* PB0          <->  STCP (12) */

uint8_t txBuffer = 0;
uint32_t prevMillis = 0;

void spi1_transmit(uint8_t *pData, uint8_t dataSize);

int main(void)
{
  /* Thêm chương trình chính tại đây */

  /* ___LIB___ */

  /* Khởi tạo hệ thống */
  startup_init();

  /* Khởi tạo delay */
  delay_init();

  /* ___GPIO___ */

  /* Bật Clock PortA và PortB */
  RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN; // 1: Enable

  /* Cấu hình PA5 (SPI1 CLK), PA7 (SPI1 MOSI) */
  GPIOA->CRL &= ~GPIO_CRL_MODE5 & ~GPIO_CRL_MODE7; // Clear;
  GPIOA->CRL |= GPIO_CRL_MODE5_1 | GPIO_CRL_MODE7_1; // 10: Output mode, max speed 2 MHz
  GPIOA->CRL &= ~GPIO_CRL_CNF5 & ~GPIO_CRL_CNF7; // Clear
  GPIOA->CRL |= GPIO_CRL_CNF5_1 | GPIO_CRL_CNF7_1; // 10: Alternate function output Push-pull

  /* Cấu hình PB0 (HC595 Latch) */
  GPIOB->BSRR |= GPIO_BSRR_BR0; // Reset PB0
  GPIOB->CRL &= ~GPIO_CRL_MODE0; // Clear;
  GPIOB->CRL |= GPIO_CRL_MODE0_1; // 10: Output mode, max speed 2 MHz
  GPIOB->CRL &= ~GPIO_CRL_CNF0; // 00: General purpose output push-pull

  /* ___SPI___ */

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

  /* Hàm lặp vô hạn */
  while (1)
  {
    /* Định thời 200ms */
    if (millis() - prevMillis >= 200)
    {
      /* Lưu thời điểm hiện tại */
      prevMillis = millis();

      /* Chốt dữ liệu HC595 */
      GPIOB->BSRR |= GPIO_BSRR_BR0; // Reset PB0

      /* Gửi dữ liệu ra SPI1 */
      spi1_transmit(&txBuffer, 1);

      /* Chốt dữ liệu HC595 */
      GPIOB->BSRR |= GPIO_BSRR_BS0; // Set PB0

      /* Tăng và kiểm tra buffer nếu quá thì đặt lại */
      if (++txBuffer > 255) txBuffer = 0;
    }
  }
}

/* Hàm gửi dữ liệu ra SPI1 */
void spi1_transmit(uint8_t *pData, uint8_t dataSize)
{
  /* Biến đếm Tx */
  uint8_t txIndex = 0;

  /* Nếu kích thước dữ liệu gửi khác 0 */
  while (txIndex < dataSize)
  {
    /* Chờ cho đến khi SPI1 TX data rỗng (TXE) */
    while ((SPI1->SR & SPI_SR_TXE) != SPI_SR_TXE);

    /* Gửi dữ liệu ra SPI1 */
    SPI1->DR = pData[txIndex];

    /* Tăng giảm biến đếm */
    txIndex++;
  }

  /* Chờ cho đến khi SPI1 không bận */
  while ((SPI1->SR & SPI_SR_BSY) == SPI_SR_BSY);
}
