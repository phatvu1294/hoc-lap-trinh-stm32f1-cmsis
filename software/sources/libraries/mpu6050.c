#include "mpu6050.h"
#include "delay.h"
#include "i2c1.h"
#include <math.h>

/* Biến private */
vectorTypeDef __ra, __rg;   // Các giá trị thô của vector
vectorTypeDef __na, __ng;   // Các giá trị của vector
vectorTypeDef __tg, __dg;   // Threshold và Delta cho con quay hồi chuyển
vectorTypeDef __th;         // Threshold
activitesTypeDef __a;       // Activities

float __dpsPerDigit, __rangePerDigit;
float __actualThreshold;
uint8_t __useCalibrate;

/* Nguyên mẫu hàm private */
void mpu6050_writeRegisterBit(uint8_t reg, uint8_t pos, uint8_t bit);
void mpu6050_writeRegister8(uint8_t reg, uint8_t value);
void mpu6050_writeRegister16(uint8_t reg, int16_t value);
uint8_t mpu6050_readRegisterBit(uint8_t reg, uint8_t pos);
uint8_t mpu6050_readRegister8(uint8_t reg);
int16_t mpu6050_readRegister16(uint8_t reg);

/* Hàm khởi tạo MPU6050 */
uint8_t mpu6050_init(mpu6050_dps_t scale, mpu6050_range_t range)
{
  /* Khởi tạo delay */
  delay_init();

  /* Khởi tạo i2c1 */
  i2c1_init();

  /* Đặt lại hiệu chỉnh */
  __dg.XAxis = 0;
  __dg.YAxis = 0;
  __dg.ZAxis = 0;
  __useCalibrate = 0;

  /* Đặt lại threshold */
  __tg.XAxis = 0;
  __tg.YAxis = 0;
  __tg.ZAxis = 0;
  __actualThreshold = 0;

  /* Kiểm tra thanh ghi Who Am I */
  if (mpu6050_readRegister8(MPU6050_REG_WHO_AM_I) != 0x68)
  {
    /* Khởi tạo thất bại */
    return 0;
  }

  /* Đặt nguồn clock */
  mpu6050_setClockSource(MPU6050_CLOCK_PLL_XGYRO);

  /* Đặt tỷ lệ và dải */
  mpu6050_setScale(scale);
  mpu6050_setRange(range);

  /* Tắt chế độ Sleep */
  mpu6050_setSleepEnabled(0);

  /* Khởi tạo thành công */
  return 1;
}

/* Hàm đặt tỷ lệ */
void mpu6050_setScale(mpu6050_dps_t scale)
{
  uint8_t value = 0;

  switch (scale)
  {
  case MPU6050_SCALE_250DPS:
    __dpsPerDigit = .007633f;
    break;

  case MPU6050_SCALE_500DPS:
    __dpsPerDigit = .015267f;
    break;

  case MPU6050_SCALE_1000DPS:
    __dpsPerDigit = .030487f;
    break;

  case MPU6050_SCALE_2000DPS:
    __dpsPerDigit = .060975f;
    break;

  default:
    break;
  }

  value = mpu6050_readRegister8(MPU6050_REG_GYRO_CONFIG);
  value &= 0xE7;
  value |= (scale << 3);
  mpu6050_writeRegister8(MPU6050_REG_GYRO_CONFIG, value);
}

/* Hàm lấy tỷ lệ */
mpu6050_dps_t mpu6050_getScale(void)
{
  uint8_t value = 0;
  value = mpu6050_readRegister8(MPU6050_REG_GYRO_CONFIG);
  value &= 0x18;
  value >>= 3;
  return (mpu6050_dps_t)value;
}

/* Hàm đặt dải */
void mpu6050_setRange(mpu6050_range_t range)
{
  uint8_t value = 0;

  switch (range)
  {
  case MPU6050_RANGE_2G:
    __rangePerDigit = .000061f;
    break;

  case MPU6050_RANGE_4G:
    __rangePerDigit = .000122f;
    break;

  case MPU6050_RANGE_8G:
    __rangePerDigit = .000244f;
    break;

  case MPU6050_RANGE_16G:
    __rangePerDigit = .0004882f;
    break;

  default:
    break;
  }

  value = mpu6050_readRegister8(MPU6050_REG_ACCEL_CONFIG);
  value &= 0xE7;
  value |= (range << 3);
  mpu6050_writeRegister8(MPU6050_REG_ACCEL_CONFIG, value);
}

