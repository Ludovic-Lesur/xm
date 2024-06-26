/*
 * gpsm.c
 *
 *  Created on: 8 jun. 2023
 *      Author: Ludo
 */

#include "gpsm.h"

#include "adc.h"
#include "dinfox.h"
#include "error.h"
#include "gpsm_reg.h"
#include "load.h"
#include "neom8n.h"
#include "node.h"
#include "power.h"
#include "rtc.h"

/*** GPSM local structures ***/

/*******************************************************************/
typedef union {
	struct {
		unsigned gps_power : 1;
		unsigned tpen : 1;
		unsigned pwmd : 1;
		unsigned pwen : 1;
	};
	uint8_t all;
} GPSM_flags_t;

/*******************************************************************/
typedef struct {
	GPSM_flags_t flags;
	DINFOX_bit_representation_t bkenst;
} GPSM_context_t;

/*** GPSM global variables ***/

#ifdef GPSM
const DINFOX_register_access_t NODE_REG_ACCESS[GPSM_REG_ADDR_LAST] = {
	COMMON_REG_ACCESS
	DINFOX_REG_ACCESS_READ_ONLY,
	DINFOX_REG_ACCESS_READ_WRITE,
	DINFOX_REG_ACCESS_READ_WRITE,
	DINFOX_REG_ACCESS_READ_WRITE,
	DINFOX_REG_ACCESS_READ_ONLY,
	DINFOX_REG_ACCESS_READ_WRITE,
	DINFOX_REG_ACCESS_READ_ONLY,
	DINFOX_REG_ACCESS_READ_ONLY,
	DINFOX_REG_ACCESS_READ_ONLY,
	DINFOX_REG_ACCESS_READ_ONLY,
	DINFOX_REG_ACCESS_READ_ONLY,
	DINFOX_REG_ACCESS_READ_ONLY,
	DINFOX_REG_ACCESS_READ_ONLY,
	DINFOX_REG_ACCESS_READ_ONLY
};
#endif

/*** GPSM local global variables ***/

#ifdef GPSM
static GPSM_context_t gpsm_ctx;
#endif

/*** GPSM local functions ***/

#ifdef GPSM
/*******************************************************************/
static void _GPSM_load_fixed_configuration(void) {
	// Local variables.
	uint32_t reg_value = 0;
	uint32_t reg_mask = 0;
	// Active antenna flag.
#ifdef GPSM_ACTIVE_ANTENNA
	DINFOX_write_field(&reg_value, &reg_mask, 0b1, GPSM_REG_CONFIGURATION_0_MASK_AAF);
#else
	DINFOX_write_field(&reg_value, &reg_mask, 0b0, GPSM_REG_CONFIGURATION_0_MASK_AAF);
#endif
	// Backup output control mode.
#ifdef GPSM_BKEN_FORCED_HARDWARE
	DINFOX_write_field(&reg_value, &reg_mask, 0b1, GPSM_REG_CONFIGURATION_0_MASK_BKFH);
#else
	DINFOX_write_field(&reg_value, &reg_mask, 0b0, GPSM_REG_CONFIGURATION_0_MASK_BKFH);
#endif
	NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, GPSM_REG_ADDR_CONFIGURATION_0, reg_mask, reg_value);
}
#endif

#ifdef GPSM
/*******************************************************************/
static void _GPSM_load_dynamic_configuration(void) {
	// Local variables.
	uint8_t reg_addr = 0;
	uint32_t reg_value = 0;
	// Load configuration registers from NVM.
	for (reg_addr=GPSM_REG_ADDR_CONFIGURATION_1 ; reg_addr<GPSM_REG_ADDR_STATUS_1 ; reg_addr++) {
		// Read NVM.
		reg_value = DINFOX_read_nvm_register(reg_addr);
		// Write register.
		NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, reg_addr, DINFOX_REG_MASK_ALL, reg_value);
	}
}
#endif

#ifdef GPSM
/*******************************************************************/
static void _GPSM_reset_analog_data(void) {
	// Local variables.
	uint32_t reg_analog_data_1 = 0;
	uint32_t reg_analog_data_1_mask = 0;
	// Reset fields to error value.
	DINFOX_write_field(&reg_analog_data_1, &reg_analog_data_1_mask, DINFOX_VOLTAGE_ERROR_VALUE, GPSM_REG_ANALOG_DATA_1_MASK_VGPS);
	DINFOX_write_field(&reg_analog_data_1, &reg_analog_data_1_mask, DINFOX_VOLTAGE_ERROR_VALUE, GPSM_REG_ANALOG_DATA_1_MASK_VANT);
	NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, GPSM_REG_ADDR_ANALOG_DATA_1, reg_analog_data_1_mask, reg_analog_data_1);
}
#endif

