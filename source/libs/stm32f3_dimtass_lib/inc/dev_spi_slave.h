/**
 * dev_spi.h
 *
 * Copyright 2018 Dimitris Tassopoulos <dimtass@gmail.com>
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
 * usage:
 * // Create a callback
 * void spi_callback(struct spi_device * dev);
 * // Create a buffers
 * DECLARE_SPI_BUFFER(spi_buffer,uint16_t,1);
 * // Create an spi spi_device
 * struct spi_device spi_slave;
 * 
 * // Then in main()
 * spi_set_options(&spi_slave, 16, SPI_MODE_0);
 * spi_init_slave(DEV_SPI2, &spi_buffer, &spi_callback, &spi_slave);
 * 
 * // Example of a callback function:
 * void spi_callback(struct spi_device * dev)
 * {
 *	   struct spi_buffers * buffers = (struct spi_buffers *) dev->data;
 *	   // uint16_t * rx_buffer = (uint16_t*) buffers->rx_buffer;
 *	   uint16_t * tx_buffer = (uint16_t*) buffers->tx_buffer;
 *     return;
 * }
 */


#ifndef DEV_SPI_SLAVE_H_
#define DEV_SPI_SLAVE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "dev_spi_common.h"

#define DECLARE_SPI_SLAVE_CHANNEL(NAME, OWNER, CHANNEL) \
	struct spi_slave_device NAME = { \
		.channel = CHANNEL, \
	}

#define DECLARE_SPI_BUFFER(NAME,TYPE,BUFFER_SIZE) \
    TYPE NAME##_rx_buffer[BUFFER_SIZE]; \
    TYPE NAME##_tx_buffer[BUFFER_SIZE]; \
    struct spi_buffers NAME = { \
        .rx_buffer = NAME##_rx_buffer, \
        .rx_buffer_len = BUFFER_SIZE, \
        .tx_buffer = NAME##_tx_buffer, \
        .tx_buffer_len = BUFFER_SIZE, \
    }



void spi_init_slave(enum en_spi_port port,
                    struct spi_buffers * buffers,
                    receive_irq_t callback,
                    struct spi_device * spi);

#ifdef __cplusplus
}
#endif

#endif //DEV_SPI_SLAVE_H_