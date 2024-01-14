/*
 * imu.c
 *
 *  Created on: Aug 16, 2021
 *      Author: Calvin
 */

#include "main.h"
#include "imu.h"
#include "spi.h"

//#include "p_watchdog.h"
#include <string.h>
#include <stdio.h>

// https://github.com/DFRobot/DFRobot_ICG20660L

bool dataModeFIFO = false;
uint8_t fifoFrameSize;

uint8_t powerMode;

float accelScale;
float accelRange;

float gyroScale;
float gyroRange;

uint8_t rawData[IMU_RAW_DATA_LENGTH];
uint8_t updateData;

char imu_sbuff[200];

void IMU_CS_ON() {
	HAL_GPIO_WritePin(IMU_CS_GPIO_Port, IMU_CS_Pin, GPIO_PIN_RESET);
}

void IMU_CS_OFF() {
	HAL_GPIO_WritePin(IMU_CS_GPIO_Port, IMU_CS_Pin, GPIO_PIN_SET);
}

uint8_t IMU_ReadRegister(uint8_t addr, uint8_t *out, size_t len) {
	uint8_t address = 0x80 | (addr);

	uint8_t txBuffer = address;
	uint8_t rxBuffer = 0;

	uint8_t count = 0;

	IMU_CS_ON();
	HAL_StatusTypeDef result = HAL_SPI_TransmitReceive(&hspi2, &txBuffer, &rxBuffer, 1, 1000);
	for (int i = 0; i < len; i++) {
		txBuffer = 0xFF;
		result = HAL_SPI_TransmitReceive(&hspi2, &txBuffer, &rxBuffer, 1, 1000);
		out[i] = rxBuffer;
		count++;
	}
	IMU_CS_OFF();

	return count;
}

void IMU_WriteRegister(uint8_t addr, uint8_t *data, size_t len) {
	uint8_t rxBuffer = 0;
	uint8_t txBuffer = 0;

	uint8_t address = (addr & ~(0x80));

	txBuffer = address;

	IMU_CS_ON();
	HAL_StatusTypeDef result = HAL_SPI_TransmitReceive(&hspi2, &txBuffer, &rxBuffer, 1, 1000);
	for (int i = 0; i < len; i++) {
		txBuffer = data[i];
		result = HAL_SPI_TransmitReceive(&hspi2, &txBuffer, &rxBuffer, 1, 1000);
	}
	IMU_CS_OFF();
}

void IMU_reset() {
	uint8_t data = 0;
	IMU_ReadRegister(IMU_PWR_MGMT_1, &data, 1);

	// enable device reset
	data = data | (0x80);
	IMU_WriteRegister(IMU_PWR_MGMT_1, &data, 1);

	sprintf(imu_sbuff, "b: PWR_MGMT_1 - %x\r\n", data);
	// CDC_Transmit_FS(imu_sbuff, strlen(imu_sbuff));

	for (int i = 0; i < 5; i++) {
		HAL_Delay(5);
		IMU_ReadRegister(IMU_PWR_MGMT_1, &data, 1);
		if ((data & 0x80) == 0) {
			// reset bit is cleared -> reset is done
			break;
		}
	}

	sprintf(imu_sbuff, "a: PWR_MGMT_1 - %x\r\n", data);
	// CDC_Transmit_FS(imu_sbuff, strlen(imu_sbuff));

}

void IMU_exitSleep() {
	uint8_t data = 0;
	IMU_ReadRegister(IMU_PWR_MGMT_1, &data, 1);

	sprintf(imu_sbuff, "b: PWR_MGMT_1 - %x\r\n", data);
	// CDC_Transmit_FS(imu_sbuff, strlen(imu_sbuff));

	// clear sleep
	data = data & ~0x40;
	IMU_WriteRegister(IMU_PWR_MGMT_1, &data, 1);
	IMU_ReadRegister(IMU_PWR_MGMT_1, &data, 1);

	sprintf(imu_sbuff, "a: PWR_MGMT_1 - %x\r\n", data);
	// CDC_Transmit_FS(imu_sbuff, strlen(imu_sbuff));

}

