#include <stddef.h>
#include "btn_lib.h"
#include "list.h"
#include "device_io_ports.h"
#include "io_ports.h"

#define BTN_EXISTS(BTN, ITTERATOR) ( (BTN->port == ITTERATOR->port) && (BTN->pin == ITTERATOR->pin) )

DECLARE_MODULE_BTN_SCANNER(btn_scanner,T_BTN_SCAN);

/* Power on/off button */
struct dev_btn onoff = DECLARE_DEV_BTN(&PIND, PIND2, BTN_ENABLE_LVL_HIGH, 100, 1200, 6000, cbk_btn_onoff, &btn_scanner);

/* Power supply detect - treat as button in order to use debouncing features */
struct dev_btn psu = DECLARE_DEV_BTN(&PIND, PIND3, BTN_ENABLE_LVL_LOW, 200, 200, 200, cbk_btn_psu, &btn_scanner);

void btn_scanner_init(struct mod_btn_scanner * mod)
{
    INIT_LIST_HEAD(&mod->btn_list);
}

static inline struct dev_btn * btn_find(struct mod_btn_scanner * mod, struct dev_btn * btn)
{
	if (!mod || !btn) return NULL;
	if (!list_empty(&mod->btn_list)) {
		struct dev_btn * btn_it = NULL;
		list_for_each_entry(btn_it, &mod->btn_list, list) {
			if BTN_EXISTS(btn, btn_it) {
				/* found */
				return(btn_it);
			}
		}
	}
	return NULL;
}


void btn_scanner_add(struct mod_btn_scanner * mod, struct dev_btn * new_btn)
{
	/* init dev head list */
	INIT_LIST_HEAD(&new_btn->list);
    /* Calculate the proper timeouts */
    new_btn->press_thres /=  mod->ticks;
    new_btn->long_press_thres /= mod->ticks;
    new_btn->very_long_press_thres /= mod->ticks;
	/* Add to led_list */
	list_add(&new_btn->list, &mod->btn_list);
}

void btn_scanner_update(struct mod_btn_scanner * mod)
{
	if (!list_empty(&mod->btn_list)) {
		struct dev_btn * btn_it;
		list_for_each_entry(btn_it, &mod->btn_list, list) {
            /* read port pin */
            uint8_t value = *(btn_it->port) & (1 << btn_it->pin);
			uint8_t st_event = BTN_STATUS_UNDEFINED;
            /* check value */
            if (!!value == btn_it->en_level) {
				if (btn_it->cntr == btn_it->very_long_press_thres)
					st_event = BTN_STATUS_DOWN_VERY_LONG_PRESSED;
				else if (btn_it->cntr == btn_it->long_press_thres)
					st_event = BTN_STATUS_DOWN_LONG_PRESSED;
				else if (btn_it->cntr == btn_it->press_thres)
					st_event = BTN_STATUS_DOWN_PRESSED;
				else if (btn_it->cntr == 0)
					st_event = BTN_STATUS_DOWN;
				btn_it->cntr++;
			}
            else {
                if (btn_it->cntr >= btn_it->very_long_press_thres)
                    st_event = BTN_STATUS_UP_VERY_LONG_PRESSED;
                else if (btn_it->cntr >= btn_it->long_press_thres)
                    st_event = BTN_STATUS_UP_LONG_PRESSED;
                else if (btn_it->cntr >= btn_it->press_thres)
                    st_event = BTN_STATUS_UP_PRESSED;
                else if (btn_it->cntr > 0)
                    st_event = BTN_STATUS_UP;
                btn_it->cntr = 0;
            }
			if (st_event != BTN_STATUS_UNDEFINED) btn_it->press_event(st_event);
		}
	}
}

enum en_btn_status btn_init_status(struct dev_btn * btn_it)
{
	enum en_btn_status ret;
	uint8_t value = *(btn_it->port) & (1 << btn_it->pin);
	btn_it->cntr = 0;
	/* check value */
	if (!!value == btn_it->en_level)
			ret = BTN_STATUS_DOWN;
	else
			ret = BTN_STATUS_UP;
	return ret;
};