/*
 * ddrm_reg.h
 *
 *  Created on: 27 nov. 2022
 *      Author: Ludo
 */

#ifndef __DDRM_REG_H__
#define __DDRM_REG_H__

#include "common_reg.h"
#include "types.h"

#ifdef DDRM

/*** DDRM registers address ***/

typedef enum {
	DDRM_REG_ADDR_STATUS_CONTROL_1 = COMMON_REG_ADDR_LAST,
	DDRM_REG_ADDR_ANALOG_DATA_1,
	DDRM_REG_ADDR_ANALOG_DATA_2,
	NODE_REG_ADDR_LAST,
} DDRM_register_address_t;

/*** DDRM registers mask ***/

#define DDRM_REG_ANALOG_DATA_1_MASK_VIN		0x0000FFFF
#define DDRM_REG_ANALOG_DATA_1_MASK_VOUT	0xFFFF0000

#define DDRM_REG_ANALOG_DATA_2_MASK_IOUT	0x0000FFFF

#define DDRM_REG_STATUS_CONTROL_1_MASK_DDEN	0x00000001

#endif /* DDRM */

#endif /* __DDRM_REG_H__ */