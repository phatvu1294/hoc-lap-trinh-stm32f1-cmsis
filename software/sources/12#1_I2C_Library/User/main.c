#include "stm32f1xx.h"
#include "startup.h"
#include "delay.h"
#include "usart1.h"
#include "i2c1.h"

/* Địa chỉ các Page */
#define AT24C08_ADDR_PAGE1 0x50
#define AT24C08_ADDR_PAGE2 0x51
#define AT24C08_ADDR_PAGE3 0x52
#define AT24C08_ADDR_PAGE4 0x53

/* Kích thước bộ đệm */
#define BUFFER_SIZE 12

uint8_t txBuffer[BUFFER_SIZE] = {'H', 'e', 'l', 'l', 'o', ' ', 'W', 'o', 'r', 'l', 'd', '!'};
uint8_t rxBuffer[BUFFER_SIZE];
//uint8_t buffer = 0;
uint32_t prevMillis = 0;

/* Hàm đọc ghi byte vào AT24C08 */
void eeprom_writeByte(uint8_t address, uint8_t wordAddress, uint8_t data);
void eeprom_readByte(uint8_t address, uint8_t wordAddress, uint8_t *data);

/* Hàm đọc ghi nhiều byte vào AT24C08 */
void eeprom_writePage(uint8_t address, uint8_t wordAddress, uint8_t *data, uint8_t size);
void eeprom_readPage(uint8_t address, uint8_t wordAddress, uint8_t *data, uint8_t size);

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
  
  /* Khởi tạo i2c1 */
  i2c1_init();

  /* ___MAIN___ */

  /* Ghi dữ liệu vào EEPROM */
  //eeprom_writeByte(AT24C08_ADDR_PAGE1, 0x00, 0xAA);
  eeprom_writePage(AT24C08_ADDR_PAGE1, 0x00, txBuffer, BUFFER_SIZE);

  /* Hàm lặp vô hạn */
  while (1)
  {
    /* Định thời 1000ms */
    if (millis() - prevMillis >= 1000)
    {
      /* Lưu lại thời điểm */
      prevMillis = millis();

      /* Đọc dữ liệu từ EEPROM */
      //eeprom_readByte(AT24C08_ADDR_PAGE1, 0x00, &buffer);
      eeprom_readPage(AT24C08_ADDR_PAGE1, 0x00, rxBuffer, BUFFER_SIZE);

      /* Gửi dữ liệu ra usart1 */
      for (uint8_t i = 0; i < BUFFER_SIZE; i++)
      {
        usart1_putChar(rxBuffer[i]);
      }
      usart1_putString((uint8_t *)"\r\n");
    }
  }
}

/* Hàm ghi byte vào AT24C08 */
void eeprom_writeByte(uint8_t address, uint8_t wordAddress, uint8_t data)
{
  i2c1_start();
  i2c1_addressDirection(address << 1, I2C_DIRECTION_TRANSMITTER);
  i2c1_transmit(wordAddress);
  i2c1_transmit(data);
  i2c1_stop();
}

/* Hàm đọc byte từ AT24C08 */
void eeprom_readByte(uint8_t address, uint8_t wordAddress, uint8_t *data)
{
  i2c1_start();
  i2c1_addressDirection(address << 1, I2C_DIRECTION_TRANSMITTER);
  i2c1_transmit(wordAddress);
  i2c1_start();
  i2c1_addressDirection(address << 1, I2C_DIRECTION_RECEIVER);
  *data = i2c1_receiveNack();
  i2c1_stop();
}

/* Hàm đọc nhiều byte từ AT24C08 */
void eeprom_writePage(uint8_t address, uint8_t wordAddress, uint8_t *data, uint8_t size)
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

/* Hàm ghi nhiều byte vào AT24C08 */
void eeprom_readPage(uint8_t address, uint8_t wordAddress, uint8_t *data, uint8_t size)
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
