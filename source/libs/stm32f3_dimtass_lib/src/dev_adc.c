/*
 * dev_adc.c
 *
 *  Created on: 10 May 2018
 *      Author: dimtass
 */

#include "dev_adc.h"

#define ADC1_DR_Address    ((uint32_t)0x4001244C)
#define ADC2_DR_Address    ((uint32_t)0x4001284C)
//#define ADC3_DR_Address    ((uint32_t)0x40013C4C)

static DMA_InitTypeDef 	_dma_conf = {0};
static ADC_InitTypeDef	_adc_conf = {0};
static uint8_t _adc_channels = 0;

#define ADC_CH_NUM	18
uint16_t adc_buffer[ADC_CH_NUM];

/**
 * @brief  Initialize ADC module. This module is common for all channels
 */
void adc_module_init(void)
{
	RCC_ADCCLKConfig(RCC_PCLK2_Div6);
	/* Enable ADCx and GPIOA clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);

    /* Enable ADC1 and GPIOA clock */
	RCC_AHBPeriphClockCmd (RCC_AHBPeriph_DMA1, ENABLE);
	_dma_conf.DMA_BufferSize = _adc_channels;
	_dma_conf.DMA_DIR = DMA_DIR_PeripheralSRC;
	_dma_conf.DMA_M2M = DMA_M2M_Disable;
	_dma_conf.DMA_MemoryBaseAddr = (uint32_t)adc_buffer;
	_dma_conf.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	_dma_conf.DMA_MemoryInc = DMA_MemoryInc_Enable;
	_dma_conf.DMA_Mode = DMA_Mode_Circular;
	_dma_conf.DMA_PeripheralBaseAddr = (uint32_t)&ADC1->DR;
	_dma_conf.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	_dma_conf.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	_dma_conf.DMA_Priority = DMA_Priority_High;
	DMA_Init(DMA1_Channel1, &_dma_conf);

	DMA_Cmd (DMA1_Channel1, ENABLE) ;

	_adc_conf.ADC_ContinuousConvMode = ENABLE;
	_adc_conf.ADC_DataAlign = ADC_DataAlign_Right;
	_adc_conf.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
	_adc_conf.ADC_Mode = ADC_Mode_Independent;
	_adc_conf.ADC_NbrOfChannel = 0;
	_adc_conf.ADC_ScanConvMode = ENABLE;
	ADC_Init(ADC1, &_adc_conf);
	// ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_239Cycles5);
	// ADC_RegularChannelConfig(ADC1, ADC_Channel_TempSensor, 2, ADC_SampleTime_239Cycles5);
	// ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_7Cycles5);
	// ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 2, ADC_SampleTime_7Cycles5);
	// ADC_RegularChannelConfig(ADC1, ADC_Channel_2, 3, ADC_SampleTime_7Cycles5);
	// ADC_RegularChannelConfig(ADC1, ADC_Channel_3, 4, ADC_SampleTime_7Cycles5);
	// ADC_RegularChannelConfig(ADC1, ADC_Channel_4, 5, ADC_SampleTime_7Cycles5);
	// ADC_RegularChannelConfig(ADC1, ADC_Channel_5, 6, ADC_SampleTime_7Cycles5);
	// ADC_RegularChannelConfig(ADC1, ADC_Channel_6, 7, ADC_SampleTime_7Cycles5);
	// ADC_RegularChannelConfig(ADC1, ADC_Channel_7, 8, ADC_SampleTime_7Cycles5);
	ADC_Cmd(ADC1 ,ENABLE );
	ADC_DMACmd(ADC1, ENABLE );
	ADC_ResetCalibration(ADC1);
	while(ADC_GetResetCalibrationStatus(ADC1));
	ADC_StartCalibration(ADC1);
	while(ADC_GetCalibrationStatus(ADC1));
	ADC_SoftwareStartConvCmd(ADC1, ENABLE) ;
}


/**
 * @brief Start ADC module
 */
void adc_start(void)
{
	ADC_Cmd(ADC1 ,ENABLE );
	ADC_DMACmd(ADC1, ENABLE );
	ADC_ResetCalibration(ADC1);
	while(ADC_GetResetCalibrationStatus(ADC1));
	ADC_StartCalibration(ADC1);
	while(ADC_GetCalibrationStatus(ADC1));
	ADC_SoftwareStartConvCmd(ADC1, ENABLE) ;
}

/**
 * @brief Stop ADC module
 */
void adc_stop(void)
{
	ADC_Cmd(ADC1 ,DISABLE);
	ADC_DMACmd(ADC1, DISABLE);
	ADC_SoftwareStartConvCmd(ADC1, DISABLE) ;
}


/**
 * @brief Add ADC channel to module
 * @param[in] ch Channel to add to module
 */
void adc_add_channel(struct adc_channel * ch)
{
	/* Check if pins are needed to setup */
	if (ch->port) {
		GPIO_InitTypeDef GPIO_InitStructure;
		RCC_APB2PeriphClockCmd ( RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE ) ;
		GPIO_InitStructure.GPIO_Pin = ch->pin;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(ch->port, &GPIO_InitStructure);
	}
	/* increment index and channels */
	ch->index = ++_adc_channels;

	DMA_Cmd (DMA1_Channel1, DISABLE);
	_dma_conf.DMA_BufferSize = _adc_channels;
	DMA_Init(DMA1_Channel1, &_dma_conf);
	DMA_Cmd (DMA1_Channel1, ENABLE);

	_adc_conf.ADC_NbrOfChannel = _adc_channels;
	ADC_Init(ADC1, &_adc_conf);

	ADC_RegularChannelConfig(ADC1, ch->channel, ch->index, ADC_SampleTime_239Cycles5);
	TRACEL(TRACE_LEVEL_ADC, ("ADC:add->%d-%d\n", ch->channel, ch->index));

}

/**
 * @brief Get the value of the ADC channel index
 * @param[in] index The index of the channel
 */
inline uint16_t adc_get_value(uint8_t index)
{
	if (!index || (index > ADC_CH_NUM))
		return 0;
	else
		return adc_buffer[index - 1];
}

/**
 * @brief The number of the channels
 */
inline uint8_t adc_get_num_of_channels(void)
{
	return _adc_channels;
}