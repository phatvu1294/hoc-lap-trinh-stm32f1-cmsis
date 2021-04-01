#include "stm32f1xx.h"
#include "startup.h"
#include "delay.h"
#include "usart1.h"
#include "mpu6050.h"
#include <stdio.h>

char str[128];

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

  /* Khởi tạo mpu6050 */
  mpu6050_init(MPU6050_SCALE_2000DPS, MPU6050_RANGE_2G);

  /* ___MAIN___ */

  /* Trạng thái ngủ */
  sprintf(str, "Sleep Mode: %s\r\n", mpu6050_getSleepEnabled() ? "Enabled" : "Disabled");
  usart1_putString((uint8_t *)str);

  /* Nguồn Clock */
  usart1_putString((uint8_t *)"Clock Source: ");
  switch (mpu6050_getClockSource())
  {
  case MPU6050_CLOCK_KEEP_RESET:
    usart1_putString((uint8_t *)"Stops the clock and keeps the timing generator in reset\r\n");
    break;

  case MPU6050_CLOCK_EXTERNAL_19MHZ:
    usart1_putString((uint8_t *)"PLL with external 19.2MHz reference\r\n");
    break;

  case MPU6050_CLOCK_EXTERNAL_32KHZ:
    usart1_putString((uint8_t *)"PLL with external 32.768kHz reference\r\n");
    break;

  case MPU6050_CLOCK_PLL_ZGYRO:
    usart1_putString((uint8_t *)"PLL with Z axis gyroscope reference\r\n");
    break;

  case MPU6050_CLOCK_PLL_YGYRO:
    usart1_putString((uint8_t *)"PLL with Y axis gyroscope reference\r\n");
    break;

  case MPU6050_CLOCK_PLL_XGYRO:
    usart1_putString((uint8_t *)"PLL with X axis gyroscope reference\r\n");
    break;

  case MPU6050_CLOCK_INTERNAL_8MHZ:
    usart1_putString((uint8_t *)"Internal 8MHz oscillator\r\n");
    break;
  }

  /* Dải gia tốc */
  usart1_putString((uint8_t *)"Accelerometer: ");
  switch (mpu6050_getRange())
  {
  case MPU6050_RANGE_16G:
    usart1_putString((uint8_t *)"+/- 16 g\r\n");
    break;

  case MPU6050_RANGE_8G:
    usart1_putString((uint8_t *)"+/- 8 g\r\n");
    break;

  case MPU6050_RANGE_4G:
    usart1_putString((uint8_t *)"+/- 4 g\r\n");
    break;

  case MPU6050_RANGE_2G:
    usart1_putString((uint8_t *)"+/- 2 g\r\n");
    break;
  }

  /* Giá trị offset gia tốc */
  usart1_putString((uint8_t *)"Accelerometer offsets: ");
  sprintf(str, "%d / %d / %d\r\n", mpu6050_getAccelOffsetX(), mpu6050_getAccelOffsetY(), mpu6050_getAccelOffsetZ());
  usart1_putString((uint8_t *)str);

  /* Giá trị offset con quay hồi chuyển */
  usart1_putString((uint8_t *)"Gyroscope offsets: ");
  sprintf(str, "%d / %d / %d\r\n", mpu6050_getGyroOffsetX(), mpu6050_getGyroOffsetY(), mpu6050_getGyroOffsetZ());
  usart1_putString((uint8_t *)str);

  /* Hàm lặp vô hạn */
  while (1)
  {
    /* Đọc giá trị thô và giá trị của gia tốc */
    vectorTypeDef rawAccel = mpu6050_readRawAccel();
    vectorTypeDef normAccel = mpu6050_readNormalizeAccel();

    /* Đọc giá trị thô và giá trị của con quay hồi chuyển */
    vectorTypeDef rawGyro = mpu6050_readRawGyro();
    vectorTypeDef normGyro = mpu6050_readNormalizeGyro();

    /* Gửi chuỗi xuống dòng */
    usart1_putString((uint8_t *)"\r\n");

    /* Gửi giá trị thô của giá tốc ra usart1 */
    sprintf(str, "Accelerometer raw: Xraw = %.4f / Yraw = %.4f / Zraw = %.4f\r\n", rawAccel.XAxis, rawAccel.YAxis, rawAccel.ZAxis);
    usart1_putString((uint8_t *)str);

    /* Gửi giá trị của gia tốc ra usart1 */
    sprintf(str, "Accelerometer nomalize: Xnorm = %.4f / Ynorm = %.4f / Znorm = %.4f\r\n", normAccel.XAxis, normAccel.YAxis, normAccel.ZAxis);
    usart1_putString((uint8_t *)str);

    /* Gửi giá trị thô của con quay hồi chuyển ra usart1 */
    sprintf(str, "Gyroscope raw: Xraw = %.4f / Yraw = %.4f / Zraw = %.4f\r\n", rawGyro.XAxis, rawGyro.YAxis, rawGyro.ZAxis);
    usart1_putString((uint8_t *)str);

    /* Gửi giá trị của con quay hồi chuyển ra usart1 */
    sprintf(str, "Gyroscope nomalize: Xnorm = %.4f / Ynorm = %.4f / Znorm = %.4f\r\n", normGyro.XAxis, normGyro.YAxis, normGyro.ZAxis);
    usart1_putString((uint8_t *)str);

    /* Tạo trễ 1 khoảng thời gian */
    delay_ms(10);
  }
}
