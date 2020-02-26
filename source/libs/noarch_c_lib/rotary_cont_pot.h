/*
 * @file rotary_cont_pot.h
 *
 * This library implements a rotary continuous pot that supports free-spining
 * dual gang pots with 90 degree phase between the two gangs.
 *
 * - Supports ADCs with different bits
 * - Supports floats or uint16_t in systems that don't support floats
 * - Supports individual dead-zones for each pot gang
 * - Support negative ranges
 * - Supports float steps
 *
 * Notes:
 * When updating the pots with the ADC values, make sure that the ADCs are low
 * noise and you're using a low-pass filter to ADC code.
 * For example:
 * 	Add the 2^N values and then shift by N. That means if N=5, then
 * 	you add 2^5=32 values of the adc and then you shift right by 5.
 *
 * Description:
 * This is a small project only for testing a dual gang free-spinning rotary
 * pot I got my hands on. The interesting thing with this pot is that the two
 * wipers (one for each internal resistor) have a 90 degree phase between
 * each other. Therefore, the code logic is based this difference.
 *
 * The beautiful and awesome ascii graphics below show the Vadc on the two
 * wipers of the pot while the pot is turning. So, if you have a 12-bit ADC
 * then the max value will be in the range of [0-4095) ((1<12)-1=4095), which
 * means that min=0 and max=4095. Each wiper is sampled with using a different
 * ADC channel ADC1 and ADC2.
 *
 * The logic of the code is just to detect the direction where the pot is
 * turning and increment its value when it's turned right and decrement
 * when it's turned left. To do this we split the below graph in to 4 main
 * quarters, each for 1/4 of the half period (aka 1 full turn of the pot).
 * With this trick by comparing two sequential ADC values of ADC1 and ADC2
 * we can detect the exact location of the pot and also in which direction
 * is turning.
 *
 * To be more specific, in Q1 we know that ADC1 is in the range [min-1/2]
 * as also ADC2. But by comparing two different ADC samples, then we can
 * detect if the pot is turned right or left. Therefore, in Q1 if ADC1 is
 * increment (+) and ADC2 is decrement (-), then we know that the pot is
 * turned left. And if ADC2=(+) and ADC1=(-) then is turned right.
 *
 * ```
 *            |1|2|3|4|1|2|3|4|
 *       max  _________________
 *                /\      /\
 * ADC2  1/2  ___/__\___ /__\___
 *              /    \  /    \
 *            _/______\/______\_
 *       min
 *
 *       max  ____________________
 *                  /\      /\
 * ADC1  1/2  _____/__\___ /__\___
 *             \  /    \  /    \
 *            __\/______\/______\_
 *       min
 * ```
 *
 * The code also supports dead-zone for each individual (ADC/wiper). The dead
 * zone is used fot two reasons. The one is the to minimize ADC noise and the
 * other is to increase or decrease the pot sensitivity. Dead-zone means that
 * the next sampled value must be larger or smaller than the current by the
 * dead-zone value. Therefore, if the current ADC value is 1000 and the dead
 * zone value is set to 30, then to actually run the calculation algorithm
 * the next sample but be >=130 or <=70. That also means that this 60 bit dead
 * zone will affect the pot sensitivity, which means that you need to do more
 * turns to get from the relative min to relative max value.
 *
 * The algorithm is based on relative values and supports both signed and
 * unsigned integers and also floats if the MCU is capable. Also the init code
 * can set the initial value. Also, each pot can have individual settings.
 * Therefore, one pot and only have integer values in the range of [0-111] with
 * initial value of 72. Another have range [-100,100] with initial value 0 and
 * another one [-120.25,70.50] with initial value of 2.25. Also, each different
 * pot supports an individual step, which also can be integer of float.
 *
 * Therefore, the pot API provides individual range, min, max, step and dead-zone
 * values and you can use any number of pots as long you initialize the proper
 * size in the init function.
 *
 *  Created on: Jul 5, 2018
 *      Author: Dimitris Tassopoulos
 */

#ifndef __ROTARY_CONT_POT_H_
#define __ROTARY_CONT_POT_H_

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include "LICENSE.h"

/* In MCUs that doesn't support hard float then,
 * you can disable floats for performance
 */