/* Hàm lấy dải */
mpu6050_range_t mpu6050_getRange(void)
{
  uint8_t value = 0;
  value = mpu6050_readRegister8(MPU6050_REG_ACCEL_CONFIG);
  value &= 0x18;
  value >>= 3;
  return (mpu6050_range_t)value;
}

/* Hàm đặt Digital High Pass Filter */
void mpu6050_setDHPFMode(mpu6050_dhpf_t dhpf)
{
  uint8_t value = 0;
  value = mpu6050_readRegister8(MPU6050_REG_ACCEL_CONFIG);
  value &= 0xF8;
  value |= dhpf;
  mpu6050_writeRegister8(MPU6050_REG_ACCEL_CONFIG, value);
}

/* Hàm đặt Digital Low Pass Filter */
void mpu6050_setDLPFMode(mpu6050_dlpf_t dlpf)
{
  uint8_t value = 0;
  value = mpu6050_readRegister8(MPU6050_REG_CONFIG);
  value &= 0xF8;
  value |= dlpf;
  mpu6050_writeRegister8(MPU6050_REG_CONFIG, value);
}

/* Hàm đặt nguồn Clock */
void mpu6050_setClockSource(mpu6050_clockSource_t source)
{
  uint8_t value = 0;
  value = mpu6050_readRegister8(MPU6050_REG_PWR_MGMT_1);
  value &= 0xF8;
  value |= source;
  mpu6050_writeRegister8(MPU6050_REG_PWR_MGMT_1, value);
}

/* Hàm lấy nguồn Clock */
mpu6050_clockSource_t mpu6050_getClockSource(void)
{
  uint8_t value = 0;
  value = mpu6050_readRegister8(MPU6050_REG_PWR_MGMT_1);
  value &= 0x07;
  return (mpu6050_clockSource_t)value;
}

/* Hàm đặt trạng thái ngủ */
void mpu6050_setSleepEnabled(uint8_t state)
{
  mpu6050_writeRegisterBit(MPU6050_REG_PWR_MGMT_1, 6, state);
}

/* Hàm lấy trạng thái ngủ */
uint8_t mpu6050_getSleepEnabled(void)
{
  return mpu6050_readRegisterBit(MPU6050_REG_PWR_MGMT_1, 6);
}

/* Hàm đặt trạng thái bật ngắt zero motion */
void mpu6050_setIntZeroMotionEnabled(uint8_t state)
{
  mpu6050_writeRegisterBit(MPU6050_REG_INT_ENABLE, 5, state);
}


/* Hàm lấy trạng thái bật ngắt zero motion */
uint8_t mpu6050_getIntZeroMotionEnabled(void)
{
  return mpu6050_readRegisterBit(MPU6050_REG_INT_ENABLE, 5);
}

/* Hàm đặt trạng thái bật ngắt motion */
void mpu6050_setIntMotionEnabled(uint8_t state)
{
  mpu6050_writeRegisterBit(MPU6050_REG_INT_ENABLE, 6, state);
}

/* Hàm lấy trạng thái bật ngắt motion */
uint8_t mpu6050_getIntMotionEnabled(void)
{
  return mpu6050_readRegisterBit(MPU6050_REG_INT_ENABLE, 6);
}

/* Hàm đặt trạng thái bật ngắt free fall */
void mpu6050_setIntFreeFallEnabled(uint8_t state)
{
  mpu6050_writeRegisterBit(MPU6050_REG_INT_ENABLE, 7, state);
}

/* Hàm lấy trạng thái bật ngắt free fall */
uint8_t mpu6050_getIntFreeFallEnabled(void)
{
  return mpu6050_readRegisterBit(MPU6050_REG_INT_ENABLE, 7);
}

/* Hàm đặt threshold motion detection */
void mpu6050_setMotionDetectionThreshold(uint8_t threshold)
{
  mpu6050_writeRegister8(MPU6050_REG_MOT_THRESHOLD, threshold);
}

/* Hàm lấy threshold motion detection */
uint8_t mpu6050_getMotionDetectionThreshold(void)
{
  return mpu6050_readRegister8(MPU6050_REG_MOT_THRESHOLD);
}

/* Hàm đặt thời gian trễ motion detection */
void mpu6050_setMotionDetectionDuration(uint8_t duration)
{
  mpu6050_writeRegister8(MPU6050_REG_MOT_DURATION, duration);
}