void IMU_setClockSource(uint8_t clkSel) {
	uint8_t data = 0;
	IMU_ReadRegister(IMU_PWR_MGMT_1, &data, 1);

	sprintf(imu_sbuff, "b: PWR_MGMT_1 - %x\r\n", data);
	// CDC_Transmit_FS(imu_sbuff, strlen(imu_sbuff));

	// set clock selection
	data = (data & ~(0b111)) | (0b111 & clkSel);

	IMU_WriteRegister(IMU_PWR_MGMT_1, &data, 1);
	IMU_ReadRegister(IMU_PWR_MGMT_1, &data, 1);

	sprintf(imu_sbuff, "a: PWR_MGMT_1 - %x\r\n", data);
	// CDC_Transmit_FS(imu_sbuff, strlen(imu_sbuff));
}

uint8_t IMU_readID() {
	uint8_t data;
	IMU_ReadRegister(IMU_WHO_AM_I, &data, 1);

	sprintf(imu_sbuff, "b: PWR_MGMT_1 - %x\r\n", data);
	// CDC_Transmit_FS(imu_sbuff, strlen(imu_sbuff));

	return data;
}

void IMU_enableSensor(uint8_t sensorBit) {
	uint8_t sensor = 0x7F & sensorBit;

	uint8_t fifo;
	uint8_t pwrReg1;
	uint8_t pwrReg2;

	IMU_ReadRegister(IMU_FIFO_EN, &fifo, 1);
	IMU_ReadRegister(IMU_PWR_MGMT_1, &pwrReg1, 1);
	IMU_ReadRegister(IMU_PWR_MGMT_2, &pwrReg2, 1);

	uint8_t tempDis = (sensor & (1 << 6)) ? 0 : ((pwrReg1 & (1 << 3)));
	uint8_t STBY_ZG = (sensor & (1 << 0)) ? 0 : ((pwrReg2 & (1 << 0)));
	uint8_t STBY_YG = (sensor & (1 << 1)) ? 0 : ((pwrReg2 & (1 << 1)));
	uint8_t STBY_XG = (sensor & (1 << 2)) ? 0 : ((pwrReg2 & (1 << 2)));
	uint8_t STBY_ZA = (sensor & (1 << 3)) ? 0 : ((pwrReg2 & (1 << 3)));
	uint8_t STBY_YA = (sensor & (1 << 4)) ? 0 : ((pwrReg2 & (1 << 4)));
	uint8_t STBY_XA = (sensor & (1 << 5)) ? 0 : ((pwrReg2 & (1 << 5)));
	uint8_t FIFO_LP_EN = 0;

	pwrReg1 = (pwrReg1 & ~(1 << 3)) | tempDis;
	pwrReg2 = (FIFO_LP_EN << 7) | STBY_XA | STBY_YA | STBY_ZA | STBY_XG | STBY_YG | STBY_ZG;

	bool temp = (sensor & (1 << 6)) ? true : (fifo & (1 << 7));
	bool gz = (sensor & (1 << 0)) ? true : (fifo & (1 << 4));
	bool gy = (sensor & (1 << 1)) ? true : (fifo & (1 << 5));
	bool gx = (sensor & (1 << 2)) ? true : (fifo & (1 << 6));

	bool a = ((sensor & (1 << 3)) | (sensor & (1 << 4)) | (sensor & (1 << 5))) ?
	true :
																					(fifo & (1 << 3));

	if (dataModeFIFO) {
		IMU_enableFIFO(temp, gx, gy, gz, a);
	}
	else {
		IMU_enableFIFO(false, false, false, false, false);
	}

	IMU_WriteRegister(IMU_PWR_MGMT_1, &pwrReg1, 1);
	IMU_WriteRegister(IMU_PWR_MGMT_2, &pwrReg2, 1);
}

