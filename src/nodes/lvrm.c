/*
 * lvrm.c
 *
 *  Created on: 04 jun. 2023
 *      Author: Ludo
 */

#include "lvrm.h"

#include "adc.h"
#include "dinfox.h"
#include "error.h"
#include "load.h"
#include "lvrm_reg.h"
#include "node.h"

/*** LVRM local macros ***/

// Note: IOUT measurement uses LT6106, OPA187 and optionally TMUX7219 chips whose minimum operating voltage is 4.5V.
#define LVRM_IOUT_MEASUREMENT_VCOM_MIN_MV	4500

/*** LVRM local structures ***/

/*******************************************************************/
typedef struct {
	DINFOX_bit_representation_t rlstst;
} LVRM_context_t;

/*** LVRM global variables ***/

#ifdef LVRM
const DINFOX_register_access_t NODE_REG_ACCESS[LVRM_REG_ADDR_LAST] = {
	COMMON_REG_ACCESS
	DINFOX_REG_ACCESS_READ_ONLY,
	DINFOX_REG_ACCESS_READ_WRITE,
	DINFOX_REG_ACCESS_READ_WRITE,
	DINFOX_REG_ACCESS_READ_ONLY,
	DINFOX_REG_ACCESS_READ_WRITE,
	DINFOX_REG_ACCESS_READ_ONLY,
	DINFOX_REG_ACCESS_READ_ONLY
};
#endif

/*** LVRM local global variables ***/

#ifdef LVRM
static LVRM_context_t lvrm_ctx;
#endif

/*** LVRM local functions ***/

#ifdef LVRM
/*******************************************************************/
static void _LVRM_load_fixed_configuration(void) {
	// Local variables.
	uint32_t reg_value = 0;
	uint32_t reg_mask = 0;
	// BMS flag.
#ifdef LVRM_MODE_BMS
	DINFOX_write_field(&reg_value, &reg_mask, 0b1, LVRM_REG_CONFIGURATION_0_MASK_BMSF);
#else
	DINFOX_write_field(&reg_value, &reg_mask, 0b0, LVRM_REG_CONFIGURATION_0_MASK_BMSF);
#endif
	// Relay control mode.
#ifdef LVRM_RLST_FORCED_HARDWARE
	DINFOX_write_field(&reg_value, &reg_mask, 0b1, LVRM_REG_CONFIGURATION_0_MASK_RLFH);
#else
	DINFOX_write_field(&reg_value, &reg_mask, 0b0, LVRM_REG_CONFIGURATION_0_MASK_RLFH);
#endif
	NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, LVRM_REG_ADDR_CONFIGURATION_0, reg_mask, reg_value);
}
#endif

#ifdef LVRM
/*******************************************************************/
static void _LVRM_load_dynamic_configuration(void) {
	// Local variables.
	uint8_t reg_addr = 0;
	uint32_t reg_value = 0;
	// Load configuration registers from NVM.
	for (reg_addr=LVRM_REG_ADDR_CONFIGURATION_1 ; reg_addr<LVRM_REG_ADDR_STATUS_1 ; reg_addr++) {
		// Read NVM.
		reg_value = DINFOX_read_nvm_register(reg_addr);
		// Write register.
		NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, reg_addr, DINFOX_REG_MASK_ALL, reg_value);
	}
}
#endif

#ifdef LVRM
/*******************************************************************/
static void _LVRM_reset_analog_data(void) {
	// Local variables.
	uint32_t reg_analog_data_1 = 0;
	uint32_t reg_analog_data_1_mask = 0;
	uint32_t reg_analog_data_2 = 0;
	uint32_t reg_analog_data_2_mask = 0;
	// VIN / VOUT.
	DINFOX_write_field(&reg_analog_data_1, &reg_analog_data_1_mask, DINFOX_VOLTAGE_ERROR_VALUE, LVRM_REG_ANALOG_DATA_1_MASK_VCOM);
	DINFOX_write_field(&reg_analog_data_1, &reg_analog_data_1_mask, DINFOX_VOLTAGE_ERROR_VALUE, LVRM_REG_ANALOG_DATA_1_MASK_VOUT);
	NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, LVRM_REG_ADDR_ANALOG_DATA_1, reg_analog_data_1_mask, reg_analog_data_1);
	// IOUT.
	DINFOX_write_field(&reg_analog_data_2, &reg_analog_data_2_mask, DINFOX_VOLTAGE_ERROR_VALUE, LVRM_REG_ANALOG_DATA_2_MASK_IOUT);
	NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, LVRM_REG_ADDR_ANALOG_DATA_2, reg_analog_data_2_mask, reg_analog_data_2);
}
#endif

