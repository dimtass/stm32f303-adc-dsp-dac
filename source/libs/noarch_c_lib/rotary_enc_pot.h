/*
 * @file rotary_enc_pot.h
 *
 *
 *  Created on: Jul 5, 2018
 *      Author: Dimitris Tassopoulos
 */

#ifndef __ROTARY_ENC_POT_H_
#define __ROTARY_ENC_POT_H_

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include "LICENSE.h"

/* In MCUs that doesn't support hard float then,
 * you can disable floats for performance
 */
//#define REP_SUPPORT_FLOATS

#ifdef REP_SUPPORT_FLOATS
typedef float rep_val_t;
#else
typedef int16_t rep_val_t;
#endif

typedef void (*update_cbk_f)(rep_val_t value);

/**
 * @brief Rotary encoder pot data
 * @param
 * @param min That's the the min value that the pot can take.
 * @param max That's the max value of the pot
 * @param step This value defines the ADC window for which the
 * 				relative value doesn't change. So, if from the
 * 				(curr_value-adc_step) to (curr_adc_value+adc_step)
 * 				the relative pot value is the same
 */
struct rep_pot {
	rep_val_t		value;
	rep_val_t		min;
	rep_val_t		max;
	rep_val_t		step;
	uint8_t			invert;
	update_cbk_f	callback;
};

#define DECLARE_REP_POT(NAME,START,MIN,MAX,STEP,INV,CBK) \
	struct rep_pot NAME = { \
		.value = START, \
		.min = MIN, \
		.max = MAX, \
		.step = STEP, \
		.invert = INV, \
		.callback = CBK, \
	}

int rep_init(struct rep_pot * pot, rep_val_t start_value,
		rep_val_t min, rep_val_t max, rep_val_t step, uint8_t invert,
		update_cbk_f callback);

/**
 * @brief Update the internal ADCs values and calculate the new pot value
 * @param[in] index The index of the pot
 * @param[in] adc1_val ADC value of the first gang pot
 * @param[in] adc2_val ADC value of the second gang pot
 */
int rep_set_update_values(struct rep_pot * pot, uint8_t p1, uint8_t p2);

/**
 * @brief Get current pot value
 * @param[in] index The pot index
 * @return rep_val_t The value of the pot
 */
static inline rep_val_t __attribute__((always_inline))
rep_get_value(struct rep_pot * pot)
{
	return pot->value;
}


/**
 * @brief Set the pot value (forcefully)
 * @param[in] index The pot index
 * @param[in] value The new value
 */
static inline void __attribute__((always_inline))
rep_set_value(struct rep_pot * pot, rep_val_t value)
{
	if ((value >= pot->min) && (value <= pot->max))
		pot->value = value;
}


#endif /* __ROTARY_ENC_POT_H_ */
