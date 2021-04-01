#include "i2c1.h"

/* Nguyên mẫu hàm private */
uint8_t i2c1_checkEvent(uint32_t evt);

/* Hàm khởi tạo I2C1 */
void i2c1_init(void)
{
  /* ___I2C___ */

  /* Bật Clock I2C1 */
  RCC->APB1ENR |= RCC_APB1ENR_I2C1EN; // 1: Enable

  /* Cấu hình I2C1 */

  /* I2C Mode */
  /*  SMBUS=0 |            SMBUS=1        */
  /* ------------------------------------ */
  /* SMTYPE=x |  SMBTYPE=0   | SMBTYPE=1  */
  /* ------------------------------------ */
  /*    I2C   | SMBus Device | SMBus Host */

  uint16_t result = 0x04;
  uint32_t pClk1 = 36000000;
  uint16_t freqRange = pClk1 / 1000000;

  uint8_t i2cDutyCycle = I2C_DUTY_CYCLE_2;
  uint32_t i2cClockSpeed = 100000;

  I2C1->CR1 &= I2C_CR1_PE; // Disable Periph
  I2C1->CR1 &= ~I2C_CR1_SMBTYPE; // Clear
  I2C1->CR1 &= ~I2C_CR1_SMBUS; // I2C mode
  I2C1->CR2 &= ~I2C_CR2_FREQ; // Peripheral clock frequency
  I2C1->CR2 |= freqRange; // Peripheral clock frequency = 36MHz
  I2C1->CR1 &= ~I2C_CR1_ACK; // 0: No acknowledge returned, 1: Acknowledge returned after a byte is received (matched address or data)
  I2C1->CCR &= ~I2C_CCR_DUTY; // Clear
  I2C1->OAR1 &= ~I2C_OAR1_ADDMODE; //0: 7-bit slave address (10-bit address not acknowledged)
  I2C1->OAR1 |= 0x00; // Slave Interface address

  /* Standard Mode (sm) */
  if (i2cClockSpeed <= 100000)
  {
    /* Thigh = CCR * TPCLK1 */
    /* Tlow = CCR * TPCLK1 */
    result = pClk1 / (i2cClockSpeed * 2);

    /* Giá trị tối thiểu là 0x04 */
    if ((result & 0x0FFF) < 0x04)
    {
      result = 0x04;
    }

    I2C1->CCR &= ~I2C_CCR_FS; // standard mode
    I2C1->CCR |= result ; // Clock control register in Fm/Sm mode (Master mode)
    I2C1->TRISE = freqRange + 1; // Maximum rise time in Fm/Sm mode (Master mode)
  }
  /* Fast Mode (fm) */
  else
  {
    /* Nếu tlow/thigh = 2 */
    /* Thigh = CCR * TPCLK1 */
    /* Tlow = 2 * CCR * TPCLK1 */
    if (i2cDutyCycle == I2C_DUTY_CYCLE_2)
    {
      result = pClk1 / (i2cClockSpeed * 3);
      I2C1->CCR &= ~I2C_CCR_DUTY; // 0: Fm mode tlow/thigh = 2
    }

    /* Nếu tlow/thigh = 16/9 */
    /* Thigh = 9 * CCR * TPCLK1 */
    /* Tlow = 16 * CCR * TPCLK1 */
    else if (i2cDutyCycle == I2C_DUTY_CYCLE_16_9)
    {
      result = pClk1 / (i2cClockSpeed * 25);
      I2C1->CCR |= I2C_CCR_DUTY; // 1: Fm mode tlow/thigh = 16/9
    }

    /* Nếu giá trị CCR < 1*/
    if ((result & 0x0FFF) < 0x01) // [11:0]I2C->CCR_CCR
    {
      /* Đặt giá trị tối thiểu */
      result |= 0x01;
    }

    I2C1->CCR |= I2C_CCR_FS; // fast mode
    I2C1->CCR |= result; // Clock control register in Fm/Sm mode (Master mode)
    I2C1->TRISE = ((freqRange * 300) / 1000) + 1; // Maximum rise time in Fm/Sm mode (Master mode)
  }

  /* Bật I2C1 */
  I2C1->CR1 |= I2C_CR1_PE; // 1: Peripheral enable

  /* ___GPIO___ */

  /* Bật Clock PortB */
  RCC->APB2ENR |= RCC_APB2ENR_IOPBEN; // 1: Enable

  /* Cấu hình PB7 (I2C1 SDA), PB6 (I2C1 SCL) */
  GPIOB->CRL &= ~GPIO_CRL_MODE6 & ~GPIO_CRL_MODE7; // Clear;
  GPIOB->CRL |= GPIO_CRL_MODE6_1 | GPIO_CRL_MODE7_1; // 10: Output mode, max speed 2 MHz
  GPIOB->CRL |= GPIO_CRL_CNF6 | GPIO_CRL_CNF7; // 11: Alternate function output Open-drain
}

