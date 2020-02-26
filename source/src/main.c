#include <stdio.h>
#include "stm32f30x.h"
#include "debug_trace.h"
#include "cortexm_delay.h"
#ifdef USE_DBGUART
#include "dev_uart.h"
#endif
#ifdef USE_STTERM
#include "stlinky.h"
#endif
#include "mod_led.h"
#include "timer_sched.h"
#include "filter_includes.h"

#define LED_TIMER_MS 500
#define LED_PORT GPIOC
#define LED_PIN GPIO_Pin_13

#define ADC_PORT GPIOA
#define ADC_PIN	GPIO_Pin_0

#define DBG_PIN GPIO_Pin_7
#define DBG_PORT GPIOB

#define SAMPLE_RATE 96000

#define ADC1_DR_ADDRESS     0x50000040
#define DAC_DHR12RD_Address      0x40007420
__IO uint16_t calibration_value = 0;

volatile uint32_t glb_tmr_1ms;
volatile uint32_t glb_tmr_1s;
volatile uint32_t irq_count;
uint32_t trace_levels;

#define NUM_OF_FILTERS 5
F_SIZE (*filter_p[NUM_OF_FILTERS])(F_SIZE sample);

struct tp_io {
	uint32_t adc_sample;
	uint32_t dac_sample;
	uint8_t sample_ready;
};
volatile struct tp_io io;

/* Create the list head for the timer */
static LIST_HEAD(obj_timer_list);

// Declare uart
#ifdef USE_DBGUART
DECLARE_UART_DEV(dbg_uart, USART1, 115200, 256, 10, 1);
#endif

#ifdef USE_OVERCLOCKING
extern uint32_t overclock_stm32f303(void);
#endif

static void TIMER_Config(void);
static void ADC_Config(void);
static void DMA_Config(void);
static void DAC_Config(void);

static inline void main_loop(void)
{
	/* 1 ms timer */
	if (glb_tmr_1ms) {
		glb_tmr_1ms = 0;
		glb_tmr_1s++;
		mod_timer_polling(&obj_timer_list);
	}
	if (glb_tmr_1s >= 1000) {
		glb_tmr_1s = 0;
		if (io.sample_ready) {
			uint32_t count = irq_count;
			irq_count = 0;
			printf("%d\n", (int)count);
			io.sample_ready = 0;
		}
	}
}

void led_on(void *data)
{
	LED_PORT->ODR |= LED_PIN;
}

void led_off(void *data)
{
	LED_PORT->ODR &= ~LED_PIN;
}

void led_init(void *data)
{
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOC, ENABLE);
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = LED_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(LED_PORT, &GPIO_InitStructure);

	LED_PORT->ODR |= LED_PIN;
	TRACE(("init\n"));
}

void dbg_pin_init(void)
{
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = DBG_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(DBG_PORT, &GPIO_InitStructure);

	DBG_PORT->ODR |= DBG_PIN;
}

int main(void)
{
#ifdef USE_OVERCLOCKING
    SystemCoreClock = overclock_stm32f303();
#endif
	if (SysTick_Config(SystemCoreClock / 1000)) {
		/* Capture error */
		while (1);
	}
	delay_init(SystemCoreClock);

    trace_levels_set(
			0
			| TRACE_LEVEL_DEFAULT
			,1);

#ifdef USE_SEMIHOSTING
	initialise_monitor_handles();
#elif USE_STTERM
	stlinky_init();
#elif USE_DBGUART
	// setup uart port
	dev_uart_add(&dbg_uart);
	// set callback for uart rx
 	dbg_uart.fp_dev_uart_cb = NULL;
 	mod_timer_add((void*) &dbg_uart, 5, (void*) &dev_uart_update, &obj_timer_list);
#endif

	/* Declare LED module and initialize it */
	DECLARE_MODULE_LED(led_module, 8, 250);
	mod_led_init(&led_module);
	mod_timer_add((void*) &led_module, led_module.tick_ms, (void*) &mod_led_update, &obj_timer_list);

	/* Declare LED */
	DECLARE_DEV_LED(def_led, &led_module, 1, NULL, &led_init, &led_on, &led_off);
	dev_led_add(&def_led);
	dev_led_set_pattern(&def_led, 0b11001100);

	/* Set your filter here: */
	so_butterworth_lpf_calculate_coeffs(10000, SAMPLE_RATE);
	so_butterworth_hpf_calculate_coeffs(5000, SAMPLE_RATE);
	so_butterworth_hpf_set_offset(2048);
	filter_p[0] = &so_butterworth_hpf_filter;
	filter_p[1] = &so_butterworth_lpf_filter;

	/* Configure peripherals */
	TIMER_Config();
	ADC_Config();
	DAC_Config();
	DMA_Config();

	TRACE(("Program started\n"));

	/* main loop */
	while (1) {
		main_loop();
	}
}

