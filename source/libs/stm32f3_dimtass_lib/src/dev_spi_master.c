/*
 * dev_spi.c
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
 */

#include "dev_spi_master.h"

#define NUM_OF_SPI_FREQ 8
uint32_t spi_available_freq[NUM_OF_SPI_FREQ];

static struct spi_controller m_devices[] = {
	[DEV_SPI1_GPIOA] = {SPI1, GPIOA, GPIO_Pin_6, GPIO_Pin_7, GPIO_Pin_5, GPIO_Pin_4},
	[DEV_SPI1_GPIOB] = {SPI1, GPIOA, GPIO_Pin_6, GPIO_Pin_7, GPIO_Pin_5, GPIO_Pin_4},
	[DEV_SPI2] = {SPI2, GPIOB, GPIO_Pin_14, GPIO_Pin_15, GPIO_Pin_13, GPIO_Pin_12},
};

static struct dma_channel m_dma_channels[] = {
	[DEV_SPI1_GPIOA] = {DMA1_Channel3, DMA1_Channel3_IRQn, DMA1_Channel2, DMA1_Channel2_IRQn, {0}},
	[DEV_SPI1_GPIOB] = {DMA1_Channel3, DMA1_Channel3_IRQn, DMA1_Channel2, DMA1_Channel2_IRQn, {0}},
	[DEV_SPI2] = {DMA1_Channel5, DMA1_Channel5_IRQn, DMA1_Channel4, DMA1_Channel4_IRQn, {0}},
};

void* spi_init_master(enum en_spi_port port, struct spi_device * spi)
{
	if (port == DEV_SPI1_GPIOA) {
		spi->controller = &m_devices[DEV_SPI1_GPIOA];
		spi->controller->dma = &m_dma_channels[DEV_SPI1_GPIOA];
		/* RCC configuration */
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_SPI1, ENABLE);
	}
	else if (port == DEV_SPI1_GPIOB) {
		spi->controller = &m_devices[DEV_SPI1_GPIOB];
		spi->controller->dma = &m_dma_channels[DEV_SPI1_GPIOB];
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_SPI1 | RCC_APB2Periph_AFIO, ENABLE);
	}
	else if (port == DEV_SPI2) {
		spi->controller = &m_devices[DEV_SPI2];
		spi->controller->dma = &m_dma_channels[DEV_SPI2];
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);
	}

	SPI_TypeDef * dev = spi->controller->spi;
	SPI_InitTypeDef * spi_conf = &spi->controller->config;
	DMA_InitTypeDef * dma_conf = &spi->controller->dma->config;
	struct dma_channel * dma = spi->controller->dma;

	/* in case that another CS pin is used */
	if (spi->chip_select)
		spi->controller->nss = spi->chip_select;
                                                     
	/* in case of SPI master, controller = master */
	spi->master = spi->controller;
    spi->slave = NULL;
 
	/* GPIO configuration */
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = spi->controller->sck | spi->controller->mosi;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(spi->controller->port, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = spi->controller->nss;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(spi->controller->port, &GPIO_InitStructure);
	spi->controller->port->ODR |= spi->controller->nss;

	GPIO_InitStructure.GPIO_Pin = spi->controller->miso;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
    GPIO_Init(spi->controller->port, &GPIO_InitStructure);

    /* SPI configuration */

    SPI_StructInit(spi_conf);
    spi_conf->SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    spi_conf->SPI_Mode = SPI_Mode_Master;
	if (spi->bits_per_word == 16)
    	spi_conf->SPI_DataSize = SPI_DataSize_16b;
	else
    	spi_conf->SPI_DataSize = SPI_DataSize_8b;
    spi_conf->SPI_NSS = SPI_NSS_Soft;
    spi_conf->SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2; //((SystemCoreClock / spi->speed) - 1) << 3;
    spi_conf->SPI_FirstBit = SPI_FirstBit_MSB;
    switch (spi->mode) {
    case SPI_MODE_0:
        spi_conf->SPI_CPOL = SPI_CPOL_Low;
        spi_conf->SPI_CPHA = SPI_CPHA_1Edge;
    	break;
    case SPI_MODE_1:
        spi_conf->SPI_CPOL = SPI_CPOL_Low;
        spi_conf->SPI_CPHA = SPI_CPHA_2Edge;
    	break;
    case SPI_MODE_2:
        spi_conf->SPI_CPOL = SPI_CPOL_High;
        spi_conf->SPI_CPHA = SPI_CPHA_1Edge;
    	break;
    case SPI_MODE_3:
        spi_conf->SPI_CPOL = SPI_CPOL_High;
        spi_conf->SPI_CPHA = SPI_CPHA_2Edge;
    	break;
    default:
        spi_conf->SPI_CPOL = SPI_CPOL_Low;
        spi_conf->SPI_CPHA = SPI_CPHA_2Edge;
    };
	SPI_CalculateCRC(dev, DISABLE);
    SPI_Init(dev, &spi->controller->config);
    SPI_Cmd(dev, ENABLE);

    /* DMA configuration */
	/* Enable DMA clock */
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
	/* Initialize DMA struct */
	DMA_StructInit(dma_conf);

	/* SPI_MASTER_Rx_DMA_Channel configuration */
	DMA_DeInit(dma->rx_ch);
	/* SPI_MASTER_Tx_DMA_Channel configuration */
	DMA_DeInit(dma->tx_ch);

	/* Init default DMA struct */
	dma_conf->DMA_PeripheralBaseAddr = (uint32_t) &dev->DR;
	dma_conf->DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	dma_conf->DMA_MemoryInc = DMA_MemoryInc_Enable;
	dma_conf->DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	dma_conf->DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	dma_conf->DMA_Mode = DMA_Mode_Normal;
	dma_conf->DMA_Priority = DMA_Priority_High;
	dma_conf->DMA_M2M = DMA_M2M_Disable;

    NVIC_EnableIRQ(dma->rx_iqrn);
    DMA_ITConfig(dma->rx_ch, DMA_IT_TC, ENABLE);
    NVIC_EnableIRQ(dma->tx_iqrn);
    DMA_ITConfig(dma->tx_ch, DMA_IT_TC, ENABLE);

	/* Enable Tx DMA DMA Tx request */
	SPI_I2S_DMACmd(dev, SPI_I2S_DMAReq_Rx | SPI_I2S_DMAReq_Tx, ENABLE);

	return dev;
}


