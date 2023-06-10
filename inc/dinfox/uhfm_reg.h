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

#ifdef UHFM

/*** UHFM registers address ***/

typedef enum {
	UHFM_REG_ADDR_STATUS_CONTROL_1 = COMMON_REG_ADDR_LAST,
	UHFM_REG_ADDR_ANALOG_DATA_1,
	UHFM_REG_ADDR_SIGFOX_EP_ID,
	UHFM_REG_ADDR_SIGFOX_EP_KEY_0,
	UHFM_REG_ADDR_SIGFOX_EP_KEY_1,
	UHFM_REG_ADDR_SIGFOX_EP_KEY_2,
	UHFM_REG_ADDR_SIGFOX_EP_KEY_3,
	UHFM_REG_ADDR_SIGFOX_EP_CONFIGURATION_0,
	UHFM_REG_ADDR_SIGFOX_EP_CONFIGURATION_1,
	UHFM_REG_ADDR_SIGFOX_EP_CONFIGURATION_2,
	UHFM_REG_ADDR_SIGFOX_UL_PAYLOAD_0,
	UHFM_REG_ADDR_SIGFOX_UL_PAYLOAD_1,
	UHFM_REG_ADDR_SIGFOX_UL_PAYLOAD_2,
	UHFM_REG_ADDR_SIGFOX_DL_PAYLOAD_0,
	UHFM_REG_ADDR_SIGFOX_DL_PAYLOAD_1,
	UHFM_REG_ADDR_SIGFOX_DL_PHY_CONTENT_0,
	UHFM_REG_ADDR_SIGFOX_DL_PHY_CONTENT_1,
	UHFM_REG_ADDR_SIGFOX_DL_PHY_CONTENT_2,
	UHFM_REG_ADDR_SIGFOX_DL_PHY_CONTENT_3,
	UHFM_REG_ADDR_RADIO_TEST_0,
	UHFM_REG_ADDR_RADIO_TEST_1,
	NODE_REG_ADDR_LAST,
} UHFM_register_address_t;

/*** UHFM registers mask ***/

#define UHFM_REG_STATUS_CONTROL_1_MASK_STRG						0x00000001
#define UHFM_REG_STATUS_CONTROL_1_MASK_TTRG						0x00000002
#define UHFM_REG_STATUS_CONTROL_1_MASK_DTRG						0x00000004
#define UHFM_REG_STATUS_CONTROL_1_MASK_CWEN						0x00000008
#define UHFM_REG_STATUS_CONTROL_1_MASK_RSEN						0x00000010
#define UHFM_REG_STATUS_CONTROL_1_MASK_MESSAGE_STATUS			0x0000FF00
#define UHFM_REG_STATUS_CONTROL_1_MASK_DL_RSSI					0x00FF0000

#define UHFM_REG_ANALOG_DATA_1_MASK_VRF_TX						0x0000FFFF
#define UHFM_REG_ANALOG_DATA_1_MASK_VRF_RX						0xFFFF0000

#define UHFM_REG_SIGFOX_EP_CONFIGURATION_0_MASK_RC				0x0000000F
#define UHFM_REG_SIGFOX_EP_CONFIGURATION_0_MASK_BR				0x00000030
#define UHFM_REG_SIGFOX_EP_CONFIGURATION_0_MASK_NFR				0x000000C0
#define UHFM_REG_SIGFOX_EP_CONFIGURATION_0_MASK_TX_POWER		0x0000FF00
#define UHFM_REG_SIGFOX_EP_CONFIGURATION_0_MASK_TEST_MODE		0x000F0000

#define UHFM_REG_SIGFOX_EP_CONFIGURATION_1_MASK_TCONF			0xFFFF0000
#define UHFM_REG_SIGFOX_EP_CONFIGURATION_1_MASK_TIFU			0x0000FFFF

#define UHFM_REG_SIGFOX_EP_CONFIGURATION_2_MASK_PRT				0x00000003
#define UHFM_REG_SIGFOX_EP_CONFIGURATION_2_MASK_CMSG			0x00000004
#define UHFM_REG_SIGFOX_EP_CONFIGURATION_2_MASK_MSGT			0x00000038
#define UHFM_REG_SIGFOX_EP_CONFIGURATION_2_MASK_BF				0x00000040
#define UHFM_REG_SIGFOX_EP_CONFIGURATION_2_MASK_UL_PAYLOAD_SIZE	0x00000F80

