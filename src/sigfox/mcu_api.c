/*!*****************************************************************
 * \file    mcu_api.c
 * \brief   MCU drivers.
 *******************************************************************
 * \copyright
 *
 * Copyright (c) 2022, UnaBiz SAS
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  1 Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *  2 Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *  3 Neither the name of UnaBiz SAS nor the names of its contributors may be
 *    used to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY
 * THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 *******************************************************************/

#include "manuf/mcu_api.h"

#ifdef USE_SIGFOX_EP_FLAGS_H
#include "sigfox_ep_flags.h"
#endif
#include "sigfox_types.h"
#include "sigfox_error.h"
#include "adc.h"
#include "aes.h"
#include "at_bus.h"
#include "error.h"
#include "mode.h"
#include "nvm.h"
#include "tim.h"

/*** MCU API local structures ***/

typedef enum {
	MCU_API_ERROR_NULL_PARAMETER = (MCU_API_SUCCESS + 1),
	MCU_API_ERROR_EP_KEY,
	MCU_API_ERROR_BASE_TIM2 = 0x0100,
	MCU_API_ERROR_BASE_NVM = (MCU_API_ERROR_BASE_TIM2 + TIM_ERROR_BASE_LAST),
	MCU_API_ERROR_BASE_AES = (MCU_API_ERROR_BASE_NVM + NVM_ERROR_BASE_LAST),
	MCU_API_ERROR_BASE_ADC = (MCU_API_ERROR_BASE_AES + AES_ERROR_BASE_LAST),
	MCU_API_ERROR_BASE_LAST = (MCU_API_ERROR_BASE_ADC + ADC_ERROR_BASE_LAST),
} MCU_API_custom_status_t;

/*** MCU API functions ***/

#if (defined ASYNCHRONOUS) || (defined LOW_LEVEL_OPEN_CLOSE)
/*******************************************************************/
MCU_API_status_t MCU_API_open(MCU_API_config_t *mcu_api_config) {
	// Local variables.
	MCU_API_status_t status = MCU_API_SUCCESS;
	RETURN();
}
#endif

#ifdef LOW_LEVEL_OPEN_CLOSE
/*******************************************************************/
MCU_API_status_t MCU_API_close(void) {
	// Local variables.
	MCU_API_status_t status = MCU_API_SUCCESS;
	RETURN();
}
#endif

#ifdef ASYNCHRONOUS
/*******************************************************************/
MCU_API_status_t MCU_API_process(void) {
	// Local variables.
	MCU_API_status_t status = MCU_API_SUCCESS;
	RETURN();
}
#endif

#ifdef TIMER_REQUIRED
/*******************************************************************/
MCU_API_status_t MCU_API_timer_start(MCU_API_timer_t *timer) {
	// Local variables.
	MCU_API_status_t status = MCU_API_SUCCESS;
	TIM_status_t tim2_status = TIM_SUCCESS;
	TIM_waiting_mode_t tim2_waiting_mode = TIM_WAITING_MODE_LOW_POWER_SLEEP;
	// Check parameter.
	if (timer == SFX_NULL) {
		status = MCU_API_ERROR_NULL_PARAMETER;
		goto errors;
	}
	// Update waiting mode according to timer reason.
	if ((timer -> reason) == MCU_API_TIMER_REASON_T_RX) {
		// T_RX completion is directly checked with the raw timer status within the RF_API_receive() function.
		// All other timers completion are checked with the MCU_API_timer_wait_cplt() function, using low power sleep waiting mode.
		tim2_waiting_mode = TIM_WAITING_MODE_ACTIVE;
	}
	// Start timer.
	tim2_status = TIM2_start((timer -> instance), (timer -> duration_ms), tim2_waiting_mode);
	TIM2_check_status(MCU_API_ERROR_BASE_TIM2);
errors:
	RETURN();
}
#endif

#ifdef TIMER_REQUIRED
/*******************************************************************/
MCU_API_status_t MCU_API_timer_stop(MCU_API_timer_instance_t timer_instance) {
	// Local variables.
	MCU_API_status_t status = MCU_API_SUCCESS;
	TIM_status_t tim2_status = TIM_SUCCESS;
	// Stop timer.
	tim2_status = TIM2_stop(timer_instance);
	TIM2_check_status(MCU_API_ERROR_BASE_TIM2);
errors:
	RETURN();
}
#endif