inline void spi_start(struct spi_device * spi)
{
	spi->controller->port->ODR &= ~spi->controller->nss;
}

inline void spi_stop(struct spi_device * spi)
{
	spi->controller->port->ODR |= spi->controller->nss;
}

static inline void spi_tx(struct spi_device * spi)
{
	struct dma_channel * dma  = spi->controller->dma;

	DMA_Init(dma->tx_ch, &dma->config);
	/* Enable DMA channels */
	DMA_Cmd(dma->tx_ch, ENABLE);
}

static inline void spi_rx(struct spi_device * spi)
{
	struct dma_channel * dma  = spi->controller->dma;

	DMA_Init(dma->rx_ch, &dma->config);
	/* Enable DMA channels */
	DMA_Cmd(dma->rx_ch, ENABLE);
}

inline void spi_wait(struct spi_device * spi)
{
	/* Wait DMA to finish */
	while(SPI_I2S_GetFlagStatus(spi->controller->spi, SPI_I2S_FLAG_BSY) == SET);
//    /* Disable DMA */
//	DMA_DeInit(spi->dma_tx_ch);
//	DMA_DeInit(spi->dma_rx_ch);
}

inline void spi_send8(struct spi_device * spi, uint8_t * data, size_t data_len)
{
	DMA_InitTypeDef * dma_conf = &spi->controller->dma->config;

	/* Configure Tx DMA */
	dma_conf->DMA_MemoryBaseAddr = (uint32_t)(data);
	dma_conf->DMA_BufferSize = data_len;

    dma_conf->DMA_Mode               = DMA_Mode_Normal;
    dma_conf->DMA_MemoryInc          = DMA_MemoryInc_Enable;
    dma_conf->DMA_DIR                = DMA_DIR_PeripheralDST;
    dma_conf->DMA_MemoryDataSize     = DMA_MemoryDataSize_Byte;
    dma_conf->DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;

	spi_tx(spi);
}

