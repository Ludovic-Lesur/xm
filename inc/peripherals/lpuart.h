/*
 * lpuart.h
 *
 *  Created on: 9 jul. 2019
 *      Author: Ludo
 */

#ifndef __LPUART_H__
#define __LPUART_H__

#include "node_common.h"
#include "types.h"

/*** LPUART structures ***/

/*!******************************************************************
 * \enum LPUART_status_t
 * \brief LPUART driver error codes.
 *******************************************************************/
typedef enum {
	LPUART_SUCCESS = 0,
	LPUART_ERROR_NULL_PARAMETER,
	LPUART_ERROR_MODE,
	LPUART_ERROR_LBUS_ADDRESS,
	LPUART_ERROR_TX_TIMEOUT,
	LPUART_ERROR_TC_TIMEOUT,
	LPUART_ERROR_STRING_SIZE,
	LPUART_ERROR_BASE_LAST = 0x0100
} LPUART_status_t;

/*** LPUART functions ***/

/*!******************************************************************
 * \fn LPUART_status_t LPUART1_init(NODE_address_t self_address)
 * \brief Init LPUART1 peripheral.
 * \param[in]  	self_address: RS485 address of the node.
 * \param[out] 	none
 * \retval		Function execution status.
 *******************************************************************/
LPUART_status_t LPUART1_init(NODE_address_t self_address);

/*!******************************************************************
 * \fn void LPUART1_enable_rx(void)
 * \brief Enable LPUART1 RX operation.
 * \param[in]   none
 * \param[out] 	none
 * \retval		none
 *******************************************************************/
void LPUART1_enable_rx(void);

/*!******************************************************************
 * \fn void LPUART1_disable_rx(void)
 * \brief Disable LPUART1 RX operation.
 * \param[in]   none
 * \param[out] 	none
 * \retval		none
 *******************************************************************/
void LPUART1_disable_rx(void);

/*!******************************************************************
 * \fn LPUART_status_t LPUART1_send(uint8_t* data, uint32_t data_size_bytes)
 * \brief Send data over LPUART1.
 * \param[in]	data: Byte array to send.
 * \param[in]	data_size_bytes: Number of bytes to send.
 * \param[out] 	none
 * \retval		none
 *******************************************************************/
LPUART_status_t LPUART1_send(uint8_t* data, uint32_t data_size_bytes);

/*******************************************************************/
#define LPUART1_check_status(error_base) { if (lpuart1_status != LPUART_SUCCESS) { status = error_base + lpuart1_status; goto errors; } }

/*******************************************************************/
#define LPUART1_stack_error() { ERROR_stack_error(lpuart1_status, LPUART_SUCCESS, ERROR_BASE_LPUART1); }

/*******************************************************************/
#define LPUART1_print_error() { ERROR_print_error(lpuart1_status, LPUART_SUCCESS, ERROR_BASE_LPUART1); }

#endif /* __LPUART_H__ */
