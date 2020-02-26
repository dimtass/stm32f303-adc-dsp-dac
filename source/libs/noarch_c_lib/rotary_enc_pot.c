/*
 * rotary_enc_pot.c
 *
 *  Created on: Jul 5, 2018
 *      Author: Dimitris Tassopoulos
 */

#include <string.h>
#include "rotary_enc_pot.h"

/**
 *
 */
int rep_init(struct rep_pot * pot, rep_val_t start_value,
		rep_val_t min, rep_val_t max, rep_val_t step, uint8_t invert,
		update_cbk_f callback)
{
	if (!pot)
		return -1;

	pot->value = start_value;
	pot->min = min;
	pot->max = max;
	pot->step = step;
	pot->invert = invert;
	pot->callback = callback;

	return 0;
}

static inline void __attribute__((always_inline))
rep_increment_value(struct rep_pot * pot)
{
	if (pot->value < pot->max) {
		rep_val_t tmp = pot->value;
		tmp += pot->step;
		if (tmp > pot->max)
			tmp = pot->max;
		pot->value = tmp;
	}
}

static inline void __attribute__((always_inline))
rep_decrement_value(struct rep_pot * pot)
{
	if (pot->value > pot->min) {
		rep_val_t tmp = pot->value;
		tmp -= pot->step;
		if (tmp < pot->min)
			tmp = pot->min;
		pot->value = tmp;
	}
}


int rep_set_update_values(struct rep_pot * pot, uint8_t p1, uint8_t p2)
{
	if (!pot) return -1;

    if (p1 != p2) {
        if (!pot->invert) rep_increment_value(pot);
		else rep_decrement_value(pot);
    }
    else {
        if (!pot->invert) rep_decrement_value(pot);
		else rep_increment_value(pot);
    }

	/* if callback exists then call it */
	if (pot->callback)
		pot->callback(pot->value);

	return 0;
}

