/*
 * aes.h
 *
 *  Created on: 19 jun. 2018
 *      Author: Ludo
 */

#ifndef __AES_H__
#define __AES_H__

#include "types.h"

/*** AES structures ***/

/*!******************************************************************
 * \enum AES_status_t
 * \brief AES driver error codes.
 *******************************************************************/
typedef enum {
	AES_SUCCESS = 0,
	AES_ERROR_NULL_PARAMETER,
	AES_ERROR_TIMEOUT,
	AES_ERROR_BASE_LAST = 0x0100
} AES_status_t;

/*** AES functions ***/

#ifdef UHFM
/*!******************************************************************
 * \fn void AES_init(void)
 * \brief Init aes peripheral.
 * \param[in]  	none
 * \param[out] 	none
 * \retval		none
 *******************************************************************/
void AES_init(void);
#endif

#ifdef UHFM
/*!******************************************************************
 * \fn AES_status_t AES_encrypt(uint8_t* data_in, uint8_t* data_out, uint8_t* init_vector, uint8_t* key
 * \brief Compute AES-128.
 * \param[in]  	data_in: Input data.
 * \param[in]	init_vector: Initialization vector.
 * \param[in]	key: AES key.
 * \param[out] 	data_out: Output data.
 * \retval		Function execution status.
 *******************************************************************/
AES_status_t AES_encrypt(uint8_t* data_in, uint8_t* data_out, uint8_t* init_vector, uint8_t* key);
#endif

/*******************************************************************/
#define AES_check_status(error_base) { if (aes_status != AES_SUCCESS) { status = error_base + aes_status; goto errors; } }

/*******************************************************************/
#define AES_stack_error() { ERROR_stack_error(aes_status, AES_SUCCESS, ERROR_BASE_AES); }

/*******************************************************************/
#define AES_print_error() { ERROR_print_error(aes_status, AES_SUCCESS, ERROR_BASE_AES); }

#endif /* __AES_H__ */
