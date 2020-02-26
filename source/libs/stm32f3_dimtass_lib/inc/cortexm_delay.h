/*
 * stm32_delay.h
 *
 * This delay driver is for cortex-M cores that support DWT.
 * DWT is the data watchpoint and trace unit, but it can be
 * used as a precision timer with it's own resources.
 *
 * Be aware that this is not supported in old CMSIS libraries,
 * therefore you need to get one of the latest ones.
 *
 * Free of use blah, blah, MIT, harvard e.t.c.
 *
 *  Created on: Jul 9, 2018
 *      Author: Dimitris Tassopoulos
 */

#ifndef CORTEXM_DELAY_H_
#define CORTEXM_DELAY_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "stm32f30x.h"

void delay_init(uint32_t system_core_clock);
void delay_us(uint32_t us);
void delay_ms(uint32_t ms);

#ifdef __cplusplus
}
#endif

#endif /* CORTEXM_DELAY_H_ */