//#define RCP_SUPPORT_FLOATS

#ifdef RCP_SUPPORT_FLOATS
typedef float rcp_val_t;
#else
typedef int16_t rcp_val_t;
#endif

typedef void (*update_cbk_f)(rcp_val_t value);

/**
 * Rotary continuous pot data.
 * Each ADC needs one of this
 */
struct rcp_data {
	uint16_t	curr_adc_value;
	uint16_t	prev_adc_value;
};

/**
 * ADC settings.
 * These are system specific settings and they are individual
 * for each pot in case 2 ADCs with different specs are used.
 * @param[in] min_adc_val The minimum ADC value (usually 0)
 * @param[in] max_adc_value This the ADCs max value (usually ((1 << num_of_adc_bits) - 1)
 * @param[in] dead_zone In case there's noise on the ADC use this to create dead zones
 * 					Usually prefer that not to be 0, but >10. The larger the zone
 * 					The more turns you need with the pot.
 */
struct rcp_settings {
	uint16_t	min_adc_val;	// usually: 0
	uint16_t	max_adc_val;	// usually: (1 << adc_bits) - 1
	uint8_t		dead_zone;
};

/**
 * @brief Rotary continuous pot data
 * @param
 * @param min That's the the min value that the pot can take.
 * @param max That's the max value of the pot
 * @param step This value defines the ADC window for which the
 * 				relative value doesn't change. So, if from the
 * 				(curr_value-adc_step) to (curr_adc_value+adc_step)
 * 				the relative pot value is the same
 */
struct rcp_pot {
	rcp_val_t		value;
	rcp_val_t		min;
	rcp_val_t		max;
	rcp_val_t		step;
	uint8_t			prev_quarter;
	update_cbk_f	callback;
	struct rcp_settings settings[2];
	struct rcp_data  	data[2];
};

/**
 * static declaration for pot settings. Just a shortcut
 */
#define DECLARE_RCP_ADC(NAME,MIN_ADC_VAL,MAX_ADC_VAL,DEAD_ZONE) \
	struct rcp_settings NAME = { \
		.min_adc_val = MIN_ADC_VAL, \
		.max_adc_val = MAX_ADC_VAL, \
		.dead_zone = DEAD_ZONE, \
	}

#define DECLARE_RCP_POT(NAME,START,MIN,MAX,STEP,CBK) \
	struct rcp_pot NAME = { \
		.value = START, \
		.min = MIN, \
		.max = MAX, \
		.step = STEP, \
		.callback = CBK, \
	}

/**
 * @brief Add a new pot. Each pot has two gangs and needs two ADCs.
 * @param[in] struct rcp_pot A pointer to a rcp_pot struct that is created e.g. with DECLARE_RCP_POT
 * @return int Returns the index in the pot array
 */
int rcp_add(struct rcp_pot * pot, struct rcp_settings *adc1_settings, struct rcp_settings *adc2_settings);

int rcp_init(struct rcp_pot * pot, uint16_t adc1_val, uint16_t adc2_val, rcp_val_t start_value,
		rcp_val_t min, rcp_val_t max, rcp_val_t step,
		struct rcp_settings *adc1_settings, struct rcp_settings *adc2_settings,
		update_cbk_f callback);
	

/**
 * @brief Update the internal ADCs values and calculate the new pot value
 * @param[in] index The index of the pot
 * @param[in] adc1_val ADC value of the first gang pot
 * @param[in] adc2_val ADC value of the second gang pot
 */
int rcp_set_update_adc_values(struct rcp_pot * pot, uint16_t adc1_val, uint16_t adc2_val);

/**
 * @brief Get current pot value
 * @param[in] index The pot index
 * @return rcp_val_t The value of the pot
 */
static inline rcp_val_t __attribute__((always_inline))
rcp_get_value(struct rcp_pot * pot)
{
	return pot->value;
}



/**
 * @brief Set the pot value (forcefully)
 * @param[in] index The pot index
 * @param[in] value The new value
 */
static inline void __attribute__((always_inline))
rcp_set_value(struct rcp_pot * pot, rcp_val_t value)
{
	if ((value >= pot->min) && (value <= pot->max))
		pot->value = value;
}


#endif /* __ROTARY_CONT_POT_H_ */