inline void spi_sendCircular8(struct spi_device * spi, uint8_t * data, size_t data_len)
{
	DMA_InitTypeDef * dma_conf = &spi->controller->dma->config;

	/* Configure Tx DMA */
	dma_conf->DMA_MemoryBaseAddr = (uint32_t)(data);
	dma_conf->DMA_BufferSize = data_len;

    dma_conf->DMA_Mode               = DMA_Mode_Circular;
    dma_conf->DMA_MemoryInc          = DMA_MemoryInc_Enable;
    dma_conf->DMA_DIR                = DMA_DIR_PeripheralDST;
    dma_conf->DMA_MemoryDataSize     = DMA_MemoryDataSize_Byte;
    dma_conf->DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;

	spi_tx(spi);
}


inline void spi_recv8(struct spi_device * spi, uint8_t * data, size_t data_len)
{
	DMA_InitTypeDef * dma_conf = &spi->controller->dma->config;

	/* Configure Rx DMA */
	dma_conf->DMA_MemoryBaseAddr = (uint32_t)(data);
	dma_conf->DMA_BufferSize = data_len;

    dma_conf->DMA_Mode               = DMA_Mode_Normal;
    dma_conf->DMA_MemoryInc          = DMA_MemoryInc_Enable;
    dma_conf->DMA_DIR                = DMA_DIR_PeripheralSRC;
    dma_conf->DMA_MemoryDataSize     = DMA_MemoryDataSize_Byte;
    dma_conf->DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;

	spi_rx(spi);
}

inline void spi_recvCircular8(struct spi_device * spi, uint8_t * data, size_t data_len)
{
	DMA_InitTypeDef * dma_conf = &spi->controller->dma->config;

	/* Configure Tx DMA */
	dma_conf->DMA_MemoryBaseAddr = (uint32_t)(data);
	dma_conf->DMA_BufferSize = data_len;

    dma_conf->DMA_Mode               = DMA_Mode_Circular;
    dma_conf->DMA_MemoryInc          = DMA_MemoryInc_Enable;
    dma_conf->DMA_DIR                = DMA_DIR_PeripheralSRC;
    dma_conf->DMA_MemoryDataSize     = DMA_MemoryDataSize_Byte;
    dma_conf->DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;

	spi_rx(spi);
}

inline void spi_sendCircular16(struct spi_device * spi, uint16_t *data, size_t data_len)
{
	DMA_InitTypeDef * dma_conf = &spi->controller->dma->config;

    dma_conf->DMA_MemoryBaseAddr = (u32) data;
    dma_conf->DMA_BufferSize     = data_len;

    dma_conf->DMA_Mode               = DMA_Mode_Circular;
    dma_conf->DMA_MemoryInc          = DMA_MemoryInc_Disable;
    dma_conf->DMA_DIR                = DMA_DIR_PeripheralDST;
    dma_conf->DMA_MemoryDataSize     = DMA_MemoryDataSize_HalfWord;
    dma_conf->DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;

    spi_tx(spi);
}

inline void spi_send16(struct spi_device * spi, uint16_t *data, size_t data_len)
{
	DMA_InitTypeDef * dma_conf = &spi->controller->dma->config;

    dma_conf->DMA_MemoryBaseAddr = (u32) data;
    dma_conf->DMA_BufferSize     = data_len;

    dma_conf->DMA_Mode               = DMA_Mode_Normal;
    dma_conf->DMA_MemoryInc          = DMA_MemoryInc_Enable;
    dma_conf->DMA_DIR                = DMA_DIR_PeripheralDST;
    dma_conf->DMA_MemoryDataSize     = DMA_MemoryDataSize_HalfWord;
    dma_conf->DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;

    spi_tx(spi);
}