#if (defined TIMER_REQUIRED) && !(defined ASYNCHRONOUS)
/*******************************************************************/
MCU_API_status_t MCU_API_timer_status(MCU_API_timer_instance_t timer_instance, sfx_bool *timer_has_elapsed) {
	// Local variables.
	MCU_API_status_t status = MCU_API_SUCCESS;
	TIM_status_t tim2_status = TIM_SUCCESS;
	// Read status.
	tim2_status = TIM2_get_status(timer_instance, timer_has_elapsed);
	TIM2_check_status(MCU_API_ERROR_BASE_TIM2);
errors:
	RETURN();
}
#endif

#if (defined TIMER_REQUIRED) && !(defined ASYNCHRONOUS)
/*******************************************************************/
MCU_API_status_t MCU_API_timer_wait_cplt(MCU_API_timer_instance_t timer_instance) {
	// Local variables.
	MCU_API_status_t status = MCU_API_SUCCESS;
	TIM_status_t tim2_status = TIM_SUCCESS;
	// Wait for timer completion.
	tim2_status = TIM2_wait_completion(timer_instance, TIM_WAITING_MODE_LOW_POWER_SLEEP);
	TIM2_check_status(MCU_API_ERROR_BASE_TIM2);
errors:
	RETURN();
}
#endif

/*******************************************************************/
MCU_API_status_t MCU_API_aes_128_cbc_encrypt(MCU_API_encryption_data_t *aes_data) {
	// Local variables.
	MCU_API_status_t status = MCU_API_SUCCESS;
	NVM_status_t nvm_status = NVM_SUCCESS;
	AES_status_t aes_status = AES_SUCCESS;
	uint8_t idx = 0;
	uint8_t local_key[SIGFOX_EP_KEY_SIZE_BYTES];
	uint8_t init_vector[SIGFOX_EP_KEY_SIZE_BYTES];
	// Get right key.
#ifdef PUBLIC_KEY_CAPABLE
	switch (aes_data -> key) {
	case SIGFOX_EP_KEY_PRIVATE:
		// Retrieve private key from NVM.
		for (idx=0 ; idx<SIGFOX_EP_KEY_SIZE_BYTES ; idx++) {
			nvm_status = NVM_read_byte((NVM_ADDRESS_SIGFOX_EP_KEY + idx), &(local_key[idx]));
			NVM_check_status(MCU_API_ERROR_BASE_NVM);
		}
		break;
	case SIGFOX_EP_KEY_PUBLIC:
		// Use public key.
		for (idx=0 ; idx<SIGFOX_EP_KEY_SIZE_BYTES ; idx++) {
			local_key[idx] = SIGFOX_EP_PUBLIC_KEY[idx];
		}
		break;
	default:
		status = MCU_API_ERROR_EP_KEY;
		goto errors;
	}
#else
	// Retrieve private key from NVM.
	for (idx=0 ; idx<SIGFOX_EP_KEY_SIZE_BYTES ; idx++) {
		nvm_status = NVM_read_byte((NVM_ADDRESS_SIGFOX_DEVICE_KEY + idx), &(local_key[idx]));
		NVM_check_status(MCU_API_ERROR_BASE_NVM);
	}
#endif
	// Use null initialization vector.
	for (idx=0 ; idx<SIGFOX_EP_KEY_SIZE_BYTES ; idx++) init_vector[idx] = 0;
	// Perform AES.
	aes_status = AES_encrypt((aes_data -> data), (aes_data -> data), init_vector, local_key);
	AES_check_status(MCU_API_ERROR_BASE_AES);
errors:
	RETURN();
}

#ifdef CRC_HW
/*******************************************************************/
MCU_API_status_t MCU_API_compute_crc16(sfx_u8 *data, sfx_u8 data_size, sfx_u16 polynom, sfx_u16 *crc) {
	// Local variables.
	MCU_API_status_t status = MCU_API_SUCCESS;
	RETURN();
}
#endif

#if (defined CRC_HW) && (defined BIDIRECTIONAL)
/*******************************************************************/
MCU_API_status_t MCU_API_compute_crc8(sfx_u8 *data, sfx_u8 data_size, sfx_u16 polynom, sfx_u8 *crc) {
	// Local variables.
	MCU_API_status_t status = MCU_API_SUCCESS;
	RETURN();
}
#endif

/*******************************************************************/
MCU_API_status_t MCU_API_get_ep_id(sfx_u8 *ep_id, sfx_u8 ep_id_size_bytes) {
	// Local variables.
	MCU_API_status_t status = MCU_API_SUCCESS;
	NVM_status_t nvm_status = NVM_SUCCESS;
	uint8_t idx = 0;
	// Get device ID.
	for (idx=0 ; idx<ep_id_size_bytes ; idx++) {
		nvm_status = NVM_read_byte((NVM_ADDRESS_SIGFOX_EP_ID + idx), &(ep_id[idx]));
		NVM_check_status(MCU_API_ERROR_BASE_NVM);
	}
errors:
	RETURN();
}

