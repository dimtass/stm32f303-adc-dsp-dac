/**
 * dev_spi_common.h
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
 */
 
#ifndef DEV_SPI_COMMON_H_
#define DEV_SPI_COMMON_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f30x.h"
#include "debug_trace.h"

enum en_spi_port {
	DEV_SPI1_GPIOA,
	DEV_SPI1_GPIOB,
	DEV_SPI2
};

struct spi_device;

struct spi_buffers {
    volatile void   *rx_buffer;
    uint16_t        rx_buffer_len;
    volatile void   *tx_buffer;
    uint16_t        tx_buffer_len;
};

typedef void (*receive_irq_t)(struct spi_device * dev);

struct dma_channel {
	DMA_Channel_TypeDef *tx_ch;
	uint32_t		    tx_iqrn;
	DMA_Channel_TypeDef *rx_ch;
	uint32_t		    rx_iqrn;
	DMA_InitTypeDef     config;
};

struct spi_controller {
	SPI_TypeDef * 	spi;
	GPIO_TypeDef *  port;
	uint16_t		miso;
	uint16_t		mosi;
	uint16_t		sck;
	uint16_t		nss;
	SPI_InitTypeDef config;
	struct dma_channel * dma;
};

struct spi_device {
	struct spi_controller * slave;
	struct spi_controller * master;
	struct spi_controller * controller;
	uint16_t	        chip_select;
	uint8_t 	        bits_per_word; // Use only 8 or 16 (default: 8)
	uint16_t 	        speed; // Actually this is the prescaller (2, 4, 8, 16, 32, 64, 128, 256) (NOT used in slave mode)
	uint8_t		        mode; // See below:
#define SPI_MODE_CPHA        0x01
#define SPI_MODE_CPOL        0x02
#define SPI_MODE_0      (0|0)
#define SPI_MODE_1      (0|SPI_MODE_CPHA)
#define SPI_MODE_2      (SPI_MODE_CPOL|0)
#define SPI_MODE_3      (SPI_MODE_CPOL|SPI_MODE_CPHA)
	receive_irq_t       rcv_callback;
    void                *data;
};

static inline void spi_device_defaults(struct spi_device * dev) {
    dev->bits_per_word = 8;
    dev->mode = SPI_MODE_0;
    dev->speed = SPI_BaudRatePrescaler_2;
    dev->chip_select = 0;
    dev->data = dev->master = dev->slave = dev->controller = 0;
}

static inline void spi_set_options(struct spi_device * dev, 
            uint8_t bits_per_word, uint8_t mode)
{
    dev->bits_per_word = bits_per_word;
    dev->mode = mode;
}

#ifdef __cplusplus
}
#endif

#endif //DEV_SPI_COMMON_H_