#define UHFM_REG_SIGFOX_UL_PAYLOAD_0_MASK_BYTE0					0x000000FF
#define UHFM_REG_SIGFOX_UL_PAYLOAD_0_MASK_BYTE1					0x0000FF00
#define UHFM_REG_SIGFOX_UL_PAYLOAD_0_MASK_BYTE2					0x00FF0000
#define UHFM_REG_SIGFOX_UL_PAYLOAD_0_MASK_BYTE3					0xFF000000

#define UHFM_REG_SIGFOX_UL_PAYLOAD_1_MASK_BYTE4					0x000000FF
#define UHFM_REG_SIGFOX_UL_PAYLOAD_1_MASK_BYTE5					0x0000FF00
#define UHFM_REG_SIGFOX_UL_PAYLOAD_1_MASK_BYTE6					0x00FF0000
#define UHFM_REG_SIGFOX_UL_PAYLOAD_1_MASK_BYTE7					0xFF000000

#define UHFM_REG_SIGFOX_UL_PAYLOAD_2_MASK_BYTE8					0x000000FF
#define UHFM_REG_SIGFOX_UL_PAYLOAD_2_MASK_BYTE9					0x0000FF00
#define UHFM_REG_SIGFOX_UL_PAYLOAD_2_MASK_BYTE10				0x00FF0000
#define UHFM_REG_SIGFOX_UL_PAYLOAD_2_MASK_BYTE11				0xFF000000

#define UHFM_REG_SIGFOX_DL_PAYLOAD_0_MASK_BYTE0					0x000000FF
#define UHFM_REG_SIGFOX_DL_PAYLOAD_0_MASK_BYTE1					0x0000FF00
#define UHFM_REG_SIGFOX_DL_PAYLOAD_0_MASK_BYTE2					0x00FF0000
#define UHFM_REG_SIGFOX_DL_PAYLOAD_0_MASK_BYTE3					0xFF000000

#define UHFM_REG_SIGFOX_DL_PAYLOAD_1_MASK_BYTE4					0x000000FF
#define UHFM_REG_SIGFOX_DL_PAYLOAD_1_MASK_BYTE5					0x0000FF00
#define UHFM_REG_SIGFOX_DL_PAYLOAD_1_MASK_BYTE6					0x00FF0000
#define UHFM_REG_SIGFOX_DL_PAYLOAD_1_MASK_BYTE7					0xFF000000

#define UHFM_REG_SIGFOX_DL_PHY_CONTENT_0_MASK_BYTE0				0x000000FF
#define UHFM_REG_SIGFOX_DL_PHY_CONTENT_0_MASK_BYTE1				0x0000FF00
#define UHFM_REG_SIGFOX_DL_PHY_CONTENT_0_MASK_BYTE2				0x00FF0000
#define UHFM_REG_SIGFOX_DL_PHY_CONTENT_0_MASK_BYTE3				0xFF000000

#define UHFM_REG_SIGFOX_DL_PHY_CONTENT_1_MASK_BYTE4				0x000000FF
#define UHFM_REG_SIGFOX_DL_PHY_CONTENT_1_MASK_BYTE5				0x0000FF00
#define UHFM_REG_SIGFOX_DL_PHY_CONTENT_1_MASK_BYTE6				0x00FF0000
#define UHFM_REG_SIGFOX_DL_PHY_CONTENT_1_MASK_BYTE7				0xFF000000

#define UHFM_REG_SIGFOX_DL_PHY_CONTENT_2_MASK_BYTE8				0x000000FF
#define UHFM_REG_SIGFOX_DL_PHY_CONTENT_2_MASK_BYTE9				0x0000FF00
#define UHFM_REG_SIGFOX_DL_PHY_CONTENT_2_MASK_BYTE10			0x00FF0000
#define UHFM_REG_SIGFOX_DL_PHY_CONTENT_2_MASK_BYTE11			0xFF000000

#define UHFM_REG_SIGFOX_DL_PHY_CONTENT_3_MASK_BYTE12			0x000000FF
#define UHFM_REG_SIGFOX_DL_PHY_CONTENT_3_MASK_BYTE13			0x0000FF00
#define UHFM_REG_SIGFOX_DL_PHY_CONTENT_3_MASK_BYTE14			0x00FF0000

#define UHFM_REG_RADIO_TEST_0_MASK_RF_FREQUENCY					DINFOX_REG_MASK_ALL

#define UHFM_REG_RADIO_TEST_1_MASK_TX_POWER						0x000000FF
#define UHFM_REG_RADIO_TEST_1_MASK_RSSI							0x0000FF00

#endif /* UHFM */

#endif /* __UHFM_REG_H__ */