#ifdef GPSM
/*******************************************************************/
static NODE_status_t _GPSM_power_control(uint8_t state) {
	// Local variables.
	NODE_status_t status = NODE_SUCCESS;
	POWER_status_t power_status = POWER_SUCCESS;
	// Check on transition.
	if ((state != 0) && (gpsm_ctx.flags.gps_power == 0)) {
		// Turn GPS on.
		power_status = POWER_enable(POWER_DOMAIN_GPS, LPTIM_DELAY_MODE_STOP);
		POWER_exit_error(NODE_ERROR_BASE_POWER);
	}
	// Check on transition.
	if ((state == 0) && (gpsm_ctx.flags.gps_power != 0)) {
		// Turn GPS off.
		power_status = POWER_disable(POWER_DOMAIN_GPS);
		POWER_exit_error(NODE_ERROR_BASE_POWER);
	}
	// Update local flag.
	gpsm_ctx.flags.gps_power = (state == 0) ? 0 : 1;
errors:
	return status;
}
#endif

#ifdef GPSM
/*******************************************************************/
static NODE_status_t _GPSM_power_request(uint8_t state) {
	// Local variables.
	NODE_status_t status = NODE_SUCCESS;
	uint32_t reg_control_1 = 0;
	// Get power mode.
	NODE_read_register(NODE_REQUEST_SOURCE_INTERNAL, GPSM_REG_ADDR_CONTROL_1, &reg_control_1);
	// Check power mode.
	if ((reg_control_1 & GPSM_REG_CONTROL_1_MASK_PWMD) == 0) {
		// Power managed by the node.
		if ((state == 0) && ((reg_control_1 & (GPSM_REG_CONTROL_1_MASK_TTRG | GPSM_REG_CONTROL_1_MASK_GTRG | GPSM_REG_CONTROL_1_MASK_TPEN)) == 0)) {
			// Turn GPS off.
			status = _GPSM_power_control(0);
			if (status != NODE_SUCCESS) goto errors;
		}
		if (state != 0) {
			// Turn GPS on.
			status = _GPSM_power_control(1);
			if (status != NODE_SUCCESS) goto errors;
		}
	}
	else {
		// Power managed by PWEN bit.
		// Rise error only in case of turn on request while PWEN=0.
		if (((reg_control_1 & GPSM_REG_CONTROL_1_MASK_PWEN) == 0) && (state != 0)) {
			status = NODE_ERROR_RADIO_POWER;
			goto errors;
		}
	}
errors:
	return status;
}
#endif