void IMU_disableSensor(uint8_t sensorBit) {
	uint8_t sensor = 0x7F & sensorBit;

	uint8_t fifo;
	uint8_t pwrReg1;
	uint8_t pwrReg2;

	IMU_ReadRegister(IMU_FIFO_EN, &fifo, 1);
	IMU_ReadRegister(IMU_PWR_MGMT_1, &pwrReg1, 1);
	IMU_ReadRegister(IMU_PWR_MGMT_2, &pwrReg2, 1);

	uint8_t tempDis = (sensor & (1 << 6)) ? 1 : ((pwrReg1 & (1 << 3)));
	uint8_t STBY_ZG = (sensor & (1 << 0)) ? 1 : ((pwrReg2 & (1 << 0)));
	uint8_t STBY_YG = (sensor & (1 << 1)) ? 1 : ((pwrReg2 & (1 << 1)));
	uint8_t STBY_XG = (sensor & (1 << 2)) ? 1 : ((pwrReg2 & (1 << 2)));
	uint8_t STBY_ZA = (sensor & (1 << 3)) ? 1 : ((pwrReg2 & (1 << 3)));
	uint8_t STBY_YA = (sensor & (1 << 4)) ? 1 : ((pwrReg2 & (1 << 4)));
	uint8_t STBY_XA = (sensor & (1 << 5)) ? 1 : ((pwrReg2 & (1 << 5)));
	uint8_t FIFO_LP_EN = 0;

	pwrReg1 = (pwrReg1 & ~(1 << 3)) | tempDis;
	pwrReg2 = (FIFO_LP_EN << 7) | STBY_XA | STBY_YA | STBY_ZA | STBY_XG | STBY_YG | STBY_ZG;

	bool temp = (sensor & (1 << 6)) ? true : (fifo & (1 << 7));
	bool gz = (sensor & (1 << 0)) ? true : (fifo & (1 << 4));
	bool gy = (sensor & (1 << 1)) ? true : (fifo & (1 << 5));
	bool gx = (sensor & (1 << 2)) ? true : (fifo & (1 << 6));

	bool a = ((sensor & (1 << 3)) | (sensor & (1 << 4)) | (sensor & (1 << 5))) ?
	true :
																					(fifo & (1 << 3));

	tempDis = ((sensor & (1 << 0)) | (sensor & (1 << 1)) | (sensor & (1 << 2))) ? 0 : tempDis;
	pwrReg1 = (pwrReg1 & ~(1 << 3)) | tempDis;

	temp = ((sensor & (1 << 0)) | (sensor & (1 << 1)) | (sensor & (1 << 2))) ?
	true :
																				temp;

	if (dataModeFIFO) {
		IMU_enableFIFO(temp, gx, gy, gz, a);
	}
	else {
		IMU_enableFIFO(false, false, false, false, false);
	}

	IMU_WriteRegister(IMU_PWR_MGMT_1, &pwrReg1, 1);
	IMU_WriteRegister(IMU_PWR_MGMT_2, &pwrReg2, 1);

}

bool IMU_enableFIFO(bool temp, bool gx, bool gy, bool gz, bool accel) {
	uint8_t fifoReg;
	uint8_t usrCtrlReg;
	uint8_t configReg;

	IMU_ReadRegister(IMU_CONFIG, &configReg, 1);
	IMU_ReadRegister(IMU_FIFO_EN, &fifoReg, 1);
	IMU_ReadRegister(IMU_USER_CTRL, &usrCtrlReg, 1);

	uint8_t FIFO_EN;
	uint8_t FIFO_RST;
	uint8_t SIG_COND_RST;
	uint8_t FIFO_MODE;

	if (temp || gx || gy || gz || accel) {
		FIFO_EN = 1;
		FIFO_RST = 1;
		SIG_COND_RST = 1;
		FIFO_MODE = 0;
	}
	else {
		FIFO_EN = 0;
		FIFO_RST = 1;
		SIG_COND_RST = 1;
		FIFO_MODE = 0;
	}

	usrCtrlReg = (usrCtrlReg & ~(1 << 6)) | (FIFO_EN << 6);
	usrCtrlReg = (usrCtrlReg & ~(1 << 2)) | (FIFO_RST << 2);
	usrCtrlReg = (usrCtrlReg & ~(1 << 0)) | (SIG_COND_RST << 0);

	configReg = (configReg & ~(1 << 6)) | (FIFO_MODE << 6);

	fifoReg = (fifoReg & ~(1 << 3)) | (accel << 3);
	fifoReg = (fifoReg & ~(1 << 3)) | (gz << 3);
	fifoReg = (fifoReg & ~(1 << 5)) | (gy << 5);
	fifoReg = (fifoReg & ~(1 << 6)) | (gx << 6);
	fifoReg = (fifoReg & ~(1 << 7)) | (temp << 7);

	IMU_WriteRegister(IMU_FIFO_EN, &fifoReg, 1);
	IMU_WriteRegister(IMU_CONFIG, &configReg, 1);
	IMU_WriteRegister(IMU_USER_CTRL, &usrCtrlReg, 1);
	IMU_ReadRegister(IMU_FIFO_EN, &fifoReg, 1);

	accel = fifoReg & (1 << 3);
	gz = fifoReg & (1 << 4);
	gy = fifoReg & (1 << 5);
	gx = fifoReg & (1 << 6);
	temp = fifoReg & (1 << 7);

	fifoFrameSize = (accel * 6) + (temp + gx + gy + gz) * 2;

	return true;
}

