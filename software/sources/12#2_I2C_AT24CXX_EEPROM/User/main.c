#include "stm32f1xx.h"
#include "startup.h"
#include "delay.h"
#include "usart1.h"
#include "at24cxx.h"

#define BUFFER_SIZE 12
uint8_t txBuffer[BUFFER_SIZE] = {'H', 'e', 'l', 'l', 'o', ' ', 'W', 'o', 'r', 'l', 'd', '!'};
uint8_t rxBuffer[BUFFER_SIZE];

//uint8_t buffer = 0;
uint32_t prevMillis = 0;

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

  /* Khởi tạo at24cxx */
  at24cxx_init();

  /* ___MAIN___ */

  /* Ghi dữ liệu vào EEPROM */
  //at24cxx_writeByte(AT24C08_ADDR_PAGE1, 0x00, 0xAA);
  at24cxx_writePage(AT24C08_ADDR_PAGE1, 0x00, txBuffer, BUFFER_SIZE);

  /* Hàm lặp vô hạn */
  while (1)
  {
    /* Định thời 1000ms */
    if (millis() - prevMillis >= 1000)
    {
      /* Lưu lại thời điểm */
      prevMillis = millis();

      /* Đọc dữ liệu từ EEPROM */
      //at24cxx_readByte(AT24C08_ADDR_PAGE1, 0x00, &buffer);
      at24cxx_readPage(AT24C08_ADDR_PAGE1, 0x00, rxBuffer, BUFFER_SIZE);

      /* Gửi dữ liệu ra usart1 */
      for (uint8_t i = 0; i < BUFFER_SIZE; i++)
      {
        usart1_putChar(rxBuffer[i]);
      }
      usart1_putString((uint8_t *)"\r\n");
    }
  }
}