/* Hàm lấy thời gian trễ motion detection */
uint8_t mpu6050_getMotionDetectionDuration(void)
{
  return mpu6050_readRegister8(MPU6050_REG_MOT_DURATION);
}

/* Hàm đặt threshold zero motion detection */
void mpu6050_setZeroMotionDetectionThreshold(uint8_t threshold)
{
  mpu6050_writeRegister8(MPU6050_REG_ZMOT_THRESHOLD, threshold);
}

/* Hàm lấy threshold zero motion detection */
uint8_t mpu6050_getZeroMotionDetectionThreshold(void)
{
  return mpu6050_readRegister8(MPU6050_REG_ZMOT_THRESHOLD);
}

/* Hàm đặt thời gian trễ zero motion detection */
void mpu6050_setZeroMotionDetectionDuration(uint8_t duration)
{
  mpu6050_writeRegister8(MPU6050_REG_ZMOT_DURATION, duration);
}

/* Hàm lấy thời gian trễ zero motion detection */
uint8_t mpu6050_getZeroMotionDetectionDuration(void)
{
  return mpu6050_readRegister8(MPU6050_REG_ZMOT_DURATION);
}

/* Hàm đặt threshold free fall detection */
void mpu6050_setFreeFallDetectionThreshold(uint8_t threshold)
{
  mpu6050_writeRegister8(MPU6050_REG_FF_THRESHOLD, threshold);
}

/* Hàm lấy threshold free fall detection */
uint8_t mpu6050_getFreeFallDetectionThreshold(void)
{
  return mpu6050_readRegister8(MPU6050_REG_FF_THRESHOLD);
}

/* Hàm đặt thời gian trễ free fall detection */
void mpu6050_setFreeFallDetectionDuration(uint8_t duration)
{
  mpu6050_writeRegister8(MPU6050_REG_FF_DURATION, duration);
}

/* Hàm lấy thời gian trễ free fall detection */
uint8_t mpu6050_getFreeFallDetectionDuration(void)
{
  return mpu6050_readRegister8(MPU6050_REG_FF_DURATION);
}

/* Hàm đặt trạng thái bật mode i2c */
void mpu6050_setI2CMasterModeEnabled(uint8_t state)
{
  mpu6050_writeRegisterBit(MPU6050_REG_USER_CTRL, 5, state);
}

/* Hàm lấy trạng thái bật mode i2c */
uint8_t mpu6050_getI2CMasterModeEnabled(void)
{
  return mpu6050_readRegisterBit(MPU6050_REG_USER_CTRL, 5);
}

/* Hàm đặt trạng thái bật i2c bypass */
void mpu6050_setI2CBypassEnabled(uint8_t state)
{
  mpu6050_writeRegisterBit(MPU6050_REG_INT_PIN_CFG, 1, state);
}

/* Hàm lấy trạng thái bật i2c bypass */
uint8_t mpu6050_getI2CBypassEnabled(void)
{
  return mpu6050_readRegisterBit(MPU6050_REG_INT_PIN_CFG, 1);
}

/* Hàm đặt thời trễ khi bật gia tốc */
void mpu6050_setAccelPowerOnDelay(mpu6050_onDelay_t delay)
{
  uint8_t value;
  value = mpu6050_readRegister8(MPU6050_REG_MOT_DETECT_CTRL);
  value &= 0xCF;
  value |= (delay << 4);
  mpu6050_writeRegister8(MPU6050_REG_MOT_DETECT_CTRL, value);
}

/* Hàm lấy thời trễ khi bật gia tốc */
mpu6050_onDelay_t mpu6050_getAccelPowerOnDelay(void)
{
  uint8_t value;
  value = mpu6050_readRegister8(MPU6050_REG_MOT_DETECT_CTRL);
  value &= 0x30;
  return (mpu6050_onDelay_t)(value >> 4);
}

/* Hàm lấy trạng thái ngắt */
uint8_t mpu6050_getIntStatus(void)
{
  return mpu6050_readRegister8(MPU6050_REG_INT_STATUS);
}

