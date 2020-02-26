/**
 * dev_spi_master.h
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
 * Created on: 14 May 2018
 * Author: Dimitris Tassopoulos <dimtass@gmail.com>
 * 
 * usage:
 * // Initialize spi
 * 
 */

#ifndef DEV_SPI_H_
#define DEV_SPI_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "dev_spi_common.h"

#define DECLARE_SPI_CHANNEL(NAME, OWNER, CHANNEL) \
	struct spi_device NAME = { \
		.channel = CHANNEL, \
	}


/* Init functions */
void* spi_init_master(enum en_spi_port port, struct spi_device * spi);
void spi_remove(struct spi_device * spi);

/* Control functions */
void spi_start(struct spi_device * spi);
void spi_stop(struct spi_device * spi);
void spi_wait(struct spi_device * spi);
void spi_set8(struct spi_device * spi);
void spi_set16(struct spi_device * spi);

/* 8-bit send/receive functions */
void spi_send8(struct spi_device * spi, uint8_t * data, size_t data_len);
void spi_sendCircular8(struct spi_device * spi, uint8_t * data, size_t data_len);
void spi_recv8(struct spi_device * spi, uint8_t * data, size_t data_len);
void spi_recvCircular8(struct spi_device * spi, uint8_t * data, size_t data_len);

/* 16-bit functions */
void spi_send16(struct spi_device * spi, uint16_t *data, size_t data_len);
void spi_sendCircular16(struct spi_device * spi, uint16_t *data, size_t data_len);
void spi_recv16(struct spi_device * spi, uint16_t * data, size_t data_len);
void spi_recvCircular16(struct spi_device * spi, uint8_t * data, size_t data_len);

void spi_get_available_freq(uint8_t * num_of_freq, uint32_t ** freq_array);

#ifdef __cplusplus
}
#endif

#endif /* DEV_SPI_H_ */
