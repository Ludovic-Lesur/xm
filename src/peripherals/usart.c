/*
 * usart.c
 *
 *  Created on: 15 apr. 2023
 *      Author: Ludo
 */

#include "usart.h"

#include "exti.h"
#include "gpio.h"
#include "mapping.h"
#include "nvic.h"
#include "rcc.h"
#include "rcc_reg.h"
#include "string.h"
#include "types.h"
#include "usart_reg.h"

/*** USART local macros ***/

#define USART_BAUD_RATE			9600
#define USART_TIMEOUT_COUNT		100000

/*** USART local global variables ***/

#ifdef GPSM
static USART_character_match_irq_cb_t usart_cm_irq_callback = NULL;
#endif

/*** USART local functions ***/

#ifdef GPSM
/*******************************************************************/
void __attribute__((optimize("-O0"))) USART2_IRQHandler(void) {
	// Character match interrupt.
	if (((USART2 -> ISR) & (0b1 << 17)) != 0) {
		// Notify upper layer.
		if ((((USART2 -> CR1) & (0b1 << 14)) != 0) && (usart_cm_irq_callback != NULL)) {
			usart_cm_irq_callback();
		}
		// Clear CM flag.
		USART2 -> ICR |= (0b1 << 17);
	}
	// Overrun error interrupt.
	if (((USART2 -> ISR) & (0b1 << 3)) != 0) {
		// Clear ORE flag.
		USART2 -> ICR |= (0b1 << 3);
	}
	EXTI_clear_flag(EXTI_LINE_USART2);
}
#endif

/*** USART functions ***/

#ifdef GPSM
/*******************************************************************/
USART_status_t USART2_init(USART_character_match_irq_cb_t irq_callback) {
	// Local variables.
	USART_status_t status = USART_SUCCESS;
	RCC_status_t rcc_status = RCC_SUCCESS;
	uint32_t usart_clock_hz = 0;
	uint32_t brr = 0;
	// Get clock source frequency.
	rcc_status = RCC_get_frequency_hz(RCC_CLOCK_HSI, &usart_clock_hz);
	RCC_exit_error(USART_ERROR_BASE_RCC);
	// Select HSI as peripheral clock.
	RCC -> CCIPR &= ~(0b11 << 2); // Reset bits 2-3.
	RCC -> CCIPR |= (0b10 << 2); // USART2SEL='10'.
	// Enable peripheral clock.
	RCC -> APB1ENR |= (0b1 << 17); // USART2EN='1'.
	// Configure peripheral.
	USART2 -> CR3 |= (0b1 << 12); // No overrun detection (OVRDIS='0').
	brr = (usart_clock_hz / USART_BAUD_RATE);
	USART2 -> BRR = (brr & 0x000FFFFF); // BRR = (fCK)/(baud rate).
	// Configure character match interrupt and DMA.
	USART2 -> CR2 |= (STRING_CHAR_LF << 24); // LF character used to trigger CM interrupt.
	USART2 -> CR3 |= (0b1 << 6); // Transfer is performed after each RXNE event.
	USART2 -> CR1 |= (0b1 << 14); // Enable CM interrupt (CMIE='1').
	// Enable transmitter and receiver.
	USART2 -> CR1 |= (0b11 << 2); // TE='1' and RE='1'.
	// Enable peripheral.
	USART2 -> CR1 |= (0b1 << 0); // UE='1'.
	// Configure GPIOs.
	GPIO_configure(&GPIO_USART2_TX, GPIO_MODE_ALTERNATE_FUNCTION, GPIO_TYPE_PUSH_PULL, GPIO_SPEED_LOW, GPIO_PULL_NONE);
	GPIO_configure(&GPIO_USART2_RX, GPIO_MODE_ALTERNATE_FUNCTION, GPIO_TYPE_PUSH_PULL, GPIO_SPEED_LOW, GPIO_PULL_NONE);
	// Register callback.
	usart_cm_irq_callback = irq_callback;
errors:
	return status;
}
#endif

#ifdef GPSM
/*******************************************************************/
void USART2_de_init(void) {
	// Disable USART alternate function.
	GPIO_configure(&GPIO_USART2_TX, GPIO_MODE_OUTPUT, GPIO_TYPE_PUSH_PULL, GPIO_SPEED_LOW, GPIO_PULL_NONE);
	GPIO_configure(&GPIO_USART2_RX, GPIO_MODE_OUTPUT, GPIO_TYPE_PUSH_PULL, GPIO_SPEED_LOW, GPIO_PULL_NONE);
	// Disable peripheral.
	USART2 -> CR1 &= ~(0b1 << 0); // UE='0'.
	// Disable peripheral clock.
	RCC -> APB1ENR &= ~(0b1 << 17); // USART2EN='0'.
}
#endif

#ifdef GPSM
/*******************************************************************/
USART_status_t USART2_write(uint8_t* data, uint32_t data_size_bytes) {
	// Local variables.
	USART_status_t status = USART_SUCCESS;
	uint8_t idx = 0;
	uint32_t loop_count = 0;
	// Check parameters.
	if (data == NULL) {
		status = USART_ERROR_NULL_PARAMETER;
		goto errors;
	}
	// Byte loop.
	for (idx=0 ; idx<data_size_bytes ; idx++) {
		// Fill transmit register.
		USART2 -> TDR = data[idx];
		// Wait for transmission to complete.
		while (((USART2 -> ISR) & (0b1 << 7)) == 0) {
			// Wait for TXE='1' or timeout.
			loop_count++;
			if (loop_count > USART_TIMEOUT_COUNT) {
				status = USART_ERROR_TX_TIMEOUT;
				goto errors;
			}
		}
	}
errors:
	return status;
}
#endif
