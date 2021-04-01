#include "stm32f1xx.h"
#include "startup.h"
#include "usart1.h"
#include "delay.h"

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

  /* Hàm lặp vô hạn */
  while (1)
  {
    /* Gửi chuỗi ra USART1 */
    usart1_putString((uint8_t *)"Hello World!\r\n");
    delay_ms(1000);
  }
}
