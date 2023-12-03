/*
 * uhfm_reg.h
 *
 *  Created on: 31 mar. 2022
 *      Author: Ludo
 */

#ifndef __UHFM_REG_H__
#define __UHFM_REG_H__

#include "common_reg.h"
#include "types.h"

/*** UHFM registers address ***/

/*!******************************************************************
 * \enum UHFM_register_address_t
 * \brief UHFM registers map.
 *******************************************************************/
typedef enum {
	UHFM_REG_ADDR_CONFIGURATION_0 = COMMON_REG_ADDR_LAST,
	UHFM_REG_ADDR_CONFIGURATION_1,
	UHFM_REG_ADDR_CONFIGURATION_2,
	UHFM_REG_ADDR_CONFIGURATION_3,
	UHFM_REG_ADDR_CONFIGURATION_4,
	UHFM_REG_ADDR_STATUS_1,
	UHFM_REG_ADDR_CONTROL_1,
	UHFM_REG_ADDR_ANALOG_DATA_1,
	UHFM_REG_ADDR_SIGFOX_EP_ID,
	UHFM_REG_ADDR_SIGFOX_EP_KEY_0,
	UHFM_REG_ADDR_SIGFOX_EP_KEY_1,
	UHFM_REG_ADDR_SIGFOX_EP_KEY_2,
	UHFM_REG_ADDR_SIGFOX_EP_KEY_3,
	UHFM_REG_ADDR_SIGFOX_UL_PAYLOAD_0,
	UHFM_REG_ADDR_SIGFOX_UL_PAYLOAD_1,
	UHFM_REG_ADDR_SIGFOX_UL_PAYLOAD_2,
	UHFM_REG_ADDR_SIGFOX_DL_PAYLOAD_0,
	UHFM_REG_ADDR_SIGFOX_DL_PAYLOAD_1,
	UHFM_REG_ADDR_SIGFOX_DL_PHY_CONTENT_0,
	UHFM_REG_ADDR_SIGFOX_DL_PHY_CONTENT_1,
	UHFM_REG_ADDR_SIGFOX_DL_PHY_CONTENT_2,
	UHFM_REG_ADDR_SIGFOX_DL_PHY_CONTENT_3,
	UHFM_REG_ADDR_LAST,
} UHFM_register_address_t;

/*** UHFM number of specific registers ***/

#define UHFM_NUMBER_OF_SPECIFIC_REG						(UHFM_REG_ADDR_LAST - COMMON_REG_ADDR_LAST)

/*** UHFM registers mask ***/

#define UHFM_REG_CONFIGURATION_0_MASK_RC				0x0000000F
#define UHFM_REG_CONFIGURATION_0_MASK_BR				0x00000030
#define UHFM_REG_CONFIGURATION_0_MASK_NFR				0x000000C0
#define UHFM_REG_CONFIGURATION_0_MASK_TX_POWER			0x0000FF00
#define UHFM_REG_CONFIGURATION_0_MASK_TEST_MODE			0x000F0000

#define UHFM_REG_CONFIGURATION_1_MASK_TCONF				0xFFFF0000
#define UHFM_REG_CONFIGURATION_1_MASK_TIFU				0x0000FFFF

#define UHFM_REG_CONFIGURATION_2_MASK_PRT				0x00000003
#define UHFM_REG_CONFIGURATION_2_MASK_CMSG				0x00000004
#define UHFM_REG_CONFIGURATION_2_MASK_MSGT				0x00000038
#define UHFM_REG_CONFIGURATION_2_MASK_BF				0x00000040
#define UHFM_REG_CONFIGURATION_2_MASK_UL_PAYLOAD_SIZE	0x00000F80

#define UHFM_REG_CONFIGURATION_3_MASK_RF_FREQUENCY		DINFOX_REG_MASK_ALL

#define UHFM_REG_CONFIGURATION_4_MASK_TX_POWER			0x000000FF
#define UHFM_REG_CONFIGURATION_4_MASK_RSSI				0x0000FF00

#define UHFM_REG_STATUS_1_MASK_MESSAGE_STATUS			0x000000FF
#define UHFM_REG_STATUS_1_MASK_DL_RSSI					0x0000FF00
#define UHFM_REG_STATUS_1_MASK_BIDIRECTIONAL_MC			0x0FFF0000

#define UHFM_REG_CONTROL_1_MASK_STRG					0x00000001
#define UHFM_REG_CONTROL_1_MASK_TTRG					0x00000002
#define UHFM_REG_CONTROL_1_MASK_CWEN					0x00000004
#define UHFM_REG_CONTROL_1_MASK_RSEN					0x00000008

#define UHFM_REG_ANALOG_DATA_1_MASK_VRF_TX				0x0000FFFF
#define UHFM_REG_ANALOG_DATA_1_MASK_VRF_RX				0xFFFF0000

#endif /* __UHFM_REG_H__ */
