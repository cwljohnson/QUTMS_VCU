/*
 * imu.h
 *
 *  Created on: Aug 16, 2021
 *      Author: Calvin
 */

#ifndef INC_IMU_H_
#define INC_IMU_H_

#include <stdbool.h>

// ICG-20660L

#define IMU_SMPLRT_DIV		0x19
#define IMU_CONFIG			0x1A
#define IMU_GYRO_CONFIG		0x1B
#define IMU_ACCEL_CONFIG	0x1C
#define IMU_ACCEL_CONFIG2	0x1D
#define IMU_LP_MODE_CFG		0x1E
#define IMU_FIFO_EN			0x23
#define IMU_ACCEL_XOUT_H	0x3B
#define IMU_USER_CTRL		0x6A
#define IMU_PWR_MGMT_1 		0x6B
#define IMU_PWR_MGMT_2		0x6C
#define IMU_FIFO_COUNTH		0x72
#define IMU_FIFO_COUNTL		0x73
#define IMU_FIFO_R_W		0x74
#define IMU_WHO_AM_I 		0x75

#define ICG20660L_DEVICE_ID 0x91
#define IMU_CLOCK_SEL_PLL 1

#define IMU_ADC_MAX_RANGE   32767.0

#define IMU_ACCEL_FSR_2G 2
#define IMU_ACCEL_FSR_4G 4
#define IMU_ACCEL_FSR_8G 8
#define IMU_ACCEL_FSR_16G 16

#define IMU_GYRO_FULL_SCALE_125DPS 125.0
#define IMU_GYRO_FULL_SCALE_250DPS 250.0
#define IMU_GYRO_FULL_SCALE_500DPS 500.0

#define IMU_RAW_DATA_AX_H_INDEX		0x00
#define IMU_RAW_DATA_AX_L_INDEX		0x01
#define IMU_RAW_DATA_AY_H_INDEX		0x02
#define IMU_RAW_DATA_AY_L_INDEX		0x03
#define IMU_RAW_DATA_AZ_H_INDEX		0x04
#define IMU_RAW_DATA_AZ_L_INDEX		0x05
#define IMU_RAW_DATA_T_H_INDEX 		0x06
#define IMU_RAW_DATA_T_L_INDEX 		0x07
#define IMU_RAW_DATA_GX_H_INDEX		0x08
#define IMU_RAW_DATA_GX_L_INDEX		0x09
#define IMU_RAW_DATA_GY_H_INDEX		0x0A
#define IMU_RAW_DATA_GY_L_INDEX		0x0B
#define IMU_RAW_DATA_GZ_H_INDEX		0x0C
#define IMU_RAW_DATA_GZ_L_INDEX		0x0D
#define IMU_RAW_DATA_LENGTH 		14

enum IMU_SENSOR_ENABLE {
	IMU_SENSOR_E_G_AXIS_Z = (1 << 6) | (1 << 0),
	IMU_SENSOR_E_G_AXIS_Y = (1 << 6) | (1 << 1),
	IMU_SENSOR_E_G_AXIS_X = (1 << 6) | (1 << 2),
	IMU_SENSOR_E_A_AXIS_Z = (1 << 3),
	IMU_SENSOR_E_A_AXIS_Y = (1 << 4),
	IMU_SENSOR_E_A_AXIS_X = (1 << 5),
	IMU_SENSOR_E_G_AXIS_XYZ = (1 << 6) | (0x07),
	IMU_SENSOR_E_A_AXIS_XYZ = 0x07 << 3,
	IMU_SENSOR_E_AXIS_ALL = ((1 << 6) | 0x3f)
};

enum IMU_POWER_MODE {
	IMU_PWR_MODE_SLEEP = 0,
	IMU_PWR_MODE_STANDBY = 1,
	IMU_PWR_MODE_ACCEL_LOWPOWER = 2,
	IMU_PWR_MODE_ACCEL_LOWNOISE = 3,
	IMU_PWR_MODE_6AXIS_LOWNOISE = 4
};

enum IMU_ACCEL_FSR {
	IMU_FSR_A_2G = 0, IMU_FSR_A_4G = 1, IMU_FSR_A_8G = 2, IMU_FSR_A_16G = 3
};

enum IMU_ACCEL_BANDWIDTH {
	IMU_ACCEL_DLPF_5_1KHZ = 0,
	IMU_ACCEL_DLPF_10_1KHZ = 1,
	IMU_ACCEL_DLPF_21_1KHZ = 2,
	IMU_ACCEL_DLPF_44_1KHZ = 3,
	IMU_ACCEL_DLPF_99_1KHZ = 4,
	IMU_ACCEL_DLPF_218_1KHZ = 5,
	IMU_ACCEL_DLPF_420_1KHZ = 6,
	IMU_ACCEL_DLPF_1046_4KHZ = 7,
	IMU_ACCEL_DLPF_55_1KHZ = 8,
	IMU_ACCEL_DLPF_110_1KHZ = 9
};

enum IMU_GYRO_BANDWIDTH {
	IMU_GYRO_DLPF_8173_32KHZ = 0,
	IMU_GYRO_DLPF_3281_32KHZ = 1,
	IMU_GYRO_DLPF_250_8KHZ = 2,
	IMU_GYRO_DLPF_176_1KHZ = 3,
	IMU_GYRO_DLPF_92_1KHZ = 4,
	IMU_GYRO_DLPF_3281_8KHZ = 5,
};

enum IMU_GYRO_FSR {
	IMU_GYRO_125DPS = 0,
	IMU_GYRO_250DPS = 1,
	IMU_GYRO_500DPS = 2
};

void IMU_CS_ON();
void IMU_CS_OFF();

void IMU_reset();
void IMU_exitSleep();

uint8_t IMU_readID();

void IMU_enableSensor(uint8_t sensorBit);
void IMU_disableSensor(uint8_t sensorBit);
bool IMU_enableFIFO(bool temp, bool gx, bool gy, bool gz, bool a);

void IMU_configAccel(uint8_t scale, uint8_t bd, uint8_t *odr, bool lowPowerFlag);
void IMU_setFullScaleForAccel(uint8_t scale);
void IMU_setBandwidthForAccel(uint8_t bandwidth);

void IMU_setBandwidthForAccelInLowPowerMode(uint8_t bandwidth);
void IMU_setBandwidthForAccelInOthersMode(uint8_t bandwidth);

void IMU_configGyro(uint8_t scale, uint8_t bd);
void IMU_setFullScaleForGyro(uint8_t scale);
void IMU_setBandwidthForGyro(uint8_t bandwidth);

void IMU_setSampleDiv(uint8_t div);

void IMU_getRawData(uint8_t *data, uint8_t len);
void IMU_readDataFromFIFO();
void IMU_readDataFromREG();

float IMU_getAccelDataX();
float IMU_getAccelDataY();
float IMU_getAccelDataZ();

float IMU_getGyroDataX();
float IMU_getGyroDataY();
float IMU_getGyroDataZ();

float IMU_getTemperatureC();

uint16_t IMU_getAccelScaler();
uint16_t IMU_getGyroScaler();

uint8_t IMU_ReadRegister(uint8_t addr, uint8_t *out, size_t len);
void IMU_WriteRegister(uint8_t addr, uint8_t *data, size_t len);

void IMU_Init(bool dataMode_FIFO);

#endif /* INC_IMU_H_ */