/*** LVRM functions ***/

#ifdef LVRM
/*******************************************************************/
void LVRM_init_registers(void) {
#ifdef NVM_FACTORY_RESET
	// Local variables.
	uint32_t reg_value = 0;
	uint32_t reg_mask = 0;
	// VBATT thresholds in BMS mode.
	DINFOX_write_field(&reg_value, &reg_mask, LVRM_BMS_VBATT_LOW_THRESHOLD_MV,  LVRM_REG_CONFIGURATION_1_MASK_VBATT_LOW_THRESHOLD);
	DINFOX_write_field(&reg_value, &reg_mask, LVRM_BMS_VBATT_HIGH_THRESHOLD_MV, LVRM_REG_CONFIGURATION_1_MASK_VBATT_HIGH_THRESHOLD);
	NODE_write_register(NODE_REQUEST_SOURCE_EXTERNAL, LVRM_REG_ADDR_CONFIGURATION_1, reg_mask, reg_value);
	// IOUT offset.
	reg_value = 0;
	reg_mask = 0;
	DINFOX_write_field(&reg_value, &reg_mask, 0, LVRM_REG_CONFIGURATION_2_MASK_IOUT_OFFSET);
	NODE_write_register(NODE_REQUEST_SOURCE_EXTERNAL, LVRM_REG_ADDR_CONFIGURATION_2, reg_mask, reg_value);
#endif
	// Read init state.
	LVRM_update_register(LVRM_REG_ADDR_STATUS_1);
	// Load defaults values.
	_LVRM_load_fixed_configuration();
	_LVRM_load_dynamic_configuration();
	_LVRM_reset_analog_data();
}
#endif

#ifdef LVRM
/*******************************************************************/
NODE_status_t LVRM_update_register(uint8_t reg_addr) {
	// Local variables.
	NODE_status_t status = NODE_SUCCESS;
	uint32_t reg_value = 0;
	uint32_t reg_mask = 0;
	// Check address.
	switch (reg_addr) {
	case LVRM_REG_ADDR_STATUS_1:
		// Relay state.
#ifdef LVRM_RLST_FORCED_HARDWARE
		lvrm_ctx.rlstst = DINFOX_BIT_FORCED_HARDWARE;
#else
		lvrm_ctx.rlstst = LOAD_get_output_state();
#endif
		DINFOX_write_field(&reg_value, &reg_mask, ((uint32_t) lvrm_ctx.rlstst), LVRM_REG_STATUS_1_MASK_RLSTST);
		break;
	default:
		// Nothing to do for other registers.
		break;
	}
	// Write register.
	NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, reg_addr, reg_mask, reg_value);
	return status;
}
#endif

