/*
 * rtd.c
 *
 *  Created on: Jan 18, 2022
 *      Author: Calvin
 */

#include "rtd.h"

#include "12vSW.h"
#include "ads8668.h"

#include "can.h"
#include <CAN_VCU.h>

ms_timer_t timer_rtd;
ms_timer_t timer_rtd_siren;
ms_timer_t timer_horn;

RTD_t RTD_state;

void rtd_horn_setup() {
	rtd_siren_off();
	timer_horn = timer_init(100, false, rtd_horn_cb);
}

void rtd_horn_start() {
	rtd_siren_on();
	timer_start(&timer_horn);
}

void rtd_horn_cb(void *args) {
	rtd_siren_off();
}

void rtd_setup() {
	ADS8668_FilterEnable(ADC_CH_RTD_BTN);
	ADS8668_SetRange(ADC_CH_RTD_BTN, ADS8668_RANGE_5V12);

	timer_rtd = timer_init(500, true, rtd_timer_cb);
	timer_rtd_siren = timer_init(250, true, rtd_siren_cb);
}

void rtd_timer_on() {
	rtd_light_on();

	timer_start(&timer_rtd);
}

void rtd_timer_off() {
	timer_stop(&timer_rtd);

	rtd_light_off();
}

void rtd_light_on() {
	SW_setState(RTD_IDX_LIGHT, true);
	RTD_state.rtd_btn_state = true;
}

void rtd_light_off() {
	SW_setState(RTD_IDX_LIGHT, false);
	RTD_state.rtd_btn_state = false;
}

bool rtd_btn_read() {
	return ADS8668_GetScaledFiltered(ADC_CH_RTD_BTN) < 2000;
}

void rtd_siren_on() {
	SW_setState(RTD_IDX_SIREN, true);
}

void rtd_siren_off() {
	SW_setState(RTD_IDX_SIREN, false);
}

void rtd_timer_cb(void *args) {
	if (RTD_state.rtd_btn_state) {
		rtd_light_off();
	} else {
		rtd_light_on();
	}
}

void rtd_siren_start() {
	timer_start(&timer_rtd_siren);

	RTD_state.siren_active = true;
	RTD_state.siren_start = HAL_GetTick();

	rtd_siren_on();
}

void rtd_siren_stop() {
	RTD_state.siren_active = false;
	rtd_siren_off();

	timer_stop(&timer_rtd_siren);
}

void rtd_siren_cb(void *args) {
	if (RTD_state.siren_active) {
		if ((HAL_GetTick() - RTD_state.siren_start) > RTD_SIREN_TIME) {
			RTD_state.siren_active = false;
			rtd_siren_off();

			timer_stop(&timer_rtd_siren);
		} else {
			rtd_siren_on();
		}
	} else {
		rtd_siren_off();
	}
}

void rtd_broadcast() {
	VCU_RTD_t msgRTD = Compose_VCU_RTD();

	CAN_TxHeaderTypeDef header = { .ExtId = msgRTD.id, .IDE =
		CAN_ID_EXT, .RTR = CAN_RTR_DATA, .DLC = 0, .TransmitGlobalTime = DISABLE };

	send_can_msg(&hcan1, &header, NULL);
}