void IMU_configAccel(uint8_t scale, uint8_t bd, uint8_t *odr, bool lowPowerFlag) {
	if (lowPowerFlag) {
		powerMode = IMU_PWR_MODE_ACCEL_LOWPOWER;
	}
	else {
		powerMode = IMU_PWR_MODE_6AXIS_LOWNOISE;
	}

	IMU_setFullScaleForAccel(scale);
	IMU_setBandwidthForAccel(bd);
	IMU_WriteRegister(IMU_LP_MODE_CFG, odr, 1);
}

void IMU_setFullScaleForAccel(uint8_t scale) {
	uint8_t accelConfigReg;
	scale &= 0x03;

	IMU_ReadRegister(IMU_ACCEL_CONFIG, &accelConfigReg, 1);

	sprintf(imu_sbuff, "b: ACCEL_CONFIG - %x\r\n", accelConfigReg);
	// CDC_Transmit_FS(imu_sbuff, strlen(imu_sbuff));

	accelConfigReg = (accelConfigReg & ~(0b11 << 3)) | ((scale & 0b11) << 3);

	// clear x,y,z self test
	accelConfigReg = (accelConfigReg & ~(0b111 << 5));

	IMU_WriteRegister(IMU_ACCEL_CONFIG, &accelConfigReg, 1);
	IMU_ReadRegister(IMU_ACCEL_CONFIG, &accelConfigReg, 1);

	sprintf(imu_sbuff, "a: ACCEL_CONFIG - %x\r\n", accelConfigReg);
	// CDC_Transmit_FS(imu_sbuff, strlen(imu_sbuff));

	switch (scale) {
	case IMU_FSR_A_2G:
		accelScale = IMU_ADC_MAX_RANGE / IMU_ACCEL_FSR_2G;
		accelRange = IMU_ACCEL_FSR_2G;
		break;
	case IMU_FSR_A_4G:
		accelScale = IMU_ADC_MAX_RANGE / IMU_ACCEL_FSR_4G;
		accelRange = IMU_ACCEL_FSR_4G;
		break;
	case IMU_FSR_A_8G:
		accelScale = IMU_ADC_MAX_RANGE / IMU_ACCEL_FSR_8G;
		accelRange = IMU_ACCEL_FSR_8G;
		break;
	case IMU_FSR_A_16G:
		accelScale = IMU_ADC_MAX_RANGE / IMU_ACCEL_FSR_16G;
		accelRange = IMU_ACCEL_FSR_16G;
		break;
	default:
		accelScale = 1;
		accelRange = 1;
		break;
	}

}

void IMU_setBandwidthForAccel(uint8_t bandwidth) {
	uint8_t pwrReg;
	IMU_ReadRegister(IMU_PWR_MGMT_1, &pwrReg, 1);

	switch (powerMode) {
	case IMU_PWR_MODE_SLEEP:
		break;
	case IMU_PWR_MODE_STANDBY:
		break;
	case IMU_PWR_MODE_ACCEL_LOWPOWER:
	case IMU_PWR_MODE_ACCEL_LOWNOISE:
		// set cycle
		pwrReg = pwrReg | (1 << 5);
		IMU_setBandwidthForAccelInLowPowerMode(bandwidth);
		break;
	case IMU_PWR_MODE_6AXIS_LOWNOISE:
		// clear cycle
		pwrReg = pwrReg & ~(1 << 5);
		// clear gyro standby
		pwrReg = pwrReg & ~(1 << 4);
		IMU_setBandwidthForAccelInOthersMode(bandwidth);
		break;
	}

	IMU_WriteRegister(IMU_PWR_MGMT_1, &pwrReg, 1);
}