/* Hàm đọc trạng thái hoạt động */
activitesTypeDef mpu6050_readActivites(void)
{
  uint8_t data = mpu6050_readRegister8(MPU6050_REG_INT_STATUS);
  __a.isOverflow = ((data >> 4) & 1);
  __a.isFreeFall = ((data >> 7) & 1);
  __a.isInactivity = ((data >> 5) & 1);
  __a.isActivity = ((data >> 6) & 1);
  __a.isDataReady = ((data >> 0) & 1);

  data = mpu6050_readRegister8(MPU6050_REG_MOT_DETECT_STATUS);
  __a.isNegActivityOnX = ((data >> 7) & 1);
  __a.isPosActivityOnX = ((data >> 6) & 1);
  __a.isNegActivityOnY = ((data >> 5) & 1);
  __a.isPosActivityOnY = ((data >> 4) & 1);
  __a.isNegActivityOnZ = ((data >> 3) & 1);
  __a.isPosActivityOnZ = ((data >> 2) & 1);

  return __a;
}

/* Hàm đọc giá trị thô của gia tốc */
vectorTypeDef mpu6050_readRawAccel(void)
{
  i2c1_start();
  i2c1_addressDirection(MPU6050_ADDR << 1, I2C_DIRECTION_TRANSMITTER);
  i2c1_transmit(MPU6050_REG_ACCEL_XOUT_H);
  i2c1_start();
  i2c1_addressDirection(MPU6050_ADDR << 1, I2C_DIRECTION_RECEIVER);
  uint8_t xH = i2c1_receiveAck();
  uint8_t xL = i2c1_receiveAck();
  uint8_t yH = i2c1_receiveAck();
  uint8_t yL = i2c1_receiveAck();
  uint8_t zH = i2c1_receiveAck();
  uint8_t zL = i2c1_receiveNack();
  i2c1_stop();

  __ra.XAxis = (int16_t)(xH << 8) | xL;
  __ra.YAxis = (int16_t)(yH << 8) | yL;
  __ra.ZAxis = (int16_t)(zH << 8) | zL;

  return __ra;
}

/* Hàm đọc giá trị của gia tốc */
vectorTypeDef mpu6050_readNormalizeAccel(void)
{
  mpu6050_readRawAccel();

  __na.XAxis = __ra.XAxis * __rangePerDigit * 9.80665f;
  __na.YAxis = __ra.YAxis * __rangePerDigit * 9.80665f;
  __na.ZAxis = __ra.ZAxis * __rangePerDigit * 9.80665f;

  return __na;
}

/* Hàm đọc giá trị tỷ lệ của gia tốc */
vectorTypeDef mpu6050_readScaledAccel(void)
{
  mpu6050_readRawAccel();

  __na.XAxis = __ra.XAxis * __rangePerDigit;
  __na.YAxis = __ra.YAxis * __rangePerDigit;
  __na.ZAxis = __ra.ZAxis * __rangePerDigit;

  return __na;
}

/* Hàm đọc giá trị thô của con quay hồi chuyển */
vectorTypeDef mpu6050_readRawGyro(void)
{
  i2c1_start();
  i2c1_addressDirection(MPU6050_ADDR << 1, I2C_DIRECTION_TRANSMITTER);
  i2c1_transmit(MPU6050_REG_GYRO_XOUT_H);
  i2c1_start();
  i2c1_addressDirection(MPU6050_ADDR << 1, I2C_DIRECTION_RECEIVER);
  uint8_t xH = i2c1_receiveAck();
  uint8_t xL = i2c1_receiveAck();
  uint8_t yH = i2c1_receiveAck();
  uint8_t yL = i2c1_receiveAck();
  uint8_t zH = i2c1_receiveAck();
  uint8_t zL = i2c1_receiveNack();
  i2c1_stop();

  __rg.XAxis = (int16_t)(xH << 8) | xL;
  __rg.YAxis = (int16_t)(yH << 8) | yL;
  __rg.ZAxis = (int16_t)(zH << 8) | zL;

  return __rg;
}

/* Hàm đọc giá trị của con quay hồi chuyển */
vectorTypeDef mpu6050_readNormalizeGyro(void)
{
  mpu6050_readRawGyro();

  if (__useCalibrate)
  {
    __ng.XAxis = (__rg.XAxis - __dg.XAxis) * __dpsPerDigit;
    __ng.YAxis = (__rg.YAxis - __dg.YAxis) * __dpsPerDigit;
    __ng.ZAxis = (__rg.ZAxis - __dg.ZAxis) * __dpsPerDigit;
  }
  else
  {
    __ng.XAxis = __rg.XAxis * __dpsPerDigit;
    __ng.YAxis = __rg.YAxis * __dpsPerDigit;
    __ng.ZAxis = __rg.ZAxis * __dpsPerDigit;
  }

  if (__actualThreshold)
  {
    if (fabs(__ng.XAxis) < __tg.XAxis) __ng.XAxis = 0;
    if (fabs(__ng.YAxis) < __tg.YAxis) __ng.YAxis = 0;
    if (fabs(__ng.ZAxis) < __tg.ZAxis) __ng.ZAxis = 0;
  }

  return __ng;
}