static void ADC_Config(void)
{
	GPIO_InitTypeDef   GPIO_InitStructure;
	ADC_InitTypeDef    ADC_InitStructure;
	ADC_CommonInitTypeDef ADC_CommonInitStructure;

	/* Enable the GPIOC Clock */
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);

	/* Configure the ADC clock */  
	RCC_ADCCLKConfig(RCC_ADC12PLLCLK_Div2);

	/* ADC1 Periph clock enable */
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_ADC12, ENABLE);

	/* Configure PC.1 (ADC Channel7) in analog mode */
	GPIO_InitStructure.GPIO_Pin = ADC_PIN;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AN;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
	GPIO_Init(ADC_PORT, &GPIO_InitStructure);  

	ADC_StructInit(&ADC_InitStructure);

	/* Calibration procedure */
	ADC_VoltageRegulatorCmd(ADC1, ENABLE);

	/* Insert delay equal to 10 us */
	delay_us(10);

	ADC_SelectCalibrationMode(ADC1, ADC_CalibrationMode_Single);
	ADC_StartCalibration(ADC1);

	while(ADC_GetCalibrationStatus(ADC1) != RESET );
	calibration_value = ADC_GetCalibrationValue(ADC1);

	/* Configure the ADC1 in continuous mode */
	ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;
	ADC_CommonInitStructure.ADC_Clock = ADC_Clock_SynClkModeDiv1;
	ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_1;
	ADC_CommonInitStructure.ADC_DMAMode = ADC_DMAMode_OneShot;
	ADC_CommonInitStructure.ADC_TwoSamplingDelay = 0;

	ADC_CommonInit(ADC1, &ADC_CommonInitStructure);

	ADC_InitStructure.ADC_ContinuousConvMode = ADC_ContinuousConvMode_Disable;
	ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
	ADC_InitStructure.ADC_ExternalTrigConvEvent = ADC_ExternalTrigConvEvent_9;
	ADC_InitStructure.ADC_ExternalTrigEventEdge = ADC_ExternalTrigEventEdge_RisingEdge;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_OverrunMode = ADC_OverrunMode_Disable;
	ADC_InitStructure.ADC_AutoInjMode = ADC_AutoInjec_Disable;
	ADC_InitStructure.ADC_NbrOfRegChannel = 1;
	ADC_Init(ADC1, &ADC_InitStructure);

	/* ADC1 regular channel7 configuration */
	ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 1, ADC_SampleTime_181Cycles5);

	/* Enable ADC1 */
	ADC_Cmd(ADC1, ENABLE);

	/* wait for ADRDY */
	while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_RDY));

	/* ADC1 DMA Enable */
	ADC_DMACmd(ADC1, ENABLE);
	ADC_DMAConfig(ADC1, ADC_DMAMode_Circular);

	/* Start ADC1 Software Conversion */ 
	ADC_StartConversion(ADC1);
}

static void TIMER_Config() 
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;

	RCC_HRTIM1CLKConfig(RCC_HRTIM1CLK_PLLCLK);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);

    TIM_TimeBaseStructInit(&TIM_TimeBaseInitStructure);
    TIM_TimeBaseInitStructure.TIM_Period = (72000000 / SAMPLE_RATE) - 1;
    TIM_TimeBaseInitStructure.TIM_Prescaler = 0;
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInitStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseInit(TIM1,&TIM_TimeBaseInitStructure);

	TIM_SelectOutputTrigger(TIM1, TIM_TRGOSource_Update); // ADC_ExternalTrigConv_T2_TRGO

    TIM_Cmd(TIM1, ENABLE); 
}

static void DMA_Config(void)
{
	DMA_InitTypeDef  DMA_InitStructure;
  	NVIC_InitTypeDef NVIC_InitStructure;

	/* Enable DMA1 clock */
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

	DMA_DeInit(DMA1_Channel1);
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)ADC1_DR_ADDRESS;
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)&io.adc_sample;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
	DMA_InitStructure.DMA_BufferSize = 1;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Disable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	DMA_Init(DMA1_Channel1, &DMA_InitStructure);

	/* Enable DMA1 Channel1 Transfer Complete interrupt */
	DMA_ITConfig(DMA1_Channel1, DMA_IT_TC, ENABLE);

	/* Enable DMA1 channel1 IRQ Channel */
	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	/* Enable DMA1 Channel1 transfer */
	DMA_Cmd(DMA1_Channel1, ENABLE);
}

static void DAC_Config(void)
{
	DAC_InitTypeDef   DAC_InitStructure;
	GPIO_InitTypeDef  GPIO_InitStructure;

	/* Enable GPIOA Periph clock --------------------------------------*/
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);

	/* Configure PA.04 (DAC1_OUT1), PA.05 (DAC1_OUT2) as analog */
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_4;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* DAC Periph clock enable */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC, ENABLE);

	/* Initialize DAC structure */
	DAC_StructInit(&DAC_InitStructure);

	/* Fill DAC InitStructure */
	DAC_InitStructure.DAC_Trigger = DAC_Trigger_None;
	DAC_InitStructure.DAC_WaveGeneration = DAC_WaveGeneration_None;
	DAC_InitStructure.DAC_LFSRUnmask_TriangleAmplitude = DAC_LFSRUnmask_Bits2_0;  
	DAC_InitStructure.DAC_Buffer_Switch = DAC_BufferSwitch_Disable;

	/* DAC channel1 Configuration */
	DAC_Init(DAC1, DAC_Channel_1, &DAC_InitStructure);

	/* Enable DAC Channel1: Once the DAC channel1 is enabled, PA.04 is 
	automatically connected to the DAC converter. */
	DAC_Cmd(DAC1, DAC_Channel_1, ENABLE);
}

void DMA1_Channel1_IRQHandler(void)
{
	/* Test on DMA1 Channel1 Transfer Complete interrupt */
	if(DMA_GetITStatus(DMA1_IT_TC1))
	{
		io.sample_ready = 1;
		io.dac_sample = io.adc_sample;
		for (int i=0; i<NUM_OF_FILTERS; i++) {
			if (filter_p[i])
				io.dac_sample = filter_p[i](io.dac_sample);
		}
    	DAC_SetChannel1Data(DAC1, DAC_Align_12b_R, io.dac_sample);
		irq_count++;

		/* Clear DMA1 Channel1 Half Transfer, Transfer Complete and Global interrupt pending bits */
		DMA_ClearITPendingBit(DMA1_IT_TC1);
	}
}