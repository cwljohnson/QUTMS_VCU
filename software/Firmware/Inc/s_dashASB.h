/*
 * s_dashASB.h
 *
 *  Created on: 1 Dec. 2022
 *      Author: Calvin
 */

#ifndef INC_S_DASHASB_H_
#define INC_S_DASHASB_H_

#include <stdbool.h>

#define ADC_CH_DASH_ASB 1

void dashASB_setup();
void dashASB_setState(bool value);

#endif /* INC_S_DASHASB_H_ */