/* Hàm đọc giá trị nhiệt độ */
float mpu6050_readTemperature(void)
{
  int16_t t;
  t = mpu6050_readRegister16(MPU6050_REG_TEMP_OUT_H);
  return (float)t / 340 + 36.53;
}

/* Hàm đặt offset trục X con quay hồi chuyển */
void mpu6050_setGyroOffsetX(int16_t offset)
{
  mpu6050_writeRegister16(MPU6050_REG_GYRO_XOFFS_H, offset);
}

/* Hàm đặt offset trục Y con quay hồi chuyển */
void mpu6050_setGyroOffsetY(int16_t offset)
{
  mpu6050_writeRegister16(MPU6050_REG_GYRO_YOFFS_H, offset);
}

/* Hàm đặt offset trục Z con quay hồi chuyển */
void mpu6050_setGyroOffsetZ(int16_t offset)
{
  mpu6050_writeRegister16(MPU6050_REG_GYRO_ZOFFS_H, offset);
}

/* Hàm lấy offset trục X con quay hồi chuyển */
int16_t mpu6050_getGyroOffsetX(void)
{
  return mpu6050_readRegister16(MPU6050_REG_GYRO_XOFFS_H);
}

/* Hàm lấy offset trục Y con quay hồi chuyển */
int16_t mpu6050_getGyroOffsetY(void)
{
  return mpu6050_readRegister16(MPU6050_REG_GYRO_YOFFS_H);
}

/* Hàm lấy offset trục Z con quay hồi chuyển */
int16_t mpu6050_getGyroOffsetZ(void)
{
  return mpu6050_readRegister16(MPU6050_REG_GYRO_ZOFFS_H);
}

/* Hàm đặt offset trục X gia tốc */
void mpu6050_setAccelOffsetX(int16_t offset)
{
  mpu6050_writeRegister16(MPU6050_REG_ACCEL_XOFFS_H, offset);
}

/* Hàm đặt offset trục Y gia tốc */
void mpu6050_setAccelOffsetY(int16_t offset)
{
  mpu6050_writeRegister16(MPU6050_REG_ACCEL_YOFFS_H, offset);
}

/* Hàm đặt offset trục Z gia tốc */
void mpu6050_setAccelOffsetZ(int16_t offset)
{
  mpu6050_writeRegister16(MPU6050_REG_ACCEL_ZOFFS_H, offset);
}

/* Hàm lấy offset trục X gia tốc */
int16_t mpu6050_getAccelOffsetX(void)
{
  return mpu6050_readRegister16(MPU6050_REG_ACCEL_XOFFS_H);
}

/* Hàm lấy offset trục Y gia tốc */
int16_t mpu6050_getAccelOffsetY(void)
{
  return mpu6050_readRegister16(MPU6050_REG_ACCEL_YOFFS_H);
}

/* Hàm lấy offset trục Z gia tốc */
int16_t mpu6050_getAccelOffsetZ(void)
{
  return mpu6050_readRegister16(MPU6050_REG_ACCEL_ZOFFS_H);
}

/* Hàm hiệu chỉnh con quay hồi chuyển */
void mpu6050_calibrateGyro(uint8_t samples)
{
  /* Đặt hiệu chỉnh */
  __useCalibrate = 1;

  /* Đặt lại giá trị */
  float sumX = 0;
  float sumY = 0;
  float sumZ = 0;
  float sigmaX = 0;
  float sigmaY = 0;
  float sigmaZ = 0;

  /* Đọc n sample */
  for (uint8_t i = 0; i < samples; ++i)
  {
    mpu6050_readRawGyro();

    sumX += __rg.XAxis;
    sumY += __rg.YAxis;
    sumZ += __rg.ZAxis;

    sigmaX += __rg.XAxis * __rg.XAxis;
    sigmaY += __rg.YAxis * __rg.YAxis;
    sigmaZ += __rg.ZAxis * __rg.ZAxis;

    delay_ms(5);
  }

  /* Tính toán các vector delta */
  __dg.XAxis = sumX / samples;
  __dg.YAxis = sumY / samples;
  __dg.ZAxis = sumZ / samples;

  /* Tính toán các vector threshold */
  __th.XAxis = sqrt((sigmaX / 50) - (__dg.XAxis * __dg.XAxis));
  __th.YAxis = sqrt((sigmaY / 50) - (__dg.YAxis * __dg.YAxis));
  __th.ZAxis = sqrt((sigmaZ / 50) - (__dg.ZAxis * __dg.ZAxis));

  /* Nếu đã đặt threshold thì đặt lại */
  if (__actualThreshold > 0)
  {
    mpu6050_setThreshold(__actualThreshold);
  }
}