#ifdef LVRM
/*******************************************************************/
NODE_status_t LVRM_check_register(uint8_t reg_addr, uint32_t reg_mask) {
	// Local variables.
	NODE_status_t status = NODE_SUCCESS;
	POWER_status_t power_status = POWER_SUCCESS;
	ADC_status_t adc1_status = ADC_SUCCESS;
#if !(defined LVRM_RLST_FORCED_HARDWARE) && !(defined LVRM_MODE_BMS)
	LOAD_status_t load_status = LOAD_SUCCESS;
	DINFOX_bit_representation_t rlst = DINFOX_BIT_ERROR;
#endif
	uint32_t reg_value = 0;
	uint32_t output_current_ua = 0;
	uint32_t reg_config_2 = 0;
	uint32_t reg_config_2_mask = 0;
	// Read register.
	status = NODE_read_register(NODE_REQUEST_SOURCE_INTERNAL, reg_addr, &reg_value);
	if (status != NODE_SUCCESS) goto errors;
	// Check address.
	switch (reg_addr) {
	case LVRM_REG_ADDR_CONFIGURATION_1:
	case LVRM_REG_ADDR_CONFIGURATION_2:
		// Store new value in NVM.
		if (reg_mask != 0) {
			DINFOX_write_nvm_register(reg_addr, reg_value);
		}
		break;
	case LVRM_REG_ADDR_CONTROL_1:
		// RLST.
		if ((reg_mask & LVRM_REG_CONTROL_1_MASK_RLST) != 0) {
			// Check pin mode.
#ifdef LVRM_RLST_FORCED_HARDWARE
			status = NODE_ERROR_FORCED_HARDWARE;
			goto errors;
#else
#ifdef LVRM_MODE_BMS
			status = NODE_ERROR_FORCED_SOFTWARE;
			goto errors;
#else
			// Read bit.
			rlst = DINFOX_read_field(reg_value, LVRM_REG_CONTROL_1_MASK_RLST);
			// Compare to current state.
			if (rlst != lvrm_ctx.rlstst) {
				// Set relay state.
				load_status = LOAD_set_output_state(rlst);
				LOAD_exit_error(NODE_ERROR_BASE_LOAD);
			}
#endif
#endif
		}
		// ZCCT.
		if ((reg_mask & LVRM_REG_CONTROL_1_MASK_ZCCT) != 0) {
			// Read bit.
			if (DINFOX_read_field(reg_value, LVRM_REG_CONTROL_1_MASK_ZCCT) != 0) {
				// Clear request.
				NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, LVRM_REG_ADDR_CONTROL_1, LVRM_REG_CONTROL_1_MASK_ZCCT, 0);
				// Perform current measurement.
				power_status = POWER_enable(POWER_DOMAIN_ANALOG, LPTIM_DELAY_MODE_ACTIVE);
				POWER_exit_error(NODE_ERROR_BASE_POWER);
				adc1_status = ADC1_perform_measurements();
				ADC1_exit_error(NODE_ERROR_BASE_ADC1);
				power_status = POWER_disable(POWER_DOMAIN_ANALOG);
				POWER_exit_error(NODE_ERROR_BASE_POWER);
				// Get output current.
				adc1_status = ADC1_get_data(ADC_DATA_INDEX_IOUT_UA, &output_current_ua);
				ADC1_exit_error(NODE_ERROR_BASE_ADC1);
				// Write register and NVM.
				DINFOX_write_field(&reg_config_2, &reg_config_2_mask, DINFOX_convert_ua(output_current_ua), LVRM_REG_CONFIGURATION_2_MASK_IOUT_OFFSET);
				NODE_write_register(NODE_REQUEST_SOURCE_EXTERNAL, LVRM_REG_ADDR_CONFIGURATION_2, reg_config_2_mask, reg_config_2);
			}
		}
		break;
	default:
		// Nothing to do for other registers.
		break;
	}
errors:
	// Update status register.
	LVRM_update_register(LVRM_REG_ADDR_STATUS_1);
	return status;
}
#endif

