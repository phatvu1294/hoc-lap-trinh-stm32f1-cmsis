#include "at24cxx.h"
#include "i2c1.h"

/* Hàm khởi tạo AT24CXX */
void at24cxx_init(void)
{
  /* Khởi tạo i2c1 */
  i2c1_init();
}

/* Hàm ghi 1 byte vào AT24CXX */
void at24cxx_writeByte(uint8_t address, uint8_t wordAddress, uint8_t data)
{
  i2c1_start();
  i2c1_addressDirection(address << 1, I2C_DIRECTION_TRANSMITTER);
  i2c1_transmit(wordAddress);
  i2c1_transmit(data);
  i2c1_stop();
}

/* Hàm đọc 1 byte từ AT24CXX */
void at24cxx_readByte(uint8_t address, uint8_t wordAddress, uint8_t *data)
{
  i2c1_start();
  i2c1_addressDirection(address << 1, I2C_DIRECTION_TRANSMITTER);
  i2c1_transmit(wordAddress);
  i2c1_start();
  i2c1_addressDirection(address << 1, I2C_DIRECTION_RECEIVER);
  *data = i2c1_receiveNack();
  i2c1_stop();
}

/* Hàm ghi nhiều byte từ AT24CXX */
void at24cxx_writePage(uint8_t address, uint8_t wordAddress, uint8_t *data, uint8_t size)
{
  uint8_t index = 0;
  i2c1_start();
  i2c1_addressDirection(address << 1, I2C_DIRECTION_TRANSMITTER);
  i2c1_transmit(wordAddress);
  while (index < size)
  {
    i2c1_transmit(data[index]);
    index++;
  }
  i2c1_stop();
}

/* Hàm đọc nhiều byte vào AT24CXX */
void at24cxx_readPage(uint8_t address, uint8_t wordAddress, uint8_t *data, uint8_t size)
{
  uint8_t index = 0;
  i2c1_start();
  i2c1_addressDirection(address << 1, I2C_DIRECTION_TRANSMITTER);
  i2c1_transmit(wordAddress);
  i2c1_start();
  i2c1_addressDirection(address << 1, I2C_DIRECTION_RECEIVER);
  while (index < size - 1)
  {
    data[index] = i2c1_receiveAck();
    index++;
  }
  data[index] = i2c1_receiveNack();
  i2c1_stop();
}
