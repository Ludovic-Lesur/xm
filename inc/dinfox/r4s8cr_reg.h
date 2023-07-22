/*
 * r4s8cr_reg.h
 *
 *  Created on: 11 jun. 2023
 *      Author: Ludo
 */

#ifndef __R4S8CR_REG_H__
#define __R4S8CR_REG_H__

#include "types.h"

/*** R4S8CR registers address ***/

/*!******************************************************************
 * \enum R4S8CR_register_address_t
 * \brief R4S8CR registers map.
 *******************************************************************/
typedef enum {
	R4S8CR_REG_ADDR_STATUS_CONTROL = 0,
	R4S8CR_REG_ADDR_LAST,
} R4S8CR_register_address_t;

/*** R4S8CR registers mask ***/

#define R4S8CR_REG_STATUS_CONTROL_MASK_ALL		0x000000FF
#define R4S8CR_REG_STATUS_CONTROL_MASK_R1ST		0x00000001
#define R4S8CR_REG_STATUS_CONTROL_MASK_R2ST		0x00000002
#define R4S8CR_REG_STATUS_CONTROL_MASK_R3ST		0x00000004
#define R4S8CR_REG_STATUS_CONTROL_MASK_R4ST		0x00000008
#define R4S8CR_REG_STATUS_CONTROL_MASK_R5ST		0x00000010
#define R4S8CR_REG_STATUS_CONTROL_MASK_R6ST		0x00000020
#define R4S8CR_REG_STATUS_CONTROL_MASK_R7ST		0x00000040
#define R4S8CR_REG_STATUS_CONTROL_MASK_R8ST		0x00000080

#endif /* __R4S8CR_REG_H__ */