#ifdef LVRM
/*******************************************************************/
NODE_status_t LVRM_mtrg_callback(ADC_status_t* adc_status) {
	// Local variables.
	NODE_status_t status = NODE_SUCCESS;
	POWER_status_t power_status = POWER_SUCCESS;
	ADC_status_t adc1_status = ADC_SUCCESS;
	uint32_t adc_data = 0;
	uint32_t vcom_mv = 0;
	uint32_t lt6106_offset_current_ua = 0;
	uint32_t reg_config_2 = 0;
	uint32_t reg_analog_data_1 = 0;
	uint32_t reg_analog_data_1_mask = 0;
	uint32_t reg_analog_data_2 = 0;
	uint32_t reg_analog_data_2_mask = 0;
	// Reset results.
	_LVRM_reset_analog_data();
	// Perform analog measurements.
	power_status = POWER_enable(POWER_DOMAIN_ANALOG, LPTIM_DELAY_MODE_ACTIVE);
	POWER_exit_error(NODE_ERROR_BASE_POWER);
	adc1_status = ADC1_perform_measurements();
	ADC1_exit_error(NODE_ERROR_BASE_ADC1);
	power_status = POWER_disable(POWER_DOMAIN_ANALOG);
	POWER_exit_error(NODE_ERROR_BASE_POWER);
	// Check status.
	if (adc1_status == ADC_SUCCESS) {
		// Relay common voltage.
		adc1_status = ADC1_get_data(ADC_DATA_INDEX_VCOM_MV, &adc_data);
		ADC1_exit_error(NODE_ERROR_BASE_ADC1);
		if (adc1_status == ADC_SUCCESS) {
			DINFOX_write_field(&reg_analog_data_1, &reg_analog_data_1_mask, (uint32_t) DINFOX_convert_mv(adc_data), LVRM_REG_ANALOG_DATA_1_MASK_VCOM);
		}
		vcom_mv = adc_data;
		// Relay output voltage.
		adc1_status = ADC1_get_data(ADC_DATA_INDEX_VOUT_MV, &adc_data);
		ADC1_exit_error(NODE_ERROR_BASE_ADC1);
		if (adc1_status == ADC_SUCCESS) {
			DINFOX_write_field(&reg_analog_data_1, &reg_analog_data_1_mask, (uint32_t) DINFOX_convert_mv(adc_data), LVRM_REG_ANALOG_DATA_1_MASK_VOUT);
		}
		// Check IOUT measurement validity.
		if (vcom_mv >= LVRM_IOUT_MEASUREMENT_VCOM_MIN_MV) {
			// Relay output current.
			adc1_status = ADC1_get_data(ADC_DATA_INDEX_IOUT_UA, &adc_data);
			ADC1_exit_error(NODE_ERROR_BASE_ADC1);
			if (adc1_status == ADC_SUCCESS) {
				// Read IOUT offset.
				NODE_read_register(NODE_REQUEST_SOURCE_INTERNAL, LVRM_REG_ADDR_CONFIGURATION_2, &reg_config_2);
				lt6106_offset_current_ua = DINFOX_get_ua(DINFOX_read_field(reg_config_2, LVRM_REG_CONFIGURATION_2_MASK_IOUT_OFFSET));
				// Remove offset.
				adc_data = (adc_data < lt6106_offset_current_ua) ? 0 : (adc_data - lt6106_offset_current_ua);
				DINFOX_write_field(&reg_analog_data_2, &reg_analog_data_2_mask, (uint32_t) DINFOX_convert_ua(adc_data), LVRM_REG_ANALOG_DATA_2_MASK_IOUT);
			}
		}
		// Write registers.
		NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, LVRM_REG_ADDR_ANALOG_DATA_1, reg_analog_data_1_mask, reg_analog_data_1);
		NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, LVRM_REG_ADDR_ANALOG_DATA_2, reg_analog_data_2_mask, reg_analog_data_2);
	}
	// Update ADC status.
	if (adc_status != NULL) {
		(*adc_status) = adc1_status;
	}
	return status;
errors:
	POWER_disable(POWER_DOMAIN_ANALOG);
	// Update ADC status.
	if (adc_status != NULL) {
		(*adc_status) = adc1_status;
	}
	return status;
}
#endif

#if (defined LVRM) && (defined LVRM_MODE_BMS)
/*******************************************************************/
NODE_status_t LVRM_bms_process(void) {
	// Local variables.
	NODE_status_t status = NODE_SUCCESS;
	ADC_status_t adc1_status = ADC_SUCCESS;
	LOAD_status_t load_status = LOAD_SUCCESS;
	uint32_t reg_config_1 = 0;
	uint32_t vbatt_mv = 0;
	uint32_t vbatt_low_threshold_mv = 0;
	uint32_t vbatt_high_threshold_mv = 0;
	// Check battery voltage.
	adc1_status = ADC1_get_data(ADC_DATA_INDEX_VCOM_MV, &vbatt_mv);
	ADC1_exit_error(NODE_ERROR_BASE_ADC1);
	// Read thresholds in registers.
	NODE_read_register(NODE_REQUEST_SOURCE_INTERNAL, LVRM_REG_ADDR_CONFIGURATION_1, &reg_config_1);
	// Convert thresholds in mV.
	vbatt_low_threshold_mv =  DINFOX_get_mv(DINFOX_read_field(reg_config_1, LVRM_REG_CONFIGURATION_1_MASK_VBATT_LOW_THRESHOLD));
	vbatt_high_threshold_mv = DINFOX_get_mv(DINFOX_read_field(reg_config_1, LVRM_REG_CONFIGURATION_1_MASK_VBATT_HIGH_THRESHOLD));
	// Check battery voltage.
	if (vbatt_mv < vbatt_low_threshold_mv) {
		// Open relay.
		load_status = LOAD_set_output_state(0);
		LOAD_exit_error(NODE_ERROR_BASE_LOAD);
	}
	if (vbatt_mv > vbatt_high_threshold_mv) {
		// Close relay.
		load_status = LOAD_set_output_state(1);
		LOAD_exit_error(NODE_ERROR_BASE_LOAD);
	}
errors:
	return status;
}
#endif
