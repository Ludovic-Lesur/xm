/*
 * load.h
 *
 *  Created on: 17 feb. 2023
 *      Author: Ludo
 */

#ifndef __LOAD_H__
#define __LOAD_H__

#include "dinfox.h"
#include "lptim.h"
#include "types.h"

/*** LOAD structures ***/

/*!******************************************************************
 * \enum LOAD_status_t
 * \brief LOAD driver error codes.
 *******************************************************************/
typedef enum {
	// Driver errors.
	LOAD_SUCCESS = 0,
	LOAD_ERROR_FORCED_HARDWARE,
	LOAD_ERROR_STATE,
	// Low level drivers errors.
	LOAD_ERROR_BASE_LPTIM = 0x0100,
	// Last base value.
	LOAD_ERROR_BASE_LAST = (LOAD_ERROR_BASE_LPTIM + LPTIM_ERROR_BASE_LAST)
} LOAD_status_t;

/*** LOAD functions ***/

#if (defined LVRM) || (defined BPSM) || (defined DDRM) || (defined RRM)
/*!******************************************************************
 * \fn void LOAD_init(void)
 * \brief Init load interface.
 * \param[in]  	none
 * \param[out] 	none
 * \retval		none
 *******************************************************************/
void LOAD_init(void);
#endif

#if (defined LVRM) || (defined BPSM) || (defined DDRM) || (defined RRM)
/*!******************************************************************
 * \fn LOAD_status_t LOAD_set_output_state(DINFOX_bit_representation_t state)
 * \brief Set load output state.
 * \param[in]  	state: New state to set.
 * \param[out] 	none
 * \retval		Function execution status.
 *******************************************************************/
LOAD_status_t LOAD_set_output_state(DINFOX_bit_representation_t state);
#endif

#if (defined LVRM) || (defined BPSM) || (defined DDRM) || (defined RRM)
/*!******************************************************************
 * \fn DINFOX_bit_representation_t LOAD_get_output_state(void)
 * \brief Read load output state.
 * \param[in]  	none
 * \param[out] 	none
 * \retval		Load state.
 *******************************************************************/
DINFOX_bit_representation_t LOAD_get_output_state(void);
#endif

#ifdef BPSM
/*!******************************************************************
 * \fn void LOAD_set_charge_state(uint8_t state)
 * \brief Set charge enable state.
 * \param[in]  	state: New state to set.
 * \param[out] 	none
 * \retval		Function execution status.
 *******************************************************************/
LOAD_status_t LOAD_set_charge_state(DINFOX_bit_representation_t state);
#endif

#ifdef BPSM
/*!******************************************************************
 * \fn uint8_t LOAD_get_charge_state(void)
 * \brief Read charge enable state.
 * \param[in]  	none
 * \param[out] 	none
 * \retval		Charge enable state.
 *******************************************************************/
DINFOX_bit_representation_t LOAD_get_charge_state(void);
#endif

#ifdef BPSM
/*!******************************************************************
 * \fn uint8_t LOAD_get_charge_status(void)
 * \brief Read charge status.
 * \param[in]  	none
 * \param[out] 	none
 * \retval		Charge status.
 *******************************************************************/
DINFOX_bit_representation_t LOAD_get_charge_status(void);
#endif

/*******************************************************************/
#define LOAD_exit_error(error_base) { if (load_status != LOAD_SUCCESS) { status = (error_base + load_status); goto errors; } }

/*******************************************************************/
#define LOAD_stack_error(void) { if (load_status != LOAD_SUCCESS) { ERROR_stack_add(ERROR_BASE_LOAD + load_status); } }

/*******************************************************************/
#define LOAD_stack_exit_error(error_code) { if (load_status != LOAD_SUCCESS) { ERROR_stack_add(ERROR_BASE_LOAD + load_status); status = error_code; goto errors; } }

#endif /* __LOAD_H__ */