#ifdef GPSM
/*******************************************************************/
static NODE_status_t _GPSM_ttrg_callback(void) {
	// Local variables.
	NODE_status_t status = NODE_SUCCESS;
	NEOM8N_status_t neom8n_status = NEOM8N_SUCCESS;
	RTC_time_t gps_time;
	uint32_t time_fix_duration = 0;
	uint32_t reg_timeout = 0;
	uint32_t reg_status_1 = 0;
	uint32_t reg_status_1_mask = 0;
	uint32_t reg_time_data_0 = 0;
	uint32_t reg_time_data_0_mask = 0;
	uint32_t reg_time_data_1 = 0;
	uint32_t reg_time_data_1_mask = 0;
	uint32_t reg_time_data_2 = 0;
	uint32_t reg_time_data_2_mask = 0;
	// Turn GPS on.
	status = _GPSM_power_request(1);
	if (status != NODE_SUCCESS) goto errors;
	// Read timeout.
	NODE_read_register(NODE_REQUEST_SOURCE_INTERNAL, GPSM_REG_ADDR_CONFIGURATION_1, &reg_timeout);
	// Reset status flag.
	DINFOX_write_field(&reg_status_1, &reg_status_1_mask, 0b0, GPSM_REG_STATUS_1_MASK_TFS);
	// Perform time fix.
	neom8n_status = NEOM8N_get_time(&gps_time, DINFOX_read_field(reg_timeout, GPSM_REG_CONFIGURATION_1_MASK_TIME_TIMEOUT), &time_fix_duration);
	NEOM8N_exit_error(NODE_ERROR_BASE_NEOM8N);
	// Update status flag.
	DINFOX_write_field(&reg_status_1, &reg_status_1_mask, 0b1, GPSM_REG_STATUS_1_MASK_TFS);
	// Fill registers with time data.
	DINFOX_write_field(&reg_time_data_0, &reg_time_data_0_mask, (uint32_t) DINFOX_convert_year(gps_time.year), GPSM_REG_TIME_DATA_0_MASK_YEAR);
	DINFOX_write_field(&reg_time_data_0, &reg_time_data_0_mask, (uint32_t) gps_time.month, GPSM_REG_TIME_DATA_0_MASK_MONTH);
	DINFOX_write_field(&reg_time_data_0, &reg_time_data_0_mask, (uint32_t) gps_time.date, GPSM_REG_TIME_DATA_0_MASK_DATE);
	DINFOX_write_field(&reg_time_data_1, &reg_time_data_1_mask, (uint32_t) gps_time.hours, GPSM_REG_TIME_DATA_1_MASK_HOUR);
	DINFOX_write_field(&reg_time_data_1, &reg_time_data_1_mask, (uint32_t) gps_time.minutes, GPSM_REG_TIME_DATA_1_MASK_MINUTE);
	DINFOX_write_field(&reg_time_data_1, &reg_time_data_1_mask, (uint32_t) gps_time.seconds, GPSM_REG_TIME_DATA_1_MASK_SECOND);
	DINFOX_write_field(&reg_time_data_2, &reg_time_data_2_mask, time_fix_duration, GPSM_REG_TIME_DATA_2_MASK_FIX_DURATION);
	// Write registers.
	NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, GPSM_REG_ADDR_TIME_DATA_0, reg_time_data_0_mask, reg_time_data_0);
	NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, GPSM_REG_ADDR_TIME_DATA_1, reg_time_data_1_mask, reg_time_data_1);
	NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, GPSM_REG_ADDR_TIME_DATA_2, reg_time_data_2_mask, reg_time_data_2);
	// Turn GPS off is possible.
	status = _GPSM_power_request(0);
	if (status != NODE_SUCCESS) goto errors;
errors:
	// Turn GPS off is possible.
	_GPSM_power_request(0);
	return status;
}
#endif

