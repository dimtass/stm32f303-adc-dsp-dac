 /*
 * dev_pwm.c
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
 */

#include "stm32f30x.h"
#include "dev_pwm.h"

#define DEBUG(X) TRACEL(TRACE_LEVEL_PWM, X)

static void pwm_channel_update(struct pwm_device * pwm);

struct pwm_chip m_chip[] = {
    /* TIM1 */
    [PWM1_1] = {1, TIM1, GPIOA, GPIO_Pin_8},
    [PWM1_2] = {2, TIM1, GPIOA, GPIO_Pin_9},
    [PWM1_3] = {3, TIM1, GPIOA, GPIO_Pin_10},
    [PWM1_4] = {4, TIM1, GPIOA, GPIO_Pin_11},
    /* TIM2 */
    [PWM2_1] = {1, TIM2, GPIOA, GPIO_Pin_0},
    [PWM2_2] = {2, TIM2, GPIOA, GPIO_Pin_1},
    [PWM2_3] = {3, TIM2, GPIOA, GPIO_Pin_2},
    [PWM2_4] = {4, TIM2, GPIOA, GPIO_Pin_3},
    /* TIM3 */
    [PWM3_1] = {1, TIM3, GPIOA, GPIO_Pin_6},
    [PWM3_2] = {2, TIM3, GPIOA, GPIO_Pin_7},
    [PWM3_3] = {3, TIM3, GPIOB, GPIO_Pin_0},
    [PWM3_4] = {4, TIM3, GPIOB, GPIO_Pin_1},
    /* TIM4 */
    [PWM4_1] = {1, TIM4, GPIOB, GPIO_Pin_6},
    [PWM4_2] = {2, TIM4, GPIOB, GPIO_Pin_7},
    [PWM4_3] = {3, TIM4, GPIOB, GPIO_Pin_8},
    [PWM4_4] = {4, TIM4, GPIOB, GPIO_Pin_9},
};


/**
 * @brief  Add a PWM channel
 * @param[in] pwm_channel The PWM channel to enable/add (see en_pwm_channel_id)
 * @param[in] pwm Pointer to the PWM pwm_device
 * @param[in] freq The frequency of the PWM
 * @return int 0, on success
 */
int pwm_add(enum en_pwm_channel_id pwm_channel, struct pwm_device * pwm, uint32_t freq)
{
    if (!pwm)
        return -1;

    if (pwm_channel >= PWM_EOL)
        return -1;
    
    /* point to the hardware timer */
    pwm->chip = &m_chip[pwm_channel];

    pwm->state.freq = 0;
    pwm->state.period = 0;
    pwm->state.enabled = 0;
    pwm->chip->num = (pwm_channel % 4) + 1;

    /* Time base configuration */
    TIM_TimeBaseInitTypeDef * tb = &pwm->config.time_base;
    TIM_TimeBaseStructInit(tb);

    TIM_TypeDef * timer = pwm->chip->timer;

    /* default channel configuration */
    TIM_OCInitTypeDef * oc = &pwm->config.oc;
    TIM_OCStructInit(oc);
    /* Channel 1, 2,3 and 4 Configuration in PWM mode */
    oc->TIM_Pulse = 0;
    oc->TIM_OCMode = TIM_OCMode_PWM2;
    oc->TIM_OutputState = TIM_OutputState_Disable;
    oc->TIM_OCPolarity = TIM_OCPolarity_Low;
    oc->TIM_OCIdleState = TIM_OCIdleState_Set;

    if (timer == TIM1) {
        DEBUG(("Using TIM1\n"));
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
    }
    else if (timer == TIM2) {
        DEBUG(("Using TIM2\n"));
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    }
    else if (timer == TIM3) {
        DEBUG(("Using TIM3\n"));
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
    }
    else if (timer == TIM4) {
        DEBUG(("Using TIM4\n"));
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
    }

    uint32_t rcc = 0;
    GPIO_TypeDef * port = pwm->chip->port;

    /* GPIO clock enable */
    if (port == GPIOA)
        rcc = RCC_APB2Periph_GPIOA;
    else if (port == GPIOB)
        rcc = RCC_APB2Periph_GPIOB;
    else if (port == GPIOC)
        rcc = RCC_APB2Periph_GPIOC;
    else if (port == GPIOD)
        rcc = RCC_APB2Periph_GPIOD;

    RCC_APB2PeriphClockCmd(rcc | RCC_APB2Periph_AFIO, ENABLE);

    /* Setup GPIOs */
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = pwm->chip->pin;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(port, &GPIO_InitStructure);

    /* TIMx counter enable */
    TIM_Cmd(timer, ENABLE);

    /* TIMx Main Output Enable */
    TIM_CtrlPWMOutputs(timer, ENABLE);

    pwm_config(pwm, freq);
    pwm_channel_update(pwm);
    
    DEBUG(("pwd_add\n"));

    return 0;
}


