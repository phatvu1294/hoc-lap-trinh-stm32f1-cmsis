#ifndef __MPU6050_H
#define __MPU6050_H

#include "stm32f1xx.h"

/* Định nghĩa địa chỉ MPU6050 */
#define MPU6050_ADDR                  0x68 // 0x69 nếu chân AD0 được nối với VCC

/* Định nghĩa các thanh ghi */
#define MPU6050_REG_ACCEL_XOFFS_H     0x06
#define MPU6050_REG_ACCEL_XOFFS_L     0x07
#define MPU6050_REG_ACCEL_YOFFS_H     0x08
#define MPU6050_REG_ACCEL_YOFFS_L     0x09
#define MPU6050_REG_ACCEL_ZOFFS_H     0x0A
#define MPU6050_REG_ACCEL_ZOFFS_L     0x0B
#define MPU6050_REG_GYRO_XOFFS_H      0x13
#define MPU6050_REG_GYRO_XOFFS_L      0x14
#define MPU6050_REG_GYRO_YOFFS_H      0x15
#define MPU6050_REG_GYRO_YOFFS_L      0x16
#define MPU6050_REG_GYRO_ZOFFS_H      0x17
#define MPU6050_REG_GYRO_ZOFFS_L      0x18
#define MPU6050_REG_CONFIG            0x1A
#define MPU6050_REG_GYRO_CONFIG       0x1B
#define MPU6050_REG_ACCEL_CONFIG      0x1C
#define MPU6050_REG_FF_THRESHOLD      0x1D
#define MPU6050_REG_FF_DURATION       0x1E
#define MPU6050_REG_MOT_THRESHOLD     0x1F
#define MPU6050_REG_MOT_DURATION      0x20
#define MPU6050_REG_ZMOT_THRESHOLD    0x21
#define MPU6050_REG_ZMOT_DURATION     0x22
#define MPU6050_REG_INT_PIN_CFG       0x37
#define MPU6050_REG_INT_ENABLE        0x38
#define MPU6050_REG_INT_STATUS        0x3A
#define MPU6050_REG_ACCEL_XOUT_H      0x3B
#define MPU6050_REG_ACCEL_XOUT_L      0x3C
#define MPU6050_REG_ACCEL_YOUT_H      0x3D
#define MPU6050_REG_ACCEL_YOUT_L      0x3E
#define MPU6050_REG_ACCEL_ZOUT_H      0x3F
#define MPU6050_REG_ACCEL_ZOUT_L      0x40
#define MPU6050_REG_TEMP_OUT_H        0x41
#define MPU6050_REG_TEMP_OUT_L        0x42
#define MPU6050_REG_GYRO_XOUT_H       0x43
#define MPU6050_REG_GYRO_XOUT_L       0x44
#define MPU6050_REG_GYRO_YOUT_H       0x45
#define MPU6050_REG_GYRO_YOUT_L       0x46
#define MPU6050_REG_GYRO_ZOUT_H       0x47
#define MPU6050_REG_GYRO_ZOUT_L       0x48
#define MPU6050_REG_MOT_DETECT_STATUS 0x61
#define MPU6050_REG_MOT_DETECT_CTRL   0x69
#define MPU6050_REG_USER_CTRL         0x6A
#define MPU6050_REG_PWR_MGMT_1        0x6B
#define MPU6050_REG_WHO_AM_I          0x75

/* Định nghĩa cấu trúc dữ liệu */
typedef struct
{
  float XAxis;
  float YAxis;
  float ZAxis;
} vectorTypeDef;

typedef struct
{
  uint8_t isOverflow;
  uint8_t isFreeFall;
  uint8_t isInactivity;
  uint8_t isActivity;
  uint8_t isPosActivityOnX;
  uint8_t isPosActivityOnY;
  uint8_t isPosActivityOnZ;
  uint8_t isNegActivityOnX;
  uint8_t isNegActivityOnY;
  uint8_t isNegActivityOnZ;
  uint8_t isDataReady;
} activitesTypeDef;

/* Định nghĩa kiểu dữ liệu */
typedef enum
{
  MPU6050_CLOCK_KEEP_RESET            = 0x07,
  MPU6050_CLOCK_EXTERNAL_19MHZ        = 0x05,
  MPU6050_CLOCK_EXTERNAL_32KHZ        = 0x04,
  MPU6050_CLOCK_PLL_ZGYRO             = 0x03,
  MPU6050_CLOCK_PLL_YGYRO             = 0x02,
  MPU6050_CLOCK_PLL_XGYRO             = 0x01,
  MPU6050_CLOCK_INTERNAL_8MHZ         = 0x00,
} mpu6050_clockSource_t;

typedef enum
{
  MPU6050_SCALE_2000DPS               = 0x03,
  MPU6050_SCALE_1000DPS               = 0x02,
  MPU6050_SCALE_500DPS                = 0x01,
  MPU6050_SCALE_250DPS                = 0x00,
} mpu6050_dps_t;

typedef enum
{
  MPU6050_RANGE_16G                   = 0x03,
  MPU6050_RANGE_8G                    = 0x02,
  MPU6050_RANGE_4G                    = 0x01,
  MPU6050_RANGE_2G                    = 0x00,
} mpu6050_range_t;