void IMU_setBandwidthForAccelInLowPowerMode(uint8_t bandwidth) {
	uint8_t reg;
	IMU_ReadRegister(IMU_ACCEL_CONFIG2, &reg, 1);

	sprintf(imu_sbuff, "b: ACCEL_CONFIG2 - %x\r\n", reg);
	// CDC_Transmit_FS(imu_sbuff, strlen(imu_sbuff));

	// clear FIFO size
	reg = reg & ~(0b11 << 6);

	// clear DEC2 config
	reg = reg & ~(0b11 << 4);

	switch (bandwidth) {
	case IMU_ACCEL_DLPF_218_1KHZ:
		reg = (reg & ~(1 << 3)) | (0 << 3);
		reg = (reg & ~(0b111)) | 7;
		reg = reg | ((1) << 4);
		break;
	case IMU_ACCEL_DLPF_420_1KHZ:
		reg = (reg & ~(1 << 3)) | (0 << 3);
		reg = (reg & ~(0b111)) | 7;
		reg = reg | ((0) << 4);
		break;
	case IMU_ACCEL_DLPF_1046_4KHZ:
		reg = (reg & ~(1 << 3)) | (1 << 3);
		break;
	case IMU_ACCEL_DLPF_55_1KHZ:
		reg = (reg & ~(1 << 3)) | (0 << 3);
		reg = (reg & ~(0b111)) | 7;
		reg = reg | ((3) << 4);
		break;
	case IMU_ACCEL_DLPF_110_1KHZ:
		reg = (reg & ~(1 << 3)) | (0 << 3);
		reg = (reg & ~(0b111)) | 7;
		reg = reg | ((2) << 4);
		break;
	}

	IMU_WriteRegister(IMU_ACCEL_CONFIG2, &reg, 1);
	IMU_ReadRegister(IMU_ACCEL_CONFIG2, &reg, 1);

	sprintf(imu_sbuff, "a: ACCEL_CONFIG2 - %x\r\n", reg);
	// CDC_Transmit_FS(imu_sbuff, strlen(imu_sbuff));

}

void IMU_setBandwidthForAccelInOthersMode(uint8_t bandwidth) {
	uint8_t reg;
	IMU_ReadRegister(IMU_ACCEL_CONFIG2, &reg, 1);

	sprintf(imu_sbuff, "b: ACCEL_CONFIG2 - %x\r\n", reg);
	// CDC_Transmit_FS(imu_sbuff, strlen(imu_sbuff));

	// clear FIFO size
	reg = reg & ~(0b11 << 6);

	// clear DEC2 config
	reg = reg & ~(0b11 << 4);

	switch (bandwidth) {
	case IMU_ACCEL_DLPF_5_1KHZ:
		// fchoice b
		reg = (reg & ~(1 << 3)) | (0 << 3);
		// dlpf cfg
		reg = (reg & ~(0b111)) | 6;
		break;
	case IMU_ACCEL_DLPF_10_1KHZ:
		// fchoice b
		reg = (reg & ~(1 << 3)) | (0 << 3);
		// dlpf cfg
		reg = (reg & ~(0b111)) | 5;
		break;
	case IMU_ACCEL_DLPF_21_1KHZ:
		// fchoice b
		reg = (reg & ~(1 << 3)) | (0 << 3);
		// dlpf cfg
		reg = (reg & ~(0b111)) | 4;
		break;
	case IMU_ACCEL_DLPF_44_1KHZ:
		// fchoice b
		reg = (reg & ~(1 << 3)) | (0 << 3);
		// dlpf cfg
		reg = (reg & ~(0b111)) | 3;
		break;
	case IMU_ACCEL_DLPF_99_1KHZ:
		// fchoice b
		reg = (reg & ~(1 << 3)) | (0 << 3);
		// dlpf cfg
		reg = (reg & ~(0b111)) | 2;
		break;
	case IMU_ACCEL_DLPF_218_1KHZ:
		// fchoice b
		reg = (reg & ~(1 << 3)) | (0 << 3);
		// dlpf cfg
		reg = (reg & ~(0b111)) | 1;
		break;
	case IMU_ACCEL_DLPF_420_1KHZ:
		// fchoice b
		reg = (reg & ~(1 << 3)) | (0 << 3);
		// dlpf cfg
		reg = (reg & ~(0b111)) | 7;
		break;
	case IMU_ACCEL_DLPF_1046_4KHZ:
		// fchoice b
		reg = (reg & ~(1 << 3)) | (1 << 3);
		break;
	}

	IMU_WriteRegister(IMU_ACCEL_CONFIG2, &reg, 1);
	IMU_ReadRegister(IMU_ACCEL_CONFIG2, &reg, 1);

	sprintf(imu_sbuff, "a: ACCEL_CONFIG2 - %x\r\n", reg);
	// CDC_Transmit_FS(imu_sbuff, strlen(imu_sbuff));
}

