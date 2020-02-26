/*
 * rotary_cont_pot.c
 *
 *  Created on: Jul 5, 2018
 *      Author: Dimitris Tassopoulos
 */

#include <string.h>
#include "rotary_cont_pot.h"

#define ADC_HALF(POT,ADC_INDEX) ((POT->settings[ADC_INDEX].max_adc_val - POT->settings[ADC_INDEX].min_adc_val) >> 1)
#define IS_INCR(VAL1,VAL2) ((VAL1-VAL2) > 0 ? 1 : 0)
#define IS_DEADZONE(POT,ADC_INDEX,VAL) ( \
			(VAL > (POT->data[ADC_INDEX].curr_adc_value - POT->settings[ADC_INDEX].dead_zone)) \
			&& (VAL < (POT->data[ADC_INDEX].curr_adc_value + POT->settings[ADC_INDEX].dead_zone)) \
					)

enum en_rcp_quarters {
	RCP_Q1,
	RCP_Q2,
	RCP_Q3,
	RCP_Q4
};

enum en_adc_num {
	RCP_ADC1 = 0,
	RCP_ADC2
};


int rcp_add(struct rcp_pot * pot, struct rcp_settings *adc1_settings, struct rcp_settings *adc2_settings)
{
	if (!pot)
		return -1;
	
	memcpy(&pot->settings[RCP_ADC1], adc1_settings, sizeof(struct rcp_settings));
	memcpy(&pot->settings[RCP_ADC2], adc2_settings, sizeof(struct rcp_settings));

	pot->data[RCP_ADC1].curr_adc_value = 0;
	pot->data[RCP_ADC1].prev_adc_value = 0;

	pot->data[RCP_ADC2].curr_adc_value = 0;
	pot->data[RCP_ADC2].prev_adc_value = 0;

	return 0;
}

/**
 *
 */
int rcp_init(struct rcp_pot * pot, uint16_t adc1_val, uint16_t adc2_val, rcp_val_t start_value,
		rcp_val_t min, rcp_val_t max, rcp_val_t step,
		struct rcp_settings *adc1_settings, struct rcp_settings *adc2_settings,
		update_cbk_f callback)
{
	if (!pot)
		return -1;

	pot->value = start_value;
	pot->min = min;
	pot->max = max;
	pot->step = step;
	pot->callback = callback;
	memcpy(&pot->settings[RCP_ADC1], adc1_settings, sizeof(struct rcp_settings));
	memcpy(&pot->settings[RCP_ADC2], adc2_settings, sizeof(struct rcp_settings));

	pot->data[RCP_ADC1].curr_adc_value = adc1_val;
	pot->data[RCP_ADC1].prev_adc_value = adc1_val;

	pot->data[RCP_ADC2].curr_adc_value = adc2_val;
	pot->data[RCP_ADC2].prev_adc_value = adc2_val;

	return 0;
}


static inline uint8_t rcp_get_quarter(struct rcp_pot * pot, uint16_t adc1_val, uint16_t adc2_val)
{
	uint8_t quarter = RCP_Q1;

	if ( (adc1_val <= ADC_HALF(pot,RCP_ADC1)) &&
			(adc2_val >= ADC_HALF(pot,RCP_ADC2)) ) {
		quarter = RCP_Q1;
	}
	else if ( (adc1_val >= ADC_HALF(pot,RCP_ADC1)) &&
			(adc2_val >= ADC_HALF(pot,RCP_ADC2)) ) {
		quarter = RCP_Q2;
	}
	else if ( (adc1_val >= ADC_HALF(pot,RCP_ADC1)) &&
			(adc2_val <= ADC_HALF(pot,RCP_ADC2)) ) {
		quarter = RCP_Q3;
	}
	else if ( (adc1_val <= ADC_HALF(pot,RCP_ADC1)) &&
			(adc2_val <= ADC_HALF(pot,RCP_ADC2)) ) {
		quarter = RCP_Q4;
	}

	return quarter;
}

static inline void rcp_increment_value(struct rcp_pot * pot)
{
	if (pot->value < pot->max) {
		rcp_val_t tmp = pot->value;
		tmp += pot->step;
		if (tmp > pot->max)
			tmp = pot->max;
		pot->value = tmp;
	}
//	TRACE(("[+]: %.2f\n", (float) pot->value));
}

static inline void rcp_decrement_value(struct rcp_pot * pot)
{
	if (pot->value > pot->min) {
		rcp_val_t tmp = pot->value;
		tmp -= pot->step;
		if (tmp < pot->min)
			tmp = pot->min;
		pot->value = tmp;
	}
//	TRACE(("[-]: %.2f\n", (float) pot->value));
}


int rcp_set_update_adc_values(struct rcp_pot * pot, uint16_t adc1_val, uint16_t adc2_val)
{
	if (!pot) return -1;

	uint8_t quarter = rcp_get_quarter(pot, adc1_val, adc2_val);

	uint16_t prev1 = pot->data[RCP_ADC1].curr_adc_value;
	uint16_t curr1 = adc1_val;
	uint16_t prev2 = pot->data[RCP_ADC2].curr_adc_value;
	uint16_t curr2 = adc2_val;

	if (quarter == RCP_Q1) {
		if (IS_DEADZONE(pot,RCP_ADC2,adc2_val))
			return -2;
		if ( IS_INCR(curr2,prev2)
				|| (pot->prev_quarter == RCP_Q4))
			rcp_increment_value(pot);
		else
			rcp_decrement_value(pot);
	}
	else if (quarter == RCP_Q2) {
		if (IS_DEADZONE(pot,RCP_ADC1,adc1_val))
			return -2;
		if ( IS_INCR(curr1,prev1)
				|| (pot->prev_quarter == RCP_Q1))
			rcp_increment_value(pot);
		else
			rcp_decrement_value(pot);
	}
	else if (quarter == RCP_Q3) {
		if (IS_DEADZONE(pot,RCP_ADC1,adc1_val))
			return -2;
		if ( !IS_INCR(curr1,prev1)
			|| (pot->prev_quarter == RCP_Q2))
			rcp_increment_value(pot);
		else
			rcp_decrement_value(pot);
	}
	else if (quarter == RCP_Q4) {
		if (IS_DEADZONE(pot,RCP_ADC2,adc2_val))
			return -2;
		if ( IS_INCR(curr2,prev2)
				|| (pot->prev_quarter == RCP_Q3))
			rcp_increment_value(pot);
		else
			rcp_decrement_value(pot);
	}
//	TRACE(("%d: [1]:%d,[2]:%d\n", quarter, curr1-prev1, curr2-prev2));

	/* update prev/curr values */
	pot->prev_quarter = quarter;
	pot->data[RCP_ADC1].prev_adc_value = prev1;
	pot->data[RCP_ADC1].curr_adc_value = curr1;
	pot->data[RCP_ADC2].prev_adc_value = prev2;
	pot->data[RCP_ADC2].curr_adc_value = curr2;

	/* if callback exists then call it */
	if (pot->callback)
		pot->callback(pot->value);

	return 0;
}
