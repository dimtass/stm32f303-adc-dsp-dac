#ifndef BTN_LIB_H_
#define BTN_LIB_H_

#include <stdint.h>
#include "list.h"

enum en_btn_status {
    BTN_STATUS_UP,
    BTN_STATUS_UP_PRESSED,
    BTN_STATUS_UP_LONG_PRESSED,
    BTN_STATUS_DOWN,
    BTN_STATUS_DOWN_PRESSED,
    BTN_STATUS_DOWN_LONG_PRESSED,
	BTN_STATUS_DOWN_VERY_LONG_PRESSED,
	
	BTN_STATUS_UNDEFINED,
};

enum en_btn_en_level {
    BTN_ENABLE_LVL_LOW = 0,
    BTN_ENABLE_LVL_HIGH,
};

#define DECLARE_MODULE_BTN_SCANNER(NAME,TICKS) \
struct mod_btn_scanner NAME = { \
        .ticks = TICKS, \
    }

#define DECLARE_DEV_BTN(PORT, PIN, EN_LEVEL, PRESS_THRES, LONG_PRESS_THRES, VERY_LONG_PRESS_THRES, PRESS_EVENT_CBK, OWNER) \
	{ \
		.owner = OWNER, \
        .port = PORT, \
		.pin = PIN, \
        .en_level = EN_LEVEL, \
		.press_thres = PRESS_THRES, \
		.long_press_thres = LONG_PRESS_THRES, \
		.very_long_press_thres = VERY_LONG_PRESS_THRES, \
        .press_event = PRESS_EVENT_CBK, \
        .cntr = 0, \
	}

struct mod_btn_scanner {
    uint16_t ticks;
    struct list_head btn_list;
};

struct dev_btn {
    struct mod_btn_scanner * owner;
    volatile uint8_t * port;
    uint8_t pin;
    uint16_t press_thres;
    uint16_t long_press_thres;
    uint8_t status;
    uint8_t curr_value;
    uint8_t prev_value;
    uint16_t cntr;
    uint8_t en_level;
    void (*press_event)(enum en_btn_status btn_status);
    struct list_head list;
};

extern void btn_scanner_init(struct mod_btn_scanner * mod);
extern void btn_scanner_add(struct mod_btn_scanner * mod, struct dev_btn * new_btn);
extern void btn_scanner_update(struct mod_btn_scanner * mod);
extern enum en_btn_status btn_init_status(struct dev_btn * btn_it);

#endif  // BTN_LIB