void IMU_configGyro(uint8_t scale, uint8_t bd) {
	IMU_setFullScaleForGyro(scale);
	IMU_setBandwidthForGyro(bd);
}

void IMU_setFullScaleForGyro(uint8_t scale) {
	scale &= 0x03;
	uint8_t reg;
	IMU_ReadRegister(IMU_GYRO_CONFIG, &reg, 1);
	sprintf(imu_sbuff, "b: GYRO_CONFIG - %x\r\n", reg);

	reg = (reg & ~(0b11 << 3)) | ((scale & 0b11) << 3);

	// clear x,y,z self test
	reg = (reg & ~(0b111 << 5));

	IMU_WriteRegister(IMU_GYRO_CONFIG, &reg, 1);
	IMU_ReadRegister(IMU_GYRO_CONFIG, &reg, 1);

	sprintf(imu_sbuff, "a: GYRO_CONFIG - %x\r\n", reg);

	switch (scale) {
	case IMU_GYRO_125DPS:
		gyroRange = IMU_GYRO_FULL_SCALE_125DPS;
		break;
	case IMU_GYRO_250DPS:
		gyroRange = IMU_GYRO_FULL_SCALE_250DPS;
		break;
	case IMU_GYRO_500DPS:
		gyroRange = IMU_GYRO_FULL_SCALE_500DPS;
		break;
	}

	gyroScale = IMU_ADC_MAX_RANGE / gyroRange;

}

void IMU_setBandwidthForGyro(uint8_t bandwidth) {
	uint8_t reg;
	uint8_t gyroReg;
	IMU_ReadRegister(IMU_GYRO_CONFIG, &gyroReg, 1);
	IMU_ReadRegister(IMU_CONFIG, &reg, 1);

	sprintf(imu_sbuff, "b: GYRO_CONFIG - %x\r\n", gyroReg);
	// CDC_Transmit_FS(imu_sbuff, strlen(imu_sbuff));

	// clear FIFO size
	reg = reg & ~(0b11 << 6);

	// clear DEC2 config
	reg = reg & ~(0b11 << 4);

	switch (bandwidth) {
	case IMU_GYRO_DLPF_8173_32KHZ:
		gyroReg = ((gyroReg & ~0b11) | (1 << 0));
		break;
	case IMU_GYRO_DLPF_3281_32KHZ:
		gyroReg = ((gyroReg & ~0b11) | ((1 << 1) | (1 << 0)));
		break;
	case IMU_GYRO_DLPF_250_8KHZ:
		gyroReg = ((gyroReg & ~0b11) | (0 << 1));
		reg = ((reg & ~0b111) | 0);
		break;
	case IMU_GYRO_DLPF_176_1KHZ:
		gyroReg = ((gyroReg & ~0b11) | (0 << 1));
		reg = ((reg & ~0b111) | 1);
		break;
	case IMU_GYRO_DLPF_92_1KHZ:
		gyroReg = ((gyroReg & ~0b11) | (0 << 1));
		reg = ((reg & ~0b111) | 2);
		break;
	case IMU_GYRO_DLPF_3281_8KHZ:
		gyroReg = ((gyroReg & ~0b11) | (0 << 1));
		reg = ((reg & ~0b111) | 7);
		break;
	}

	IMU_WriteRegister(IMU_GYRO_CONFIG, &gyroReg, 1);
	IMU_WriteRegister(IMU_CONFIG, &reg, 1);

	sprintf(imu_sbuff, "a: GYRO_CONFIG - %x\r\n", gyroReg);
	// CDC_Transmit_FS(imu_sbuff, strlen(imu_sbuff));

}

void IMU_Init(bool dataMode_FIFO) {
	dataModeFIFO = dataMode_FIFO;
	fifoFrameSize = 14;
	updateData = 0;

	IMU_CS_OFF();

	IMU_reset();

	// delay for 100ms, but refresh watchdog every 15ms
	for (int i = 0; i < 7; i++) {
		HAL_Delay(15);

		// refresh watchdogs
		//watchdog_refresh();
	}

	IMU_exitSleep();

	// delay for 100ms, but refresh watchdog every 15ms
	for (int i = 0; i < 7; i++) {
		HAL_Delay(15);

		// reset watchdogs
		//watchdog_refresh();
	}

	IMU_setClockSource(IMU_CLOCK_SEL_PLL);
	uint8_t ID = IMU_readID();

	if (ID != ICG20660L_DEVICE_ID) {
		sprintf(imu_sbuff, "BAD IMU ID: - %x\r\n", ID);
		// CDC_Transmit_FS(imu_sbuff, strlen(imu_sbuff));
	}

	IMU_disableSensor(IMU_SENSOR_E_AXIS_ALL);

	uint8_t odr;
	IMU_configAccel(IMU_FSR_A_16G, IMU_ACCEL_DLPF_218_1KHZ, &odr, false);
	IMU_configGyro(IMU_GYRO_500DPS, IMU_GYRO_DLPF_176_1KHZ);
}

void IMU_setSampleDiv(uint8_t div) {
	IMU_WriteRegister(IMU_SMPLRT_DIV, &div, 1);
}

float IMU_getAccelDataX() {
	if (!(updateData & 0x01)) {
		IMU_getRawData(NULL, 0);
	}

	updateData &= 0xFE;

	return ((int16_t) (rawData[IMU_RAW_DATA_AX_H_INDEX] << 8 | rawData[IMU_RAW_DATA_AX_L_INDEX])) / accelScale;
}

float IMU_getAccelDataY() {
	if (!(updateData & 0x02)) {
		IMU_getRawData(NULL, 0);
	}

	updateData &= 0xFD;

	return ((int16_t) (rawData[IMU_RAW_DATA_AY_H_INDEX] << 8 | rawData[IMU_RAW_DATA_AY_L_INDEX])) / accelScale;
}

float IMU_getAccelDataZ() {
	if (!(updateData & 0x04)) {
		IMU_getRawData(NULL, 0);
	}

	updateData &= 0xFB;

	return ((int16_t) (rawData[IMU_RAW_DATA_AZ_H_INDEX] << 8 | rawData[IMU_RAW_DATA_AZ_L_INDEX])) / accelScale;
}

float IMU_getGyroDataX() {
	if (!(updateData & 0x10)) {
		IMU_getRawData(NULL, 0);
	}
	updateData &= 0xEF;
	return ((int16_t) (rawData[IMU_RAW_DATA_GX_H_INDEX] << 8 | rawData[IMU_RAW_DATA_GX_L_INDEX])) / gyroScale;
}

float IMU_getGyroDataY() {
	if (!(updateData & 0x20)) {
		IMU_getRawData(NULL, 0);
	}
	updateData &= 0xDF;

	return ((int16_t) (rawData[IMU_RAW_DATA_GY_H_INDEX] << 8 | rawData[IMU_RAW_DATA_GY_L_INDEX])) / gyroScale;

}

float IMU_getGyroDataZ() {
	if (!(updateData & 0x40)) {
		IMU_getRawData(NULL, 0);
	}
	updateData &= 0xBF;
	return ((int16_t) (rawData[IMU_RAW_DATA_GZ_H_INDEX] << 8 | rawData[IMU_RAW_DATA_GZ_L_INDEX])) / gyroScale;

}

float IMU_getTemperatureC() {
	if (!(updateData & 0x08)) {
		IMU_getRawData(NULL, 0);
	}

	updateData &= 0xF7;

	return ((int16_t) (rawData[IMU_RAW_DATA_T_H_INDEX] << 8 | rawData[IMU_RAW_DATA_T_L_INDEX])) / 326.8 + 23;
}

void IMU_readDataFromFIFO() {
	uint8_t fifoEnReg;

	uint8_t val[2] = { 0, 0 };
	uint8_t rslt[fifoFrameSize];
	uint16_t size;
	uint8_t count = 0;

	memset(rslt, 0, fifoFrameSize);

	IMU_ReadRegister(IMU_FIFO_EN, &fifoEnReg, 1);
	IMU_ReadRegister(IMU_FIFO_COUNTH, val, 2);
	size = ((val[0] & 0x1F) << 8) | val[1];

	if (size >= fifoFrameSize) {
		IMU_ReadRegister(IMU_FIFO_R_W, rslt, fifoFrameSize);

		if ((fifoEnReg & (1 << 7)) && (count < fifoFrameSize)) {
			rawData[IMU_RAW_DATA_T_H_INDEX] = rslt[count++];
			rawData[IMU_RAW_DATA_T_L_INDEX] = rslt[count++];
		}

		if ((fifoEnReg & (1 << 6)) && (count < fifoFrameSize)) {
			rawData[IMU_RAW_DATA_GX_H_INDEX] = rslt[count++];
			rawData[IMU_RAW_DATA_GX_L_INDEX] = rslt[count++];
		}

		if ((fifoEnReg & (1 << 5)) && (count < fifoFrameSize)) {
			rawData[IMU_RAW_DATA_GY_H_INDEX] = rslt[count++];
			rawData[IMU_RAW_DATA_GY_L_INDEX] = rslt[count++];
		}

		if ((fifoEnReg & (1 << 4)) && (count < fifoFrameSize)) {
			rawData[IMU_RAW_DATA_GZ_H_INDEX] = rslt[count++];
			rawData[IMU_RAW_DATA_GZ_L_INDEX] = rslt[count++];
		}

		if ((fifoEnReg & (1 << 3)) && (count < fifoFrameSize)) {
			rawData[IMU_RAW_DATA_AX_H_INDEX] = rslt[count++];
			rawData[IMU_RAW_DATA_AX_L_INDEX] = rslt[count++];
			rawData[IMU_RAW_DATA_AY_H_INDEX] = rslt[count++];
			rawData[IMU_RAW_DATA_AY_L_INDEX] = rslt[count++];
			rawData[IMU_RAW_DATA_AZ_H_INDEX] = rslt[count++];
			rawData[IMU_RAW_DATA_AZ_L_INDEX] = rslt[count++];

		}
	}
}

void IMU_readDataFromREG() {
	uint8_t value[14] = { 0 };
	IMU_ReadRegister(IMU_ACCEL_XOUT_H, value, 14);

	rawData[IMU_RAW_DATA_AX_H_INDEX] = value[0];
	rawData[IMU_RAW_DATA_AX_L_INDEX] = value[1];
	rawData[IMU_RAW_DATA_AY_H_INDEX] = value[2];
	rawData[IMU_RAW_DATA_AY_L_INDEX] = value[3];
	rawData[IMU_RAW_DATA_AZ_H_INDEX] = value[4];
	rawData[IMU_RAW_DATA_AZ_L_INDEX] = value[5];
	rawData[IMU_RAW_DATA_T_H_INDEX] = value[6];
	rawData[IMU_RAW_DATA_T_L_INDEX] = value[7];
	rawData[IMU_RAW_DATA_GX_H_INDEX] = value[8];
	rawData[IMU_RAW_DATA_GX_L_INDEX] = value[9];
	rawData[IMU_RAW_DATA_GY_H_INDEX] = value[10];
	rawData[IMU_RAW_DATA_GY_L_INDEX] = value[11];
	rawData[IMU_RAW_DATA_GZ_H_INDEX] = value[12];
	rawData[IMU_RAW_DATA_GZ_L_INDEX] = value[13];
}

void IMU_getRawData(uint8_t *data, uint8_t len) {
	memset(&rawData, 0, IMU_RAW_DATA_LENGTH);

	if (dataModeFIFO) {
		IMU_readDataFromFIFO();
	}
	else {
		IMU_readDataFromREG();
	}

	if (data != NULL) {
		memcpy(data, rawData, len);
	}

	updateData = 0x7F;
}

uint16_t IMU_getAccelScaler() {
	return (uint16_t) (accelRange);
}

uint16_t IMU_getGyroScaler() {
	return (uint16_t) (gyroRange);
}