#ifdef GPSM
/*******************************************************************/
static NODE_status_t _GPSM_gtrg_callback(void) {
	// Local variables.
	NODE_status_t status = NODE_SUCCESS;
	NEOM8N_status_t neom8n_status = NEOM8N_SUCCESS;
	NEOM8N_position_t gps_position;
	uint32_t geoloc_fix_duration = 0;
	uint32_t reg_timeout = 0;
	uint32_t reg_status_1 = 0;
	uint32_t reg_status_1_mask = 0;
	uint32_t reg_geoloc_data_0 = 0;
	uint32_t reg_geoloc_data_0_mask = 0;
	uint32_t reg_geoloc_data_1 = 0;
	uint32_t reg_geoloc_data_1_mask = 0;
	uint32_t reg_geoloc_data_2 = 0;
	uint32_t reg_geoloc_data_2_mask = 0;
	uint32_t reg_geoloc_data_3 = 0;
	uint32_t reg_geoloc_data_3_mask = 0;
	// Turn GPS on.
	status = _GPSM_power_request(1);
	if (status != NODE_SUCCESS) goto errors;
	// Read timeout.
	NODE_read_register(NODE_REQUEST_SOURCE_INTERNAL, GPSM_REG_ADDR_CONFIGURATION_1, &reg_timeout);
	// Reset status flag.
	DINFOX_write_field(&reg_status_1, &reg_status_1_mask, 0b0, GPSM_REG_STATUS_1_MASK_GFS);
	// Perform time fix.
	neom8n_status = NEOM8N_get_position(&gps_position, DINFOX_read_field(reg_timeout, GPSM_REG_CONFIGURATION_1_MASK_GEOLOC_TIMEOUT), &geoloc_fix_duration);
	NEOM8N_exit_error(NODE_ERROR_BASE_NEOM8N);
	// Update status flag.
	DINFOX_write_field(&reg_status_1, &reg_status_1_mask, 0b1, GPSM_REG_STATUS_1_MASK_GFS);
	// Fill registers with geoloc data.
	DINFOX_write_field(&reg_geoloc_data_0, &reg_geoloc_data_0_mask, (uint32_t) gps_position.lat_north_flag, GPSM_REG_GEOLOC_DATA_0_MASK_NF);
	DINFOX_write_field(&reg_geoloc_data_0, &reg_geoloc_data_0_mask, gps_position.lat_seconds, GPSM_REG_GEOLOC_DATA_0_MASK_SECOND);
	DINFOX_write_field(&reg_geoloc_data_0, &reg_geoloc_data_0_mask, (uint32_t) gps_position.lat_minutes, GPSM_REG_GEOLOC_DATA_0_MASK_MINUTE);
	DINFOX_write_field(&reg_geoloc_data_0, &reg_geoloc_data_0_mask, (uint32_t) gps_position.lat_degrees, GPSM_REG_GEOLOC_DATA_0_MASK_DEGREE);
	DINFOX_write_field(&reg_geoloc_data_1, &reg_geoloc_data_1_mask, (uint32_t) gps_position.long_east_flag, GPSM_REG_GEOLOC_DATA_1_MASK_EF);
	DINFOX_write_field(&reg_geoloc_data_1, &reg_geoloc_data_1_mask, gps_position.long_seconds, GPSM_REG_GEOLOC_DATA_1_MASK_SECOND);
	DINFOX_write_field(&reg_geoloc_data_1, &reg_geoloc_data_1_mask, (uint32_t) gps_position.long_minutes, GPSM_REG_GEOLOC_DATA_1_MASK_MINUTE);
	DINFOX_write_field(&reg_geoloc_data_1, &reg_geoloc_data_1_mask, (uint32_t) gps_position.long_degrees, GPSM_REG_GEOLOC_DATA_1_MASK_DEGREE);
	DINFOX_write_field(&reg_geoloc_data_2, &reg_geoloc_data_2_mask, gps_position.altitude, GPSM_REG_GEOLOC_DATA_2_MASK_ALTITUDE);
	DINFOX_write_field(&reg_geoloc_data_3, &reg_geoloc_data_3_mask, geoloc_fix_duration, GPSM_REG_GEOLOC_DATA_3_MASK_FIX_DURATION);
	// Write registers.
	NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, GPSM_REG_ADDR_GEOLOC_DATA_0, reg_geoloc_data_0_mask, reg_geoloc_data_0);
	NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, GPSM_REG_ADDR_GEOLOC_DATA_1, reg_geoloc_data_1_mask, reg_geoloc_data_1);
	NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, GPSM_REG_ADDR_GEOLOC_DATA_2, reg_geoloc_data_2_mask, reg_geoloc_data_2);
	NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, GPSM_REG_ADDR_GEOLOC_DATA_3, reg_geoloc_data_3_mask, reg_geoloc_data_3);
	// Turn GPS off is possible.
	status = _GPSM_power_request(0);
	if (status != NODE_SUCCESS) goto errors;
errors:
	// Turn GPS off is possible.
	_GPSM_power_request(0);
	return status;
}
#endif

#ifdef GPSM
/*******************************************************************/
static NODE_status_t _GPSM_tpen_callback(uint8_t state) {
	// Local variables.
	NODE_status_t status = NODE_SUCCESS;
	NEOM8N_status_t neom8n_status = NEOM8N_SUCCESS;
	NEOM8N_timepulse_config_t timepulse_config;
	uint32_t reg_timepulse_configuration_0 = 0;
	uint32_t reg_timepulse_configuration_1 = 0;
	// Turn GPS on.
	status = _GPSM_power_request(1);
	if (status != NODE_SUCCESS) goto errors;
	// Read registers.
	NODE_read_register(NODE_REQUEST_SOURCE_INTERNAL, GPSM_REG_ADDR_CONFIGURATION_2, &reg_timepulse_configuration_0);
	NODE_read_register(NODE_REQUEST_SOURCE_INTERNAL, GPSM_REG_ADDR_CONFIGURATION_3, &reg_timepulse_configuration_1);
	// Set parameters.
	timepulse_config.active = state;
	timepulse_config.frequency_hz = DINFOX_read_field(reg_timepulse_configuration_0, GPSM_REG_CONFIGURATION_2_MASK_TP_FREQUENCY);
	timepulse_config.duty_cycle_percent = (uint8_t) DINFOX_read_field(reg_timepulse_configuration_1, GPSM_REG_CONFIGURATION_3_MASK_TP_DUTY_CYCLE);
	// Set timepulse.
	neom8n_status = NEOM8N_configure_timepulse(&timepulse_config);
	NEOM8N_exit_error(NODE_ERROR_BASE_NEOM8N);
	// Turn GPS off is possible.
	status = _GPSM_power_request(0);
	if (status != NODE_SUCCESS) goto errors;
errors:
	// Turn GPS off is possible.
	_GPSM_power_request(0);
	return status;
}
#endif

