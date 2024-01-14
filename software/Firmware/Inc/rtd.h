/*
 * rtd.h
 *
 *  Created on: Jan 18, 2022
 *      Author: Calvin
 */

#ifndef INC_RTD_H_
#define INC_RTD_H_

#include <Timer.h>
#include <stdbool.h>

#define PRECHARGE_BTN_TIME 200
#define RTD_BTN_TIME 5000
#define DVL_PRECHARGE_BTN_TIME 2000

#define RTD_IDX_LIGHT 0
#define RTD_IDX_SIREN 1

#define RTD_SIREN_TIME 2000

#define ADC_CH_RTD_BTN 6

typedef struct RTD {
	uint32_t precharge_ticks;
	uint32_t RTD_ticks;

	bool rtd_btn_state;

	uint32_t siren_start;
	bool siren_active;
} RTD_t;

extern RTD_t RTD_state;
extern ms_timer_t timer_rtd;
extern ms_timer_t timer_rtd_siren;
extern ms_timer_t timer_horn;

void rtd_timer_cb(void *args);

void rtd_horn_setup();
void rtd_horn_start();
void rtd_horn_cb(void *args);

void rtd_setup();
void rtd_timer_on();
void rtd_timer_off();

void rtd_light_on();
void rtd_light_off();

bool rtd_btn_read();

void rtd_siren_on();
void rtd_siren_off();

void rtd_siren_start();
void rtd_siren_stop();
void rtd_siren_cb(void *args);

void rtd_broadcast();

#endif /* INC_RTD_H_ */
