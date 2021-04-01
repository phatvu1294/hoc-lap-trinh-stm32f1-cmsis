#ifndef __I2C1_H
#define __I2C1_H

#include "stm32f1xx.h"

/* Sự kiện tham khảo từ ST */
#define I2C_EVENT_MASTER_MODE_SELECT                      0x00030001 /* BUSY, MSL and SB flag */
#define I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED        0x00070082 /* BUSY, MSL, ADDR, TXE and TRA flags */
#define I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED           0x00030002 /* BUSY, MSL and ADDR flags */
#define I2C_EVENT_MASTER_MODE_ADDRESS10                   0x00030008 /* BUSY, MSL and ADD10 flags */
#define I2C_EVENT_MASTER_BYTE_RECEIVED                    0x00030040 /* BUSY, MSL and RXNE flags */
#define I2C_EVENT_MASTER_BYTE_TRANSMITTING                0x00070080 /* TRA, BUSY, MSL, TXE flags */
#define I2C_EVENT_MASTER_BYTE_TRANSMITTED                 0x00070084 /* TRA, BUSY, MSL, TXE and BTF flags */
#define I2C_EVENT_SLAVE_RECEIVER_ADDRESS_MATCHED          0x00020002 /* BUSY and ADDR flags */
#define I2C_EVENT_SLAVE_TRANSMITTER_ADDRESS_MATCHED       0x00060082 /* TRA, BUSY, TXE and ADDR flags */
#define I2C_EVENT_SLAVE_RECEIVER_SECONDADDRESS_MATCHED    0x00820000 /* DUALF and BUSY flags */
#define I2C_EVENT_SLAVE_TRANSMITTER_SECONDADDRESS_MATCHED 0x00860080 /* DUALF, TRA, BUSY and TXE flags */
#define I2C_EVENT_SLAVE_GENERALCALLADDRESS_MATCHED        0x00120000 /* GENCALL and BUSY flags */
#define I2C_EVENT_SLAVE_BYTE_RECEIVED                     0x00020040 /* BUSY and RXNE flags */
#define I2C_EVENT_SLAVE_STOP_DETECTED                     0x00000010 /* STOPF flag */
#define I2C_EVENT_SLAVE_BYTE_TRANSMITTED                  0x00060084 /* TRA, BUSY, TXE and BTF flags */
#define I2C_EVENT_SLAVE_BYTE_TRANSMITTING                 0x00060080 /* TRA, BUSY and TXE flags */
#define I2C_EVENT_SLAVE_ACK_FAILURE                       0x00000400 /* AF flag */

/* Cấu hình I2C */
#define I2C_DUTY_CYCLE_2                                  0 /* Duty Cycle 2 */
#define I2C_DUTY_CYCLE_16_9                               1 /* Duty Cycle 16:9 */
#define I2C_DIRECTION_TRANSMITTER                         0 /* Direction Transmitter */
#define I2C_DIRECTION_RECEIVER                            1 /* Direction Receiver */

/* Nguyên mẫu hàm public */
void i2c1_init(void);
void i2c1_start(void);
void i2c1_stop(void);
void i2c1_addressDirection(uint8_t address, uint8_t direction);
void i2c1_transmit(uint8_t byte);
uint8_t i2c1_receiveAck(void);
uint8_t i2c1_receiveNack(void);
void i2c1_write(uint8_t address, uint8_t data);
void i2c1_read(uint8_t address, uint8_t *data);

#endif