typedef enum
{
  MPU6050_DELAY_3MS                   = 0x03,
  MPU6050_DELAY_2MS                   = 0x02,
  MPU6050_DELAY_1MS                   = 0x01,
  MPU6050_NO_DELAY                    = 0x00,
} mpu6050_onDelay_t;

typedef enum
{
  MPU6050_DHPF_HOLD                   = 0x07,
  MPU6050_DHPF_0_63HZ                 = 0x04,
  MPU6050_DHPF_1_25HZ                 = 0x03,
  MPU6050_DHPF_2_5HZ                  = 0x02,
  MPU6050_DHPF_5HZ                    = 0x01,
  MPU6050_DHPF_RESET                  = 0x00,
} mpu6050_dhpf_t;

typedef enum
{
  MPU6050_DLPF_6                      = 0x06,
  MPU6050_DLPF_5                      = 0x05,
  MPU6050_DLPF_4                      = 0x04,
  MPU6050_DLPF_3                      = 0x03,
  MPU6050_DLPF_2                      = 0x02,
  MPU6050_DLPF_1                      = 0x01,
  MPU6050_DLPF_0                      = 0x00,
} mpu6050_dlpf_t;

/* Nguyên mẫu hàm public */
uint8_t mpu6050_init(mpu6050_dps_t scale, mpu6050_range_t range);
void mpu6050_setScale(mpu6050_dps_t scale);
mpu6050_dps_t mpu6050_getScale(void);
void mpu6050_setRange(mpu6050_range_t range);
mpu6050_range_t mpu6050_getRange(void);
void mpu6050_setDHPFMode(mpu6050_dhpf_t dhpf);
void mpu6050_setDLPFMode(mpu6050_dlpf_t dlpf);
void mpu6050_setClockSource(mpu6050_clockSource_t source);
mpu6050_clockSource_t mpu6050_getClockSource(void);
void mpu6050_setSleepEnabled(uint8_t state);
uint8_t mpu6050_getSleepEnabled(void);
void mpu6050_setIntZeroMotionEnabled(uint8_t state);
uint8_t mpu6050_getIntZeroMotionEnabled(void);
void mpu6050_setIntMotionEnabled(uint8_t state);
uint8_t mpu6050_getIntMotionEnabled(void);
void mpu6050_setIntFreeFallEnabled(uint8_t state);
uint8_t mpu6050_getIntFreeFallEnabled(void);
void mpu6050_setMotionDetectionThreshold(uint8_t threshold);
uint8_t mpu6050_getMotionDetectionThreshold(void);
void mpu6050_setMotionDetectionDuration(uint8_t duration);
uint8_t mpu6050_getMotionDetectionDuration(void);
void mpu6050_setZeroMotionDetectionThreshold(uint8_t threshold);
uint8_t mpu6050_getZeroMotionDetectionThreshold(void);
void mpu6050_setZeroMotionDetectionDuration(uint8_t duration);
uint8_t mpu6050_getZeroMotionDetectionDuration(void);
void mpu6050_setFreeFallDetectionThreshold(uint8_t threshold);
uint8_t mpu6050_getFreeFallDetectionThreshold(void);
void mpu6050_setFreeFallDetectionDuration(uint8_t duration);
uint8_t mpu6050_getFreeFallDetectionDuration(void);
void mpu6050_setI2CMasterModeEnabled(uint8_t state);
uint8_t mpu6050_getI2CMasterModeEnabled(void);
void mpu6050_setI2CBypassEnabled(uint8_t state);
uint8_t mpu6050_getI2CBypassEnabled(void);
void mpu6050_setAccelPowerOnDelay(mpu6050_onDelay_t delay);
mpu6050_onDelay_t mpu6050_getAccelPowerOnDelay(void);
uint8_t mpu6050_getIntStatus(void);
activitesTypeDef mpu6050_readActivites(void);
vectorTypeDef mpu6050_readRawAccel(void);
vectorTypeDef mpu6050_readNormalizeAccel(void);
vectorTypeDef mpu6050_readScaledAccel(void);
vectorTypeDef mpu6050_readRawGyro(void);
vectorTypeDef mpu6050_readNormalizeGyro(void);
float mpu6050_readTemperature(void);
void mpu6050_setGyroOffsetX(int16_t offset);
void mpu6050_setGyroOffsetY(int16_t offset);
void mpu6050_setGyroOffsetZ(int16_t offset);
int16_t mpu6050_getGyroOffsetX(void);
int16_t mpu6050_getGyroOffsetY(void);
int16_t mpu6050_getGyroOffsetZ(void);
void mpu6050_setAccelOffsetX(int16_t offset);
void mpu6050_setAccelOffsetY(int16_t offset);
void mpu6050_setAccelOffsetZ(int16_t offset);
int16_t mpu6050_getAccelOffsetX(void);
int16_t mpu6050_getAccelOffsetY(void);
int16_t mpu6050_getAccelOffsetZ(void);
void mpu6050_calibrateGyro(uint8_t samples); // Mặc định: 50
void mpu6050_setThreshold(uint8_t multiple); // Mặc định: 1
uint8_t mpu6050_getThreshold(void);

#endif