/*** GPSM functions ***/

#ifdef GPSM
/*******************************************************************/
void GPSM_init_registers(void) {
#ifdef NVM_FACTORY_RESET
	// Local variables.
	uint32_t reg_value = 0;
	uint32_t reg_mask = 0;
	// Timeouts.
	DINFOX_write_field(&reg_value, &reg_mask, GPSM_TIME_TIMEOUT_SECONDS,   GPSM_REG_CONFIGURATION_1_MASK_TIME_TIMEOUT);
	DINFOX_write_field(&reg_value, &reg_mask, GPSM_GEOLOC_TIMEOUT_SECONDS, GPSM_REG_CONFIGURATION_1_MASK_GEOLOC_TIMEOUT);
	NODE_write_register(NODE_REQUEST_SOURCE_EXTERNAL, GPSM_REG_ADDR_CONFIGURATION_1, reg_mask, reg_value);
	// Timepulse settings.
	reg_value = 0;
	reg_mask = 0;
	DINFOX_write_field(&reg_value, &reg_mask, GPSM_TIMEPULSE_FREQUENCY_HZ, GPSM_REG_CONFIGURATION_2_MASK_TP_FREQUENCY);
	NODE_write_register(NODE_REQUEST_SOURCE_EXTERNAL, GPSM_REG_ADDR_CONFIGURATION_2, reg_mask, reg_value);
	reg_value = 0;
	reg_mask = 0;
	DINFOX_write_field(&reg_value, &reg_mask, GPSM_TIMEPULSE_DUTY_CYCLE, GPSM_REG_CONFIGURATION_3_MASK_TP_DUTY_CYCLE);
	NODE_write_register(NODE_REQUEST_SOURCE_EXTERNAL, GPSM_REG_ADDR_CONFIGURATION_3, reg_mask, reg_value);
#endif
	// Init flags.
	gpsm_ctx.flags.all = 0;
	// Read init state.
	GPSM_update_register(GPSM_REG_ADDR_STATUS_1);
	// Load default values.
	_GPSM_load_fixed_configuration();
	_GPSM_load_dynamic_configuration();
	_GPSM_reset_analog_data();
}
#endif