/* Hàm đặt giá trị threshold */
void mpu6050_setThreshold(uint8_t multiple)
{
  if (multiple > 0)
  {
    /* Nếu không sử dụng hiệu chỉnh thì hiệu chỉnh */
    if (!__useCalibrate)
    {
      mpu6050_calibrateGyro(50);
    }

    /* Tính toán ngưỡng vector */
    __tg.XAxis = __th.XAxis * multiple;
    __tg.YAxis = __th.YAxis * multiple;
    __tg.ZAxis = __th.ZAxis * multiple;
  }
  else
  {
    /* Đặt lại threshold */
    __tg.XAxis = 0;
    __tg.YAxis = 0;
    __tg.ZAxis = 0;
  }

  /* Lưu lại giá trị threshold */
  __actualThreshold = multiple;
}

/* Hàm lấy giá trị threshold */
uint8_t mpu6050_getThreshold(void)
{
  return ((uint8_t)__actualThreshold);
}

/* Hàm ghi bit vào thanh ghi */
void mpu6050_writeRegisterBit(uint8_t reg, uint8_t pos, uint8_t bit)
{
  uint8_t value = 0;
  value = mpu6050_readRegister8(reg);

  if (bit != 0)
  {
    value |= (1 << pos);
  }
  else
  {
    value &= ~(1 << pos);
  }

  mpu6050_writeRegister8(reg, value);
}

/* Hàm ghi dữ liệu 8-bit vào thanh ghi */
void mpu6050_writeRegister8(uint8_t reg, uint8_t value)
{
  i2c1_start();
  i2c1_addressDirection(MPU6050_ADDR << 1, I2C_DIRECTION_TRANSMITTER);
  i2c1_transmit(reg);
  i2c1_transmit(value);
  i2c1_stop();
}

/* Hàm ghi dữ liệu 16-bit vào thanh ghi */
void mpu6050_writeRegister16(uint8_t reg, int16_t value)
{
  i2c1_start();
  i2c1_addressDirection(MPU6050_ADDR << 1, I2C_DIRECTION_TRANSMITTER);
  i2c1_transmit(reg);
  i2c1_transmit((value >> 8) & 0xFF);
  i2c1_transmit(value & 0xFF);
  i2c1_stop();
}

/* Hàm đọc bit từ thanh ghi */
uint8_t mpu6050_readRegisterBit(uint8_t reg, uint8_t pos)
{
  uint8_t value = 0;
  value = mpu6050_readRegister8(reg);
  return ((value >> pos) & 0x01);
}

/* Hàm đọc dữ liệu 8-bit từ thanh ghi */
uint8_t mpu6050_readRegister8(uint8_t reg)
{
  uint8_t value = 0;

  i2c1_start();
  i2c1_addressDirection(MPU6050_ADDR << 1, I2C_DIRECTION_TRANSMITTER);
  i2c1_transmit(reg);
  i2c1_start();
  i2c1_addressDirection(MPU6050_ADDR << 1, I2C_DIRECTION_RECEIVER);
  value = i2c1_receiveNack();
  i2c1_stop();

  return value;
}

/* Hàm đọc dữ liệu 16-bit từ thanh ghi */
int16_t mpu6050_readRegister16(uint8_t reg)
{
  int16_t value = 0;
  uint8_t valueH = 0;
  uint8_t valueL = 0;

  i2c1_start();
  i2c1_addressDirection(MPU6050_ADDR << 1, I2C_DIRECTION_TRANSMITTER);
  i2c1_transmit(reg);
  i2c1_start();
  i2c1_addressDirection(MPU6050_ADDR << 1, I2C_DIRECTION_RECEIVER);
  valueH = i2c1_receiveAck();
  valueL = i2c1_receiveNack();
  i2c1_stop();

  value = (int16_t)(valueH << 8) | valueL;
  return value;
}
