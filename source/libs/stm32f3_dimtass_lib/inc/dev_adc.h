/*
 * dev_adc.h
 *
 * Copyright 2018 Dimitris Tassopoulos
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 * 
 * This is a custom library for the ADCs of the stm32f103c8
 * 
 * Created on: 14 May 2018
 * Author: Dimitris Tassopoulos <dimtass@gmail.com>
 * 
 * Usage:
 * // Decalre callback
 * void adc_cbk(struct adc_channel * adc, uint16_t value);
 * // Declare channel
 * DECLARE_ADC_CH(adc_temp, ADC_Channel_0, GPIOA, GPIO_Pin_0);
 * 
 * // Initialize adc in main:
 * 	adc_module_init();
 *	ADC_TempSensorVrefintCmd(ENABLE);
 *	adc_add_channel(&adc_temp);
 *	adc_start();
 *
 * // callback example
 * void adc_cbk(struct adc_channel * adc, uint16_t value)
 * {
 * }
 */

#ifndef DEV_ADC_H_
#define DEV_ADC_H_

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "stm32f30x.h"
#include "debug_trace.h"

struct adc_channel;

#define ADC_CH(OWNER, CHANNEL, ENABLE) { OWNER, CHANNEL, ENABLE, 0, 0 }

#define DECLARE_ADC_CH(NAME, CHANNEL, PORT, PIN) \
	struct adc_channel NAME = { \
		.channel = CHANNEL, \
		.index = 0, \
		.port = PORT, \
		.pin = PIN, \
	}

struct adc_channel {
	uint8_t	channel;			/* the ADC channel */
	uint8_t index;				/* this is the index in the ADC DMA buffer */
	volatile uint16_t value;

    GPIO_TypeDef *      port;   /* ADC channel port */
    uint16_t            pin;    /* ADC channel pin */
};

void adc_module_init(void);
void adc_start(void);
void adc_stop(void);
void adc_add_channel(struct adc_channel * channel);
void adc_del_channel(struct adc_channel * channel);
uint16_t adc_get_value(uint8_t channel);
uint8_t adc_get_num_of_channels(void);

#endif /* DEV_ADC_H_ */