#ifdef GPSM
/*******************************************************************/
NODE_status_t GPSM_update_register(uint8_t reg_addr) {
	// Local variables.
	NODE_status_t status = NODE_SUCCESS;
	uint32_t reg_value = 0;
	uint32_t reg_mask = 0;
	// Check address.
	switch (reg_addr) {
	case GPSM_REG_ADDR_STATUS_1:
		// Timepulse enable.
		DINFOX_write_field(&reg_value, &reg_mask, ((uint32_t) gpsm_ctx.flags.tpen), GPSM_REG_STATUS_1_MASK_TPST);
		// Power enable.
		DINFOX_write_field(&reg_value, &reg_mask, ((uint32_t) gpsm_ctx.flags.pwen), GPSM_REG_STATUS_1_MASK_PWST);
		// Backup enable.
#ifdef GPSM_BKEN_FORCED_HARDWARE
		gpsm_ctx.bkenst = DINFOX_BIT_FORCED_HARDWARE;
#else
		gpsm_ctx.bkenst = NEOM8N_get_backup();
#endif
		DINFOX_write_field(&reg_value, &reg_mask, ((uint32_t) gpsm_ctx.bkenst), GPSM_REG_STATUS_1_MASK_BKENST);
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

#ifdef GPSM
/*******************************************************************/
NODE_status_t GPSM_check_register(uint8_t reg_addr, uint32_t reg_mask) {
	// Local variables.
	NODE_status_t status = NODE_SUCCESS;
#ifndef GPSM_BKEN_FORCED_HARDWARE
	NEOM8N_status_t neom8n_status = NEOM8N_SUCCESS;
	DINFOX_bit_representation_t bken = 0;
#endif
	DINFOX_bit_representation_t pwmd = 0;
	DINFOX_bit_representation_t pwen = 0;
	DINFOX_bit_representation_t tpen = 0;
	uint32_t reg_value = 0;
	uint32_t new_reg_value = 0;
	uint32_t new_reg_mask = 0;
	// Read register.
	status = NODE_read_register(NODE_REQUEST_SOURCE_INTERNAL, reg_addr, &reg_value);
	if (status != NODE_SUCCESS) goto errors;
	// Check address.
	switch (reg_addr) {
	case GPSM_REG_ADDR_CONFIGURATION_1:
		// Store new value in NVM.
		if (reg_mask != 0) {
			DINFOX_write_nvm_register(reg_addr, reg_value);
		}
		break;
	case GPSM_REG_ADDR_CONFIGURATION_2:
	case GPSM_REG_ADDR_CONFIGURATION_3:
		// Store new value in NVM.
		if (reg_mask != 0) {
			DINFOX_write_nvm_register(reg_addr, reg_value);
		}
		// Update timepulse signal if running.
		if (gpsm_ctx.flags.tpen != 0) {
			// Start timepulse with new settings.
			status = _GPSM_tpen_callback(1);
			if (status != NODE_SUCCESS) goto errors;
		}
		break;
	case GPSM_REG_ADDR_CONTROL_1:
		// TTRG.
		if ((reg_mask & GPSM_REG_CONTROL_1_MASK_TTRG) != 0) {
			// Read bit.
			if (DINFOX_read_field(reg_value, GPSM_REG_CONTROL_1_MASK_TTRG) != 0) {
				// Clear request.
				NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, GPSM_REG_ADDR_CONTROL_1, GPSM_REG_CONTROL_1_MASK_TTRG, 0b0);
				// Start GPS time fix.
				status = _GPSM_ttrg_callback();
				if (status != NODE_SUCCESS) goto errors;
			}
		}
		// GTRG.
		if ((reg_mask & GPSM_REG_CONTROL_1_MASK_GTRG) != 0) {
			// Read bit.
			if (DINFOX_read_field(reg_value, GPSM_REG_CONTROL_1_MASK_GTRG) != 0) {
				// Clear request.
				NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, GPSM_REG_ADDR_CONTROL_1, GPSM_REG_CONTROL_1_MASK_GTRG, 0b0);
				// Start GPS geolocation fix.
				status = _GPSM_gtrg_callback();
				if (status != NODE_SUCCESS) goto errors;
			}
		}
		// TPEN.
		if ((reg_mask & GPSM_REG_CONTROL_1_MASK_TPEN) != 0) {
			// Read bit.
			tpen = DINFOX_read_field(reg_value, GPSM_REG_CONTROL_1_MASK_TPEN);
			// Compare to current state.
			if (tpen != gpsm_ctx.flags.tpen) {
				// Start timepulse.
				status = _GPSM_tpen_callback(tpen);
				if (status != NODE_SUCCESS) {
					// Clear request.
					DINFOX_write_field(&new_reg_value, &new_reg_mask, gpsm_ctx.flags.tpen, GPSM_REG_CONTROL_1_MASK_TPEN);
					goto errors;
				}
				// Update local flag.
				gpsm_ctx.flags.tpen = tpen;
			}
		}
		// PWMD.
		if ((reg_mask & GPSM_REG_CONTROL_1_MASK_TPEN) != 0) {
			// Read bit.
			pwmd = DINFOX_read_field(reg_value, GPSM_REG_CONTROL_1_MASK_PWMD);
			// Check PWMD bit change.
			if ((pwmd != 0) && (gpsm_ctx.flags.pwmd == 0)) {
				// Apply PWEN bit.
				_GPSM_power_control(pwen);
				// Update local flag.
				gpsm_ctx.flags.pwmd = pwmd;
			}
			// Check PWMD bit change.
			if ((pwmd == 0) && (gpsm_ctx.flags.pwmd != 0)) {
				// Try turning GPS off.
				status = _GPSM_power_request(0);
				if (status != NODE_SUCCESS) goto errors;
				// Update local flag.
				gpsm_ctx.flags.pwmd = pwmd;
			}
		}
		// PWEN.
		if ((reg_mask & GPSM_REG_CONTROL_1_MASK_PWEN) != 0) {
			// Check control mode.
			if (pwmd != 0) {
				// Read bit.
				pwen = DINFOX_read_field(reg_value, GPSM_REG_CONTROL_1_MASK_PWEN);
				// Compare to current state.
				if (pwen != gpsm_ctx.flags.pwen) {
					// Apply PWEN bit.
					status = _GPSM_power_control(pwen);
					if (status != NODE_SUCCESS) goto errors;
					// Update local flag.
					gpsm_ctx.flags.pwen = pwen;
				}
			}
			else {
				status = NODE_ERROR_FORCED_SOFTWARE;
				goto errors;
			}
		}
		// BKEN.
		if ((reg_mask & GPSM_REG_CONTROL_1_MASK_BKEN) != 0) {
			// Check pin mode.
#ifdef GPSM_BKEN_FORCED_HARDWARE
			status = NODE_ERROR_FORCED_HARDWARE;
			goto errors;
#else
			// Read bit.
			bken = DINFOX_read_field(reg_value, GPSM_REG_CONTROL_1_MASK_BKEN);
			// Compare to current state.
			if (bken != gpsm_ctx.bkenst) {
				// Set backup voltage.
				neom8n_status = NEOM8N_set_backup(bken);
				NEOM8N_exit_error(NODE_ERROR_BASE_NEOM8N);
			}
#endif
		}
		break;
	default:
		// Nothing to do for other registers.
		break;
	}
errors:
	// Update status register.
	GPSM_update_register(GPSM_REG_ADDR_STATUS_1);
	// Update checked register.
	NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, reg_addr, new_reg_mask, new_reg_value);
	return status;
}
#endif

