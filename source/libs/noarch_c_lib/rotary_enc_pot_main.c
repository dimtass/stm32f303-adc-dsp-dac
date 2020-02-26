/**
 * This is an example test project for the rotary_enc_pot.h (rotary encoder library)
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
#include "rotary_enc_pot.h"

/* Set trace levels */
uint32_t trace_levels = \
		TRACE_LEVEL_DEFAULT |
		0;

void pot1_cbk(rep_val_t value)
{
	TRACE(("POT1: %d\n", value));
}

void pot2_cbk(rep_val_t value)
{
	TRACE(("POT2: %d\n", value));
}

int main()
{
	TRACE(("POT1:\n"));
	struct rep_pot pot1;
	rep_init(&pot1, 0, 0, 100, 1, 0, &pot1_cbk);
	rep_set_update_values(&pot1, 0, 1);
	rep_set_update_values(&pot1, 1, 0);
	rep_set_update_values(&pot1, 0, 1);

	TRACE(("\n\nPOT2:\n"));
	DECLARE_REP_POT(pot2, 0, -100, 100, 5, 1, pot2_cbk);
	rep_set_update_values(&pot2, 0, 0);
	rep_set_update_values(&pot2, 1, 1);
	rep_set_update_values(&pot2, 0, 0);

	TRACE(("End\n"));
	return 0;
}
