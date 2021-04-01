#include "pcf8574lcd.h"
#include "delay.h"
#include "i2c1.h"

/* Định nghĩa chân của PCF8574LCD private */
#define PCF8574LCD_BL (1 << 3)
#define PCF8574LCD_RS (1 << 0)
#define PCF8574LCD_RW (1 << 1)
#define PCF8574LCD_EN (1 << 2)
#define PCF8574LCD_D4 (1 << 4)
#define PCF8574LCD_D5 (1 << 5)
#define PCF8574LCD_D6 (1 << 6)
#define PCF8574LCD_D7 (1 << 7)

/* Biến private */
uint8_t __lcdPinData = 0;

/* Nguyên mẫu hàm private */
void pcf8574lcd_write(uint8_t address, uint8_t data);
void pcf8574lcd_delay(void);
void pcf8574lcd_write4Bits(uint8_t nb);
void pcf8574lcd_pulseEnable(void);

/* Hàm gửi dữ liệu ra PCF8574LCD */
void pcf8574lcd_write(uint8_t address, uint8_t data)
{
  i2c1_start();
  i2c1_addressDirection(address << 1, I2C_DIRECTION_TRANSMITTER);
  i2c1_transmit(data);
  i2c1_stop();
}

/* Hàm tạo trễ LCD */
void pcf8574lcd_delay(void)
{
  for (uint8_t i = 0; i < 255; i++) __NOP();
}

/* Hàm tạo xung chốt tại chân EN */
void pcf8574lcd_pulseEnable(void)
{
  /* Tạo xung L-H-L chốt dữ liệu */
  __lcdPinData &= ~PCF8574LCD_EN;
  pcf8574lcd_write(PCF8574LCD_ADDR, __lcdPinData);
  pcf8574lcd_delay();

  __lcdPinData |= PCF8574LCD_EN;
  pcf8574lcd_write(PCF8574LCD_ADDR, __lcdPinData);
  pcf8574lcd_delay();

  __lcdPinData &= ~PCF8574LCD_EN;
  pcf8574lcd_write(PCF8574LCD_ADDR, __lcdPinData);
  pcf8574lcd_delay();
}

/* Hàm gửi dữ liệu 4 bit ra các chân dữ liệu D4 - D7 */
void pcf8574lcd_write4Bits(uint8_t nb)
{
  /* Lấy bit D0 gửi ra chân D4 */
  if (((nb >> 0) & 0x01) == 1) { __lcdPinData |= PCF8574LCD_D4; }
  else { __lcdPinData &= ~PCF8574LCD_D4; }

  /* Lấy bit D1 gửi ra chân D5 */
  if (((nb >> 1) & 0x01) == 1) { __lcdPinData |= PCF8574LCD_D5; }
  else { __lcdPinData &= ~PCF8574LCD_D5; }

  /* Lấy bit D2 gửi ra chân D6 */
  if (((nb >> 2) & 0x01) == 1) { __lcdPinData |= PCF8574LCD_D6; }
  else { __lcdPinData &= ~PCF8574LCD_D6; }

  /* Lấy bit D3 gửi ra chân D7 */
  if (((nb >> 3) & 0x01) == 1) { __lcdPinData |= PCF8574LCD_D7; }
  else { __lcdPinData &= ~PCF8574LCD_D7; }
  
  /* Gửi 4 bit ra LCD */
  pcf8574lcd_write(PCF8574LCD_ADDR, __lcdPinData);

  /* Tạo xung chốt dữ liệu */
  pcf8574lcd_pulseEnable();
}

/* Hàm gửi lệnh ra LCD */
void pcf8574lcd_command(uint8_t cmd)
{
  /* Chế độ gửi lệnh và ghi */
  __lcdPinData &= ~PCF8574LCD_RS;
  __lcdPinData &= ~PCF8574LCD_RW;
  pcf8574lcd_write(PCF8574LCD_ADDR, __lcdPinData);

  /* Ghi 2 lần 4 bit ra LCD */
  pcf8574lcd_write4Bits(cmd >> 4);
  pcf8574lcd_write4Bits(cmd);
}