#ifdef GPSM
/*******************************************************************/
NODE_status_t GPSM_mtrg_callback(ADC_status_t* adc_status) {
	// Local variables.
	NODE_status_t status = NODE_SUCCESS;
	POWER_status_t power_status = POWER_SUCCESS;
	ADC_status_t adc1_status = ADC_SUCCESS;
	uint32_t adc_data = 0;
	uint32_t reg_analog_data_1 = 0;
	uint32_t reg_analog_data_1_mask = 0;
	// Reset results.
	_GPSM_reset_analog_data();
	// Turn GPS on.
	status = _GPSM_power_request(1);
	if (status != NODE_SUCCESS) goto errors;
	// Perform analog measurements.
	power_status = POWER_enable(POWER_DOMAIN_ANALOG, LPTIM_DELAY_MODE_ACTIVE);
	POWER_exit_error(NODE_ERROR_BASE_POWER);
	adc1_status = ADC1_perform_measurements();
	ADC1_exit_error(NODE_ERROR_BASE_ADC1);
	power_status = POWER_disable(POWER_DOMAIN_ANALOG);
	POWER_exit_error(NODE_ERROR_BASE_POWER);
	// Turn GPS off is possible.
	status = _GPSM_power_request(0);
	if (status != NODE_SUCCESS) goto errors;
	// Check status.
	if (adc1_status == ADC_SUCCESS) {
		// GPS voltage.
		adc1_status = ADC1_get_data(ADC_DATA_INDEX_VGPS_MV, &adc_data);
		ADC1_exit_error(NODE_ERROR_BASE_ADC1);
		if (adc1_status == ADC_SUCCESS) {
			DINFOX_write_field(&reg_analog_data_1, &reg_analog_data_1_mask, (uint32_t) DINFOX_convert_mv(adc_data), GPSM_REG_ANALOG_DATA_1_MASK_VGPS);
		}
		// Active antenna voltage.
		adc1_status = ADC1_get_data(ADC_DATA_INDEX_VANT_MV, &adc_data);
		ADC1_exit_error(NODE_ERROR_BASE_ADC1);
		if (adc1_status == ADC_SUCCESS) {
			DINFOX_write_field(&reg_analog_data_1, &reg_analog_data_1_mask, (uint32_t) DINFOX_convert_mv(adc_data), GPSM_REG_ANALOG_DATA_1_MASK_VANT);
		}
		// Write register.
		NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, GPSM_REG_ADDR_ANALOG_DATA_1, reg_analog_data_1_mask, reg_analog_data_1);
	}
	// Update ADC status.
	if (adc_status != NULL) {
		(*adc_status) = adc1_status;
	}
	return status;
errors:
	// Turn GPS off is possible.
	_GPSM_power_request(0);
	// Update ADC status.
	if (adc_status != NULL) {
		(*adc_status) = adc1_status;
	}
	return status;
}
#endif
