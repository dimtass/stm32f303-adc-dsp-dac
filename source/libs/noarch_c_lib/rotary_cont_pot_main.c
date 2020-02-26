/**
 * This is an example test project for the rotary_cont_pot.h (rotary continuous pot library)
 *
 * @author: Dimitris Tassopoulos <dimtass@gmail.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

#include "LICENSE.h"
#include "various_defs.h"
#include "trace.h"
#include "rotary_cont_pot.h"

/* Set trace levels */
uint32_t trace_levels = \
		TRACE_LEVEL_DEFAULT |
		0;

void pot1_cbk(rcp_val_t value)
{
	TRACE(("POT1: %d\n", value));
}

void pot2_cbk(rcp_val_t value)
{
	TRACE(("POT2: %d\n", value));
}

int main()
{
	DECLARE_RCP_ADC(adc1, 0, 1023, 10);
	DECLARE_RCP_ADC(adc2, 0, 1023, 10);

	TRACE(("POT1:\n"));
	struct rcp_pot pot1;
	rcp_init(&pot1, 0, 0, 0, 0, 100, 1, &adc1, &adc2, &pot1_cbk);
	rcp_set_update_adc_values(&pot1, 20, 50);
	rcp_set_update_adc_values(&pot1, 30, 60);
	rcp_set_update_adc_values(&pot1, 40, 80);

	TRACE(("\n\nPOT2:\n"));
	DECLARE_RCP_POT(pot2, 0, 0, 100, 5, pot2_cbk);
	rcp_add(&pot2, &adc1, &adc2);
	rcp_set_update_adc_values(&pot2, 20, 50);
	rcp_set_update_adc_values(&pot2, 30, 60);
	rcp_set_update_adc_values(&pot2, 40, 80);

	TRACE(("End\n"));
	return 0;
}
