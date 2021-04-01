#include "stm32f1xx.h"
#include "startup.h"
#include "pcf8574lcd.h"

int main(void)
{
  /* Thêm chương trình chính tại đây */

  /* ___LIB___ */

  /* Khởi tạo hệ thống */
  startup_init();

  /* Khởi tạo pcf8574lcd */
  pcf8574lcd_init();

  /* ___MAIN___ */

  /* Bật đèn nền */
  pcf8574lcd_backlight(1);

  /* Đặt tọa độ LCD */
  pcf8574lcd_setPos(1, 0);

  /* Gửi chuỗi ra màn hình LCD */
  pcf8574lcd_string("STM32F103C8T6");

  /* Đặt tọa độ LCD */
  pcf8574lcd_setPos(1, 1);

  /* Gửi chuỗi ra màn hình LCD */
  pcf8574lcd_string("ARM Cortex M3");

  /* Hàm lặp vô hạn */
  while (1)
  {

  }
}


