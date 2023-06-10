/*
 * common.h
 *
 *  Created on: 4 jun. 2023
 *      Author: Ludo
 */

#ifndef __COMMON_H__
#define __COMMON_H__

#include "common_reg.h"
#include "dinfox_common.h"
#include "node.h"
#include "types.h"

/*** DINFOX macros ***/

#define COMMON_REG_ACCESS \
	DINFOX_REG_ACCESS_READ_ONLY,  \
	DINFOX_REG_ACCESS_READ_ONLY,  \
	DINFOX_REG_ACCESS_READ_ONLY,  \
	DINFOX_REG_ACCESS_READ_ONLY,  \
	DINFOX_REG_ACCESS_READ_ONLY,  \
	DINFOX_REG_ACCESS_READ_ONLY,  \
	DINFOX_REG_ACCESS_READ_WRITE, \
	DINFOX_REG_ACCESS_READ_ONLY,  \

/*** DINFOX functions ***/

NODE_status_t COMMON_init_registers(void);
NODE_status_t COMMON_update_register(uint8_t reg_addr);
NODE_status_t COMMON_check_register(uint8_t reg_addr);

#endif /* __COMMON_H__ */