/* Hàm tạo điều kiện START */
void i2c1_start(void)
{
  /* Tạo điều kiện START */
  I2C1->CR1 |= I2C_CR1_START; // 1: Repeated start generation

  /* Kiểm tra cờ BUSY, MSL and SB flag */
  while (!i2c1_checkEvent(I2C_EVENT_MASTER_MODE_SELECT));
}

/* Hàm tạo điều kiện STOP */
void i2c1_stop(void)
{
  /* Tạo điều khiện STOP */
  I2C1->CR1 |= I2C_CR1_STOP; // 1: Stop generation after the current byte transfer or after the current Start condition is sent.

  /* Chờ cho đến khi điều kiện STOP được kết thúc */
  while ((I2C1->SR1 & I2C_SR1_STOPF) == I2C_SR1_STOPF);
}

/* Hàm gửi địa chỉ và điều hướng bus */
void i2c1_addressDirection(uint8_t address, uint8_t direction)
{
  /* Gửi địa chỉ I2C */
  if (direction == I2C_DIRECTION_TRANSMITTER)
  {
    /* Kéo địa chỉ bit 0 xuống mức thấp để ghi */
    address &= ~(1 << 0);
  }
  else if (direction == I2C_DIRECTION_RECEIVER)
  {
    /* Kéo địa chỉ bit 0 lên mức cao để đọc */
    address |= (1 << 0);
  }

  /* Đặt địa chỉ */
  I2C1->DR = address;

  /* Chờ I2C EV6, nghĩa là slave đã chấp nhận địa chỉ */
  if (direction == I2C_DIRECTION_TRANSMITTER)
  {
    while (!i2c1_checkEvent(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));
  }
  else if (direction == I2C_DIRECTION_RECEIVER)
  {
    while (!i2c1_checkEvent(I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));
  }
}

/* Hàm truyền dữ liệu */
void i2c1_transmit(uint8_t byte)
{
  /* Gửi dữ liệu I2C */
  I2C1->DR = byte;

  /* Chờ I2C EV8_2, nghĩa là dữ liệu đã được chuyển ra ngoài trên đường bus */
  while (!i2c1_checkEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED));
}

/* Hàm nhận dữ liệu ACK */
uint8_t i2c1_receiveAck(void)
{
  /* Cho phép ACK để nhận dữ liệu */
  I2C1->CR1 |= I2C_CR1_ACK;

  /* Chờ I2C EV7, nghĩa là dữ liệu đã nhận được trong thanh ghi i2c */
  while (!i2c1_checkEvent(I2C_EVENT_MASTER_BYTE_RECEIVED));

  /* Trả về dữ liệu nhận được từ thanh ghi */
  return I2C1->DR;
}

/* Hàm nhận dữ liệu NACK */
uint8_t i2c1_receiveNack(void)
{
  /* Tắt ACK để nhận dữ liệu */
  I2C1->CR1 &= ~I2C_CR1_ACK;

  /* Chờ I2C EV7, nghĩa là dữ liệu đã nhận được trong thanh ghi i2c */
  while (!i2c1_checkEvent(I2C_EVENT_MASTER_BYTE_RECEIVED));

  /* Trả về dữ liệu nhận được từ thanh ghi */
  return I2C1->DR;
}

/* Hàm demo ghi dữ liệu */
void i2c1_write(uint8_t address, uint8_t data)
{
  i2c1_start();
  i2c1_addressDirection(address << 1, I2C_DIRECTION_TRANSMITTER);
  i2c1_transmit(data);
  i2c1_stop();
}

/* Hàm demo đọc dữ liệu */
void i2c1_read(uint8_t address, uint8_t *data)
{
  i2c1_start();
  i2c1_addressDirection(address << 1, I2C_DIRECTION_RECEIVER);
  *data = i2c1_receiveNack();
  i2c1_stop();
}

/* Hàm kiểm tra sự kiện */
uint8_t i2c1_checkEvent(uint32_t evt)
{
  /* Sự kiện được hợp thành từ 2 thanh ghi */
  uint32_t flag1 = I2C1->SR1;
  uint32_t flag2 = I2C1->SR2;
  uint32_t flag = (flag2 << 16) | flag1;

  /* Nếu sự kiện bằng thì trả về 1 */
  if ((flag & evt) == evt)
  {
    return 1;
  }
  /* Ngược lại trả về 0 */
  else
  {
    return 0;
  }
}