/**
 * @brief  Configure the PWM channel
 * @param[in] pwm The PWM device to Configure
 * @param[in] freq The frequency of the PWM device
 */
void pwm_config(struct pwm_device * pwm, uint32_t freq)
{
    TIM_TypeDef * timer = pwm->chip->timer;

    pwm->state.freq = freq;
    pwm->state.period = (SystemCoreClock / pwm->state.freq) - 1;

    TIM_TimeBaseInitTypeDef * tb = &pwm->config.time_base;
    
    tb->TIM_Period = pwm->state.period;
    TIM_TimeBaseInit(timer, tb);
}


/**
 * @brief  Update the PWM device configuration
 * @param[in] pwm The PWM device
 */
void pwm_channel_update(struct pwm_device * pwm)
{
    uint8_t channel = pwm->chip->num;
    TIM_TypeDef * timer = pwm->chip->timer;
    TIM_OCInitTypeDef * oc = &pwm->config.oc;

    switch (channel) {
    case 1:
        TIM_OC1Init(timer, oc);
        break;
    case 2:
        TIM_OC2Init(timer, oc);
        break;
    case 3:
        TIM_OC3Init(timer, oc);
        break;
    case 4:
        TIM_OC4Init(timer, oc);
        break;
    };
    
    // DEBUG(("Setting "
    //     "  channel: %d\n"
    //     "  pin: %d\n"
    //     "  pulse: %d\n",
    //     pwm->chip->num,
    //     pwm->chip->pin,
    //     oc->TIM_Pulse
    //     ));
}


/**
 * @brief  Enable the PWM device
 * @param[in] pwm The PWM device
 */
void pwm_enable(struct pwm_device * pwm)
{
    TIM_OCInitTypeDef * TIM_OCInitStructure = &pwm->config.oc;
    TIM_OCInitStructure->TIM_OutputState = TIM_OutputState_Enable;

    pwm->state.enabled = 1;
    pwm_channel_update(pwm);
}


/**
 * @brief  Disable the PWM device
 * @param[in] pwm The PWM device
 */
void pwm_disable(struct pwm_device * pwm)
{
    TIM_OCInitTypeDef * TIM_OCInitStructure = &pwm->config.oc;
    TIM_OCInitStructure->TIM_OutputState = TIM_OutputState_Disable;

    pwm->state.enabled = 0;
    pwm_channel_update(pwm);
}



/**
 * @brief Set the polarity of the PWM
 * @param[in] pwm The PWM device
 */
void pwm_set_polarity(struct pwm_device * pwm, enum en_pwm_polarity polarity)
{
    TIM_OCInitTypeDef * TIM_OCInitStructure = &pwm->config.oc;

    switch(polarity) {
        case PWM_POLARITY_NORMAL:
            TIM_OCInitStructure->TIM_OCPolarity = TIM_OCPolarity_Low;
            break;
        case PWM_POLARITY_INVERSED:
            TIM_OCInitStructure->TIM_OCPolarity = TIM_OCPolarity_High;
            break;
    };
    pwm_channel_update(pwm);
}


/**
 * @brief  Set the PWM duty duty cycle
 * @param[in] pwm The PWM device
 * @param[in] duty_cycle The duty cycle (0-100)
 */
void pwm_set_duty_cycle(struct pwm_device * pwm, float duty_cycle)
{
    TIM_OCInitTypeDef * oc = &pwm->config.oc;
    TIM_TimeBaseInitTypeDef * tb = &pwm->config.time_base;
    tb->TIM_Period = pwm->state.period;
    uint16_t new_val = (uint16_t)(duty_cycle * 10);
    
    pwm->state.duty_cycle = duty_cycle;
    oc->TIM_Pulse = (uint16_t) (((uint32_t) new_val * (tb->TIM_Period - 1)) / 1000);
    pwm_channel_update(pwm);

    DEBUG(("PWM%d.%d: %d [%d/%d]\n",
    pwm->config.id / 4 + 1,
    pwm->chip->num,
    new_val, oc->TIM_Pulse, tb->TIM_Period));
}
