// This is code adapted from:
// https://github.com/nekromant/antares
// License GPLv2, according to
// https://github.com/nekromant/antares/blob/master/README
// Modified version stolen from https://github.com/onnokort/stm32f0-cc3000-echotest

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "stlinky.h"

#define min_t(type, a, b) ((a<b) ? a : b)

#define MY_STERM (*(volatile struct stlinky*)0x20000000)

volatile uint16_t tx_index = 0;

//static volatile struct stlinky sterm;
    
void stlinky_init()
{
    volatile struct stlinky *st=&MY_STERM;
	st->magic = STLINKY_MAGIC;
	st->bufsize = CONFIG_LIB_STLINKY_BSIZE;
}

int stlinky_tx(volatile struct stlinky* st, char* buf, int siz)
{
	int sz = min_t(int, CONFIG_LIB_STLINKY_BSIZE, siz);
	while(st->txsize != 0);;; 
	memcpy((char*) st->txbuf, buf, sz);
	st->txsize = (unsigned char) sz;
	return sz;
}

/* TODO: We loose data here if we read less than avaliable */ 
int stlinky_rx(volatile struct stlinky* st, char* buf, int siz)
{
	int ret;
	while(st->rxsize == 0);;; 
	int sz = min_t(int, (int) st->rxsize, siz);
	memcpy(buf, (char*) st->rxbuf, sz);
	ret = (int) (st->rxsize);
	st->rxsize = 0;
	return ret;
}

int stlinky_default_getchar() {
    static int read=CONFIG_LIB_STLINKY_BSIZE;

    if (!MY_STERM.rxsize)
        return -1;

    int res=MY_STERM.rxbuf[read++];
    
    if (read==MY_STERM.rxsize) {
        MY_STERM.rxsize=0;
        read=0;
    }
    return res;
}

int stlinky_rx_ready(void) {
    return MY_STERM.rxsize;
}

void stlinky_wait_for_terminal(volatile struct stlinky* st)
{
	st->txbuf[0]='\n';
	st->txsize=1;
	while(st->txsize != 0);;;
}

#ifdef USE_STTERM
int _write(int file, char *ptr, int len) {
    file=file; // fix unused warning
    return stlinky_tx(&MY_STERM, ptr, len);
}

int _read(int file, char *ptr, int len) {
    file=file; // fix unused warning
	return stlinky_rx(&MY_STERM, ptr, len);
}
#endif