/* Hàm gửi dữ liệu ra LCD */
void pcf8574lcd_data(uint8_t dt)
{
  /* Chế độ gửi dữ liệu và ghi */
  __lcdPinData |= PCF8574LCD_RS;
  __lcdPinData &= ~PCF8574LCD_RW;
  pcf8574lcd_write(PCF8574LCD_ADDR, __lcdPinData);

  /* Ghi 2 lần 4 bit ra LCD */
  pcf8574lcd_write4Bits(dt >> 4);
  pcf8574lcd_write4Bits(dt);
}

/* Hàm gửi ký tự ra LCD */
void pcf8574lcd_char(char chr)
{
  /* Gửi ký tự ra LCD */
  pcf8574lcd_data((uint8_t) chr);
}

/* Hàm gửi chuỗi ra LCD */
void pcf8574lcd_string(char * str)
{
  /* Biến đếm */
  uint8_t i = 0;

  /* Trong khi ký tự khác '\0' */
  while (str[i] != 0)
  {
    /* Gửi ký tự tương ứng ra LCD */
    pcf8574lcd_char(str[i]);

    /* Tăng biến đếm */
    i++;
  }
}

/* Hàm đặt vị trí con trỏ LCD theo toạ độ x và  y */
void pcf8574lcd_setPos(uint8_t x, uint8_t y)
{
  /* Mảng lưu trữ vị trí đầu tiên của các dòng */
  uint8_t firstChar[] = {0x80, 0xC0, 0x94, 0xD4};

  /* Gửi lệnh đưa con trỏ đến vị trí đặt */
  pcf8574lcd_command(firstChar[y] + x);
}

/* Hàm xoá màn hình LCD */
void pcf8574lcd_clear(void)
{
  /* Gửi lệnh xoá màn hình */
  pcf8574lcd_command(0x01);

  /* Tạo trễ tối thiếu */
  delay_ms(2);
}

/* Hàm tạo ký tự tự tạo */
void pcf8574lcd_createChar(uint8_t loc, uint8_t charmap[])
{
  /* Có tất cả 8 vị trí */
  loc &= 0x7;

  /* Gửi lệnh tạo ký tự LCD */
  pcf8574lcd_command(0x40 | (loc << 3));

  /* Ghi ký tự ra CGRAM */
  for (int i = 0; i < 8; i++)
  {
    pcf8574lcd_data(charmap[i]);
  }
}

/* Hàm khởi tạo LCD */
void pcf8574lcd_init(void)
{
  /* Khởi tạo delay */
  delay_init();

  /* Khởi tạo i2c1 */
  i2c1_init();
  
  /* Tạo trễ 1 khoảng thời gian */
  delay_ms(50);

  /* Đặt lại các chân */
  __lcdPinData &= ~PCF8574LCD_BL;
  __lcdPinData &= ~PCF8574LCD_RS;
  __lcdPinData &= ~PCF8574LCD_RW;
  __lcdPinData &= ~PCF8574LCD_EN;
  pcf8574lcd_write(PCF8574LCD_ADDR, __lcdPinData);

  /* Gửi các lệnh khởi tạo LCD 4 bit */
  pcf8574lcd_write4Bits(0x03);
  delay_ms(5);
  pcf8574lcd_write4Bits(0x03);
  delay_ms(5);
  pcf8574lcd_write4Bits(0x03);
  delay_ms(1);
  pcf8574lcd_write4Bits(0x02);
  pcf8574lcd_command(0x28);
  pcf8574lcd_command(0x0C);
  pcf8574lcd_clear();
  pcf8574lcd_command(0x06);
}

/* Hàm bật đèn nền HC595 LCD */
void pcf8574lcd_backlight(uint8_t enable)
{
  if (enable)
    __lcdPinData |= PCF8574LCD_BL;
  else
    __lcdPinData &= ~PCF8574LCD_BL;
  pcf8574lcd_write(PCF8574LCD_ADDR, __lcdPinData);
}

