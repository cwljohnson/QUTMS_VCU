/*
 * p_adc.c
 *
 *  Created on: 3 Feb. 2022
 *      Author: Calvin
 */

#include "p_adc.h"
#include "ads8668.h"

#include "main.h"

ms_timer_t timer_adc;

bool setup_ADC() {
	ADS8668_Init();

	//ADS8668_Background_Enable();

	ADS8668_ReadAll();

	for (int i = 0; i < 8; i++) {
		window_filter_initialize(&adc_filtered[i], adc_readings_raw[i], ADC_FILTER_SIZE);
		window_filter_initialize(&adc_period_filtered[i], 0, ADC_FILTER_SIZE);
	}

	timer_adc = timer_init(4, true, adc_timer_cb);

	timer_start(&timer_adc);

	return true;
}

void adc_timer_cb(void *args) {
	ADS8668_ReadAll();

}