inline void spi_recv16(struct spi_device * spi, uint16_t * data, size_t data_len)
{
	DMA_InitTypeDef * dma_conf = &spi->controller->dma->config;

	/* Configure Rx DMA */
	dma_conf->DMA_MemoryBaseAddr = (uint32_t)(data);
	dma_conf->DMA_BufferSize = data_len;

    dma_conf->DMA_Mode               = DMA_Mode_Normal;
    dma_conf->DMA_MemoryInc          = DMA_MemoryInc_Enable;
    dma_conf->DMA_DIR                = DMA_DIR_PeripheralSRC;
    dma_conf->DMA_MemoryDataSize     = DMA_MemoryDataSize_HalfWord;
    dma_conf->DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;

	spi_rx(spi);
}

inline void spi_recvCircular16(struct spi_device * spi, uint8_t * data, size_t data_len)
{
	DMA_InitTypeDef * dma_conf = &spi->controller->dma->config;

	/* Configure Rx DMA */
	dma_conf->DMA_MemoryBaseAddr = (uint32_t)(data);
	dma_conf->DMA_BufferSize = data_len;

    dma_conf->DMA_Mode               = DMA_Mode_Circular;
    dma_conf->DMA_MemoryInc          = DMA_MemoryInc_Enable;
    dma_conf->DMA_DIR                = DMA_DIR_PeripheralSRC;
    dma_conf->DMA_MemoryDataSize     = DMA_MemoryDataSize_HalfWord;
    dma_conf->DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;

	spi_rx(spi);
}

void spi_set8(struct spi_device * spi)
{
	SPI_TypeDef * dev = spi->controller->spi;

    dev->CR1 &= ~SPI_CR1_SPE; // DISABLE SPI
    dev->CR1 &= ~SPI_CR1_DFF; // SPI 8
    dev->CR1 |= SPI_CR1_SPE;  // ENABLE SPI
}

void spi_set16(struct spi_device * spi)
{
	SPI_TypeDef * dev = spi->controller->spi;

    dev->CR1 &= ~SPI_CR1_SPE; // DISABLE SPI
    dev->CR1 |= SPI_CR1_DFF;  // SPI 16
    dev->CR1 |= SPI_CR1_SPE;  // ENABLE SPI
}

void spi_calculate_available_freq(uint32_t * freq_array)
{
	for (int i=0; i<NUM_OF_SPI_FREQ; i++) {
		freq_array[i] = SystemCoreClock / (1 << (i+1));
	}
}

void spi_get_available_freq(uint8_t * num_of_freq, uint32_t ** freq_array)
{
	*num_of_freq = NUM_OF_SPI_FREQ;
	*freq_array = spi_available_freq;
}


void DMA1_Channel2_IRQHandler(void) {
    if (DMA_GetITStatus(DMA1_IT_TC2) == SET) {
        DMA_Cmd(DMA1_Channel2, DISABLE);
        DMA_ClearITPendingBit(DMA1_IT_TC2);
    }
}

void DMA1_Channel3_IRQHandler(void) {
    if (DMA_GetITStatus(DMA1_IT_TC3) == SET) {
        DMA_Cmd(DMA1_Channel3, DISABLE);
        DMA_ClearITPendingBit(DMA1_IT_TC3);
    }
}

// void DMA1_Channel4_IRQHandler(void) {
//     if (DMA_GetITStatus(DMA1_IT_TC4) == SET) {
//         DMA_Cmd(DMA1_Channel4, DISABLE);
//         DMA_ClearITPendingBit(DMA1_IT_TC4);
//     }
// }

// void DMA1_Channel5_IRQHandler(void) {
//     if (DMA_GetITStatus(DMA1_IT_TC5) == SET) {
//         DMA_Cmd(DMA1_Channel5, DISABLE);
//         DMA_ClearITPendingBit(DMA1_IT_TC5);
//     }
// }
