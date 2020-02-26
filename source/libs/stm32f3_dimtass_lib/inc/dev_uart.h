/*
 * dev_uart.h
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
 * Usage:
 * // Declare uart
 * DECLARE_UART_DEV(dbg_uart, USART1, 115200, 256, 10, 1);
 * // setup uart port
 * dev_uart_add(&dbg_uart);
 * // set callback for uart rx
 * dbg_uart.fp_dev_uart_cb = dbg_uart_parser;
 * dev_timer_add((void*) &dbg_uart, 5, (void*) &dev_uart_update, &dev_timer_list);

 * If not timer module is used then:
 * Add this line to an 1ms timer interrupt:
 * 		if (m_buff.rx_ready) m_buff.rx_ready_tmr++;
 * Add this line to the __io_putchar(int ch) function (used for printf)
 * 		debug_uart_send(ch);
 * Add this line to the main loop in order to check for incoming data
 * 		debug_uart_rx_poll()
 * Add this function in the interrupt handler of the used UART port
 * 		debug_uart_irq()
 */

#ifndef DEV_UART_H_
#define DEV_UART_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "stm32f30x.h"
#include "comm_buffer.h"

#define DECLARE_UART_DEV(NAME, PORT, BAUDRATE, BUFFER_SIZE, TIMEOUT_MS, DEBUG) \
	struct dev_uart NAME = { \
		.port = PORT, \
		.config = { \
			.USART_BaudRate = BAUDRATE, \
			.USART_WordLength = USART_WordLength_8b, \
			.USART_StopBits = USART_StopBits_1, \
			.USART_Parity = USART_Parity_No, \
			.USART_Mode = USART_Mode_Rx | USART_Mode_Tx, \
			.USART_HardwareFlowControl = USART_HardwareFlowControl_None, \
		}, \
		.debug = DEBUG, \
		.timeout_ms = TIMEOUT_MS, \
		.uart_buff = { \
			.tx_buffer_size = BUFFER_SIZE, \
			.rx_buffer_size = BUFFER_SIZE, \
		}, \
		.fp_dev_uart_cb = NULL, \
	}

/**
 * @brief Callback function definition for reception
 * @param[in] buffer Pointer to the RX buffer
 * @param[in] bufferlen The length of the received data
 */
typedef void (*dev_uart_cb)(uint8_t *buffer, size_t bufferlen, uint8_t sender);

struct dev_uart {
	USART_TypeDef*		port;
	USART_InitTypeDef	config;
	NVIC_InitTypeDef	nvic;
	uint8_t				debug;
	uint8_t				timeout_ms;
	uint8_t				available;
	volatile struct tp_comm_buffer uart_buff;
	/**
	* @brief Callback function definition for reception
	* @param[in] buffer Pointer to the RX buffer
	* @param[in] bufferlen The length of the received data
	*/
	dev_uart_cb fp_dev_uart_cb;
};

void dev_uart_add(struct dev_uart * uart);
void dev_uart_remove(struct dev_uart * dev);
void dev_uart_irq(struct dev_uart * dev);
size_t dev_uart_send_buffer(struct dev_uart * dev, uint8_t * buffer, size_t buffer_len);
void dev_uart_set_baud_rate(struct dev_uart * dev, uint32_t baudrate);
void dev_uart_update(struct dev_uart * uart);

#ifdef __cplusplus
}
#endif

#endif /* DEBUG_UART_H_ */
