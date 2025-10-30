#ifndef MPU6050_H
#define MPU6050_H

#include "hardware/i2c.h"

// Endereço I2C do MPU6050
#define I2C_PORT i2c0               // i2c0 pinos 0 e 1, i2c1 pinos 2 e 3
#define I2C_SDA 0                   // 0 ou 2
#define I2C_SCL 1                  // 1 ou 3

static int addr = 0x68;             // O endereço padrao deste IMU é o 0x68

static void mpu6050_reset();
static void mpu6050_read_raw(int16_t accel[3], int16_t gyro[3], int16_t *temp);

#endif