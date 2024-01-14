/*
 * imu_timer.c
 *
 *  Created on: Oct 13, 2021
 *      Author: Calvin
 */

#include "main.h"
#include "imu.h"
#include "can.h"

#include "p_imu.h"

#include <CAN_VCU.h>

ms_timer_t timer_IMU;

uint8_t imu_odr;

bool setup_IMU() {
/*
	timer_IMU = timer_init(50, true, imu_timer_cb);

	IMU_Init(false);

	IMU_enableSensor(IMU_SENSOR_E_AXIS_ALL);
	IMU_configAccel(IMU_ACCEL_FSR_4G, IMU_ACCEL_DLPF_1046_4KHZ, &imu_odr,
			false);
	IMU_configGyro(IMU_GYRO_250DPS, IMU_GYRO_DLPF_3281_32KHZ);

	IMU_setSampleDiv(19);

	// start timer
	timer_start(&timer_IMU);
*/

	return true;
}

void imu_timer_cb(void *args) {

	CAN_TxHeaderTypeDef header = { .IDE = CAN_ID_EXT, .RTR = CAN_RTR_DATA,
			.TransmitGlobalTime = DISABLE, };

	VCU_IMU_Accel_t accelMsg;
	VCU_IMU_Gyro_t gyroMsg;

	uint16_t accelScale = IMU_getAccelScaler();

	int16_t accel_x = (IMU_getAccelDataX() * IMU_ADC_MAX_RANGE / accelScale);
	int16_t accel_y = (IMU_getAccelDataY() * IMU_ADC_MAX_RANGE / accelScale);
	int16_t accel_z = (IMU_getAccelDataZ() * IMU_ADC_MAX_RANGE / accelScale);

	accelMsg = Compose_VCU_IMU_Accel(VCU_ID, accelScale, accel_x, accel_y,
			accel_z);

	header.ExtId = accelMsg.id;
	header.DLC = sizeof(accelMsg.data);

	send_can_msg(&hcan1, &header, accelMsg.data);

	uint16_t gyroScale = IMU_getGyroScaler();

	int16_t gyro_x = (IMU_getGyroDataX() * IMU_ADC_MAX_RANGE / gyroScale);
	int16_t gyro_y = (IMU_getGyroDataY() * IMU_ADC_MAX_RANGE / gyroScale);
	int16_t gyro_z = (IMU_getGyroDataZ() * IMU_ADC_MAX_RANGE / gyroScale);
	gyroMsg = Compose_VCU_IMU_Gyro(VCU_ID, gyroScale, gyro_x, gyro_y, gyro_z);

	header.ExtId = gyroMsg.id;
	header.DLC = sizeof(gyroMsg.data);

	send_can_msg(&hcan1, &header, gyroMsg.data);
}