/*******************************************************************/
MCU_API_status_t MCU_API_get_nvm(sfx_u8 *nvm_data, sfx_u8 nvm_data_size_bytes) {
	// Local variables.
	MCU_API_status_t status = MCU_API_SUCCESS;
	NVM_status_t nvm_status = NVM_SUCCESS;
	uint8_t idx = 0;
	// Read data.
	for (idx=0 ; idx<nvm_data_size_bytes ; idx++) {
		nvm_status = NVM_read_byte((NVM_ADDRESS_SIGFOX_EP_LIB_DATA + idx), &(nvm_data[idx]));
		NVM_check_status(MCU_API_ERROR_BASE_NVM);
	}
errors:
	RETURN();
}

/*******************************************************************/
MCU_API_status_t MCU_API_set_nvm(sfx_u8 *nvm_data, sfx_u8 nvm_data_size_bytes) {
	// Local variables.
	MCU_API_status_t status = MCU_API_SUCCESS;
	NVM_status_t nvm_status = NVM_SUCCESS;
	uint8_t idx = 0;
	// Write data.
	for (idx=0 ; idx<nvm_data_size_bytes ; idx++) {
		nvm_status = NVM_write_byte((NVM_ADDRESS_SIGFOX_EP_LIB_DATA + idx), nvm_data[idx]);
		NVM_check_status(MCU_API_ERROR_BASE_NVM);
	}
errors:
	RETURN();
}

#if (defined CONTROL_KEEP_ALIVE_MESSAGE) || (defined BIDIRECTIONAL)
/*******************************************************************/
MCU_API_status_t MCU_API_get_voltage_temperature(sfx_u16 *voltage_idle_mv, sfx_u16 *voltage_tx_mv, sfx_s16 *temperature_tenth_degrees) {
	// Local variables.
	MCU_API_status_t status = MCU_API_SUCCESS;
	ADC_status_t adc1_status = ADC_SUCCESS;
	uint32_t mcu_supply_voltage_mv = 0;
	int8_t mcu_temperature_degrees = 0;
	// Perform measurements.
	adc1_status = ADC1_perform_measurements();
	ADC1_check_status(MCU_API_ERROR_BASE_ADC);
	// Get MCU supply voltage.
	adc1_status = ADC1_get_data(ADC_DATA_INDEX_VMCU_MV, &mcu_supply_voltage_mv);
	ADC1_check_status(MCU_API_ERROR_BASE_ADC);
	(*voltage_idle_mv) = (sfx_u16) mcu_supply_voltage_mv;
	(*voltage_tx_mv) = (sfx_u16) mcu_supply_voltage_mv;
	// Get MCU internal temperature.
	adc1_status = ADC1_get_tmcu(&mcu_temperature_degrees);
	ADC1_check_status(MCU_API_ERROR_BASE_ADC);
	(*temperature_tenth_degrees) = ((sfx_s16) mcu_temperature_degrees) * 10;
errors:
	RETURN();
}
#endif

#ifdef CERTIFICATION
/*******************************************************************/
MCU_API_status_t MCU_API_print_dl_payload(sfx_u8 *dl_payload, sfx_u8 dl_payload_size, sfx_s16 rssi_dbm) {
	// Local variables.
	MCU_API_status_t status = MCU_API_SUCCESS;
#if (defined UHFM) && (defined ATM)
	// Print data on bus.
	AT_BUS_print_dl_payload(dl_payload, dl_payload_size, rssi_dbm);
#endif
	RETURN();
}
#endif

#ifdef VERBOSE
/*******************************************************************/
MCU_API_status_t MCU_API_get_initial_pac(sfx_u8 *initial_pac, sfx_u8 initial_pac_size_bytes) {
	// Local variables.
	MCU_API_status_t status = MCU_API_SUCCESS;
	RETURN();
}
#endif

#ifdef VERBOSE
/*******************************************************************/
MCU_API_status_t MCU_API_get_version(sfx_u8 **version, sfx_u8 *version_size_char) {
	// Local variables.
	MCU_API_status_t status = MCU_API_SUCCESS;
	RETURN();
}
#endif

#ifdef ERROR_CODES
/*******************************************************************/
void MCU_API_error(void) {
	// Nothing to do.
}
#endif
