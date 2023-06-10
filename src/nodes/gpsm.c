/*
 * gpsm.c
 *
 *  Created on: 8 jun. 2023
 *      Author: Ludo
 */

#include "gpsm.h"

#include "adc.h"
#include "dinfox_common.h"
#include "load.h"
#include "neom8n.h"
#include "gpsm_reg.h"
#include "node.h"
#include "usart.h"

#ifdef GPSM

/*** GPSM local macros ***/

#define GPSM_REG_TIMEOUT_DEFAULT_VALUE						0x00B40078

#define GPSM_REG_TIMEPULSE_CONFIGURATION_0_DEFAULT_VALUE	0x00989680
#define GPSM_REG_TIMEPULSE_CONFIGURATION_1_DEFAULT_VALUE	0x00000032

#define GPSM_REG_TIME_DATA_0_YEAR_OFFSET					2000

/*** GPSM local structures ***/

typedef union {
	struct {
		unsigned gps_power : 1;
		unsigned tpen : 1;
		unsigned pwmd : 1;
		unsigned pwen : 1;
		unsigned bken : 1;
	};
	uint8_t all;
} GPSM_flags_t;

/*** GPSM local global variables ***/

static GPSM_flags_t gpsm_flags;

/*** GPSM local functions ***/

/* CONTROL GPS POWER SUPPLY.
 * @param state:	0 to turn GPS on, 1 to turn GPS on.
 * @return status:	Function execution status.
 */
static NODE_status_t _GPSM_power_control(uint8_t state) {
	// Local variables.
	NODE_status_t status = NODE_SUCCESS;
	USART_status_t usart2_status = USART_SUCCESS;
	// Check on transition.
	if ((state != 0) && (gpsm_flags.gps_power == 0)) {
		// Turn GPS on.
		usart2_status = USART2_power_on();
		USART2_status_check(NODE_ERROR_BASE_USART2);
	}
	// Check on transition.
	if ((state == 0) && (gpsm_flags.gps_power != 0)) {
		// Turn GPS off.
		USART2_power_off();
	}
	// Update local flag.
	gpsm_flags.gps_power = (state == 0) ? 0 : 1;
errors:
	return status;
}

/* MANAGE GPS POWER SUPPLY.
 * @param state:	0 to turn GPS on, 1 to turn GPS on.
 * @return status:	Function execution status.
 */
static NODE_status_t _GPSM_power_request(uint8_t state) {
	// Local variables.
	NODE_status_t status = NODE_SUCCESS;
	uint32_t status_control_1 = 0;
	// Get power mode.
	status = NODE_read_register(NODE_REQUEST_SOURCE_INTERNAL, GPSM_REG_ADDR_STATUS_CONTROL_1, &status_control_1);
	if (status != NODE_SUCCESS) goto errors;
	// Check power mode.
	if ((status_control_1 & GPSM_REG_STATUS_CONTROL_1_MASK_PWMD) == 0) {
		// Power managed by the node.
		if ((state == 0) && ((status_control_1 & 0x00000015) == 0)) {
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
		if (((status_control_1 & GPSM_REG_STATUS_CONTROL_1_MASK_PWEN) == 0) && (state != 0)) {
			status = NODE_ERROR_RADIO_POWER;
			goto errors;
		}
	}
errors:
	return status;
}

/* PERFORM GPS TIME FIX.
 * @param:			None.
 * @return status:	Function execution status.
 */
static NODE_status_t _GPSM_ttrg_callback(void) {
	// Local variables.
	NODE_status_t status = NODE_SUCCESS;
	NEOM8N_status_t neom8n_status = NEOM8N_SUCCESS;
	uint32_t timeout_seconds = 0;
	RTC_time_t gps_time;
	uint32_t time_fix_duration = 0;
	// Turn GPS on.
	status = _GPSM_power_request(1);
	if (status != NODE_SUCCESS) goto errors;
	// Read timeout.
	status = NODE_read_field(NODE_REQUEST_SOURCE_INTERNAL, GPSM_REG_ADDR_TIMEOUT, GPSM_REG_TIMEOUT_MASK_TIME_TIMEOUT, &timeout_seconds);
	if (status != NODE_SUCCESS) goto errors;
	// Reset status flag.
	NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, GPSM_REG_ADDR_STATUS_CONTROL_1, GPSM_REG_STATUS_CONTROL_1_MASK_TFS, 0);
	// Perform time fix.
	neom8n_status = NEOM8N_get_time(&gps_time, timeout_seconds, &time_fix_duration);
	// Check status.
	if (neom8n_status == NEOM8N_SUCCESS) {
		// Fill registers with time data.
		NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, GPSM_REG_ADDR_TIME_DATA_0, GPSM_REG_TIME_DATA_0_MASK_YEAR, (uint32_t) (gps_time.year - GPSM_REG_TIME_DATA_0_YEAR_OFFSET));
		NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, GPSM_REG_ADDR_TIME_DATA_0, GPSM_REG_TIME_DATA_0_MASK_MONTH, (uint32_t) gps_time.month);
		NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, GPSM_REG_ADDR_TIME_DATA_0, GPSM_REG_TIME_DATA_0_MASK_DATE, (uint32_t) gps_time.date);
		NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, GPSM_REG_ADDR_TIME_DATA_1, GPSM_REG_TIME_DATA_1_MASK_HOUR, (uint32_t) gps_time.hours);
		NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, GPSM_REG_ADDR_TIME_DATA_1, GPSM_REG_TIME_DATA_1_MASK_MINUTE, (uint32_t) gps_time.minutes);
		NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, GPSM_REG_ADDR_TIME_DATA_1, GPSM_REG_TIME_DATA_1_MASK_SECOND, (uint32_t) gps_time.seconds);
		NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, GPSM_REG_ADDR_TIME_DATA_2, GPSM_REG_TIME_DATA_2_MASK_FIX_DURATION, time_fix_duration);
		// Update status flag.
		NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, GPSM_REG_ADDR_STATUS_CONTROL_1, GPSM_REG_STATUS_CONTROL_1_MASK_TFS, 1);
	}
	else {
		// Do not consider timeout error.
		if (neom8n_status != NEOM8N_ERROR_TIME_TIMEOUT) {
			NEOM8N_status_check(NODE_ERROR_BASE_NEOM8N);
		}
	}
errors:
	// Clear flag.
	NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, GPSM_REG_ADDR_STATUS_CONTROL_1, GPSM_REG_STATUS_CONTROL_1_MASK_TTRG, 0);
	// Turn GPS off is possible.
	_GPSM_power_request(0);
	return status;
}

/* PERFORM GPS GEOLOCATION FIX.
 * @param:			None.
 * @return status:	Function execution status.
 */
static NODE_status_t _GPSM_gtrg_callback(void) {
	// Local variables.
	NODE_status_t status = NODE_SUCCESS;
	NEOM8N_status_t neom8n_status = NEOM8N_SUCCESS;
	uint32_t timeout_seconds = 0;
	NEOM8N_position_t gps_position;
	uint32_t geoloc_fix_duration = 0;
	// Turn GPS on.
	status = _GPSM_power_request(1);
	if (status != NODE_SUCCESS) goto errors;
	// Read timeout.
	status = NODE_read_field(NODE_REQUEST_SOURCE_INTERNAL, GPSM_REG_ADDR_TIMEOUT, GPSM_REG_TIMEOUT_MASK_GEOLOC_TIMEOUT, &timeout_seconds);
	if (status != NODE_SUCCESS) goto errors;
	// Reset status flag.
	NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, GPSM_REG_ADDR_STATUS_CONTROL_1, GPSM_REG_STATUS_CONTROL_1_MASK_GFS, 0);
	// Perform time fix.
	neom8n_status = NEOM8N_get_position(&gps_position, timeout_seconds, &geoloc_fix_duration);
	// Check status.
	if (neom8n_status == NEOM8N_SUCCESS) {
		// Fill registers with time data.
		NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, GPSM_REG_ADDR_GEOLOC_DATA_0, GPSM_REG_GEOLOC_DATA_0_MASK_NF, (uint32_t) (gps_position.lat_north_flag));
		NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, GPSM_REG_ADDR_GEOLOC_DATA_0, GPSM_REG_GEOLOC_DATA_0_MASK_SECOND, gps_position.lat_seconds);
		NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, GPSM_REG_ADDR_GEOLOC_DATA_0, GPSM_REG_GEOLOC_DATA_0_MASK_MINUTE, (uint32_t) (gps_position.lat_minutes));
		NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, GPSM_REG_ADDR_GEOLOC_DATA_0, GPSM_REG_GEOLOC_DATA_0_MASK_DEGREE, (uint32_t) (gps_position.lat_degrees));
		NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, GPSM_REG_ADDR_GEOLOC_DATA_1, GPSM_REG_GEOLOC_DATA_1_MASK_EF, (uint32_t) (gps_position.long_east_flag));
		NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, GPSM_REG_ADDR_GEOLOC_DATA_1, GPSM_REG_GEOLOC_DATA_1_MASK_SECOND, gps_position.long_seconds);
		NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, GPSM_REG_ADDR_GEOLOC_DATA_1, GPSM_REG_GEOLOC_DATA_1_MASK_MINUTE, (uint32_t) (gps_position.long_minutes));
		NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, GPSM_REG_ADDR_GEOLOC_DATA_1, GPSM_REG_GEOLOC_DATA_1_MASK_DEGREE, (uint32_t) (gps_position.long_degrees));
		NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, GPSM_REG_ADDR_GEOLOC_DATA_2, GPSM_REG_GEOLOC_DATA_2_MASK_ALTITUDE, gps_position.altitude);
		NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, GPSM_REG_ADDR_GEOLOC_DATA_3, GPSM_REG_GEOLOC_DATA_3_MASK_FIX_DURATION, geoloc_fix_duration);
		// Update status flag.
		NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, GPSM_REG_ADDR_STATUS_CONTROL_1, GPSM_REG_STATUS_CONTROL_1_MASK_GFS, 1);
	}
	else {
		// Do not consider position error.
		if (neom8n_status != NEOM8N_ERROR_POSITION_TIMEOUT) {
			NEOM8N_status_check(NODE_ERROR_BASE_NEOM8N);
		}
	}
errors:
	// Clear flag.
	NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, GPSM_REG_ADDR_STATUS_CONTROL_1, GPSM_REG_STATUS_CONTROL_1_MASK_GTRG, 0);
	// Turn GPS off is possible.
	_GPSM_power_request(0);
	return status;
}

/* MANAGE TIMEPULSE OUTPUT MODE.
 * @param state:	Start (1) or stop (0) timepulse output.
 * @return status:	Function execution status.
 */
static NODE_status_t _GPSM_tpen_callback(uint8_t state) {
	// Local variables.
	NODE_status_t status = NODE_SUCCESS;
	NEOM8N_status_t neom8n_status = NEOM8N_SUCCESS;
	NEOM8N_timepulse_config_t timepulse_config;
	uint32_t generic_u32 = 0;
	// Turn GPS on.
	status = _GPSM_power_request(1);
	if (status != NODE_SUCCESS) goto errors;
	// Set state.
	timepulse_config.active = state;
	// Read frequency.
	status = NODE_read_field(NODE_REQUEST_SOURCE_INTERNAL, GPSM_REG_ADDR_TIMEPULSE_CONFIGURATION_0, GPSM_REG_TIMEPULSE_CONFIGURATION_0_MASK_FREQUENCY, &generic_u32);
	if (status != NODE_SUCCESS) goto errors;
	timepulse_config.frequency_hz = generic_u32;
	// Read duty cycle.
	status = NODE_read_field(NODE_REQUEST_SOURCE_INTERNAL, GPSM_REG_ADDR_TIMEPULSE_CONFIGURATION_1, GPSM_REG_TIMEPULSE_CONFIGURATION_1_MASK_DUTY_CUCLE, &generic_u32);
	if (status != NODE_SUCCESS) goto errors;
	timepulse_config.duty_cycle_percent = (uint8_t) generic_u32;
	// Set timepulse.
	neom8n_status = NEOM8N_configure_timepulse(&timepulse_config);
	NEOM8N_status_check(NODE_ERROR_BASE_NEOM8N);
	// Update local flag.
	gpsm_flags.tpen = (state == 0) ? 0 : 1;
errors:
	// Turn GPS off is possible.
	_GPSM_power_request(0);
	return status;
}

/*** GPSM functions ***/

/* INIT GPSM REGISTERS.
 * @param:			None.
 * @return status:	Function execution status.
 */
NODE_status_t GPSM_init_registers(void) {
	// Local variables.
	NODE_status_t status = NODE_SUCCESS;
	// Init flags.
	gpsm_flags.all = 0;
	// Load default values.
	status = NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, GPSM_REG_ADDR_TIMEOUT, DINFOX_REG_MASK_ALL, GPSM_REG_TIMEOUT_DEFAULT_VALUE);
	if (status != NODE_SUCCESS) goto errors;
	status = NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, GPSM_REG_ADDR_TIMEPULSE_CONFIGURATION_0, DINFOX_REG_MASK_ALL, GPSM_REG_TIMEPULSE_CONFIGURATION_0_DEFAULT_VALUE);
	if (status != NODE_SUCCESS) goto errors;
	status = NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, GPSM_REG_ADDR_TIMEPULSE_CONFIGURATION_1, DINFOX_REG_MASK_ALL, GPSM_REG_TIMEPULSE_CONFIGURATION_1_DEFAULT_VALUE);
	if (status != NODE_SUCCESS) goto errors;
errors:
	return status;
}

/* UPDATE GPSM REGISTER.
 * @param reg_addr:	Address of the register to update.
 * @return status:	Function execution status.
 */
NODE_status_t GPSM_update_register(uint8_t reg_addr) {
	// Local variables.
	NODE_status_t status = NODE_SUCCESS;
	// Check address.
	switch (reg_addr) {
	case GPSM_REG_ADDR_STATUS_CONTROL_1:
		// GPS backup voltage.
		status = NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, GPSM_REG_ADDR_STATUS_CONTROL_1, GPSM_REG_STATUS_CONTROL_1_MASK_BKEN, (uint32_t) (NEOM8N_get_backup()));
		if (status != NODE_SUCCESS) goto errors;
		break;
	default:
		// Nothing to do for other registers.
		break;
	}
errors:
	return status;
}

/* CHECK GPSM NODE ACTIONS.
 * @param reg_addr:	Address of the register to check.
 * @return status:	Function execution status.
 */
NODE_status_t GPSM_check_register(uint8_t reg_addr) {
	// Local variables.
	NODE_status_t status = NODE_SUCCESS;
	uint32_t bit_0 = 0;
	uint32_t bit_1 = 0;
	// Check address.
	switch (reg_addr) {
	case GPSM_REG_ADDR_STATUS_CONTROL_1:
		// Check time fix trigger bit.
		status = NODE_read_field(NODE_REQUEST_SOURCE_INTERNAL, GPSM_REG_ADDR_STATUS_CONTROL_1, GPSM_REG_STATUS_CONTROL_1_MASK_TTRG, &bit_0);
		if (status != NODE_SUCCESS) goto errors;
		// Check bit.
		if (bit_0 != 0) {
			// Start GPS time fix.
			status = _GPSM_ttrg_callback();
			if (status != NODE_SUCCESS) goto errors;
		}
		// Check geolocation fix trigger bit.
		status = NODE_read_field(NODE_REQUEST_SOURCE_INTERNAL, GPSM_REG_ADDR_STATUS_CONTROL_1, GPSM_REG_STATUS_CONTROL_1_MASK_GTRG, &bit_0);
		if (status != NODE_SUCCESS) goto errors;
		// Check bit.
		if (bit_0 != 0) {
			// Start GPS geolocation fix.
			status = _GPSM_gtrg_callback();
			if (status != NODE_SUCCESS) goto errors;
		}
		// Read TPEN bit.
		status = NODE_read_field(NODE_REQUEST_SOURCE_INTERNAL, GPSM_REG_ADDR_STATUS_CONTROL_1, GPSM_REG_STATUS_CONTROL_1_MASK_TPEN, &bit_0);
		if (status != NODE_SUCCESS) goto errors;
		// Check bit change.
		if (bit_0 != gpsm_flags.tpen) {
			// Start timepulse.
			status = _GPSM_tpen_callback(bit_0);
			if (status != NODE_SUCCESS) {
				// Clear request.
				NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, GPSM_REG_ADDR_STATUS_CONTROL_1, GPSM_REG_STATUS_CONTROL_1_MASK_TPEN, gpsm_flags.tpen);
				goto errors;
			}
			// Update local flag.
			gpsm_flags.tpen = bit_0;
		}
		// Read power bits.
		status = NODE_read_field(NODE_REQUEST_SOURCE_INTERNAL, GPSM_REG_ADDR_STATUS_CONTROL_1, GPSM_REG_STATUS_CONTROL_1_MASK_PWMD, &bit_0);
		if (status != NODE_SUCCESS) goto errors;
		// Read power enable bit.
		status = NODE_read_field(NODE_REQUEST_SOURCE_INTERNAL, GPSM_REG_ADDR_STATUS_CONTROL_1, GPSM_REG_STATUS_CONTROL_1_MASK_PWEN, &bit_1);
		if (status != NODE_SUCCESS) goto errors;
		// Check PWMD bit change.
		if ((bit_0 != 0) && (gpsm_flags.pwmd == 0)) {
			// Apply PWEN bit.
			status = _GPSM_power_control(bit_1);
			if (status != NODE_SUCCESS) {
				// Clear request.
				NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, GPSM_REG_ADDR_STATUS_CONTROL_1, GPSM_REG_STATUS_CONTROL_1_MASK_PWMD, gpsm_flags.pwmd);
				goto errors;
			}
			// Update local flag.
			gpsm_flags.pwmd = bit_0;
		}
		// Check PWMD bit change.
		if ((bit_0 == 0) && (gpsm_flags.pwmd != 0)) {
			// Try turning GPS off.
			status = _GPSM_power_request(0);
			if (status != NODE_SUCCESS) {
				// Clear request.
				NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, GPSM_REG_ADDR_STATUS_CONTROL_1, GPSM_REG_STATUS_CONTROL_1_MASK_PWMD, gpsm_flags.pwmd);
				goto errors;
			}
			// Update local flag.
			gpsm_flags.pwmd = bit_0;
		}
		// Check PWEN bit change.
		if ((bit_0 != 0) && (bit_1 != gpsm_flags.pwen)) {
			// Apply PWEN bit.
			status = _GPSM_power_control(bit_1);
			if (status != NODE_SUCCESS) {
				// Clear request.
				NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, GPSM_REG_ADDR_STATUS_CONTROL_1, GPSM_REG_STATUS_CONTROL_1_MASK_PWMD, gpsm_flags.pwmd);
				goto errors;
			}
			// Update local flag.
			gpsm_flags.pwen = bit_1;
		}
		// Read backup voltage control bits.
		status = NODE_read_field(NODE_REQUEST_SOURCE_INTERNAL, GPSM_REG_ADDR_STATUS_CONTROL_1, GPSM_REG_STATUS_CONTROL_1_MASK_BKEN, &bit_0);
		if (status != NODE_SUCCESS) goto errors;
		// Check bit change.
		if (bit_0 != gpsm_flags.bken) {
			// Start timepulse.
			NEOM8N_set_backup(bit_0);
			// Update local flag.
			gpsm_flags.bken = bit_0;
		}
		break;
	case GPSM_REG_ADDR_TIMEPULSE_CONFIGURATION_0:
	case GPSM_REG_ADDR_TIMEPULSE_CONFIGURATION_1:
		// Update timepulse signal if running.
		if (gpsm_flags.tpen != 0) {
			// Start timepulse with new settings.
			status = _GPSM_tpen_callback(1);
			if (status != NODE_SUCCESS) goto errors;
		}
		break;
	default:
		// Nothing to do for other registers.
		break;
	}
errors:
	return status;
}

/* MEASURE TRIGGER CALLBACK.
 * @param:			None.
 * @return status:	Function execution status.
 */
NODE_status_t GPSM_mtrg_callback(void) {
	// Local variables.
	NODE_status_t status = NODE_SUCCESS;
	ADC_status_t adc1_status = ADC_SUCCESS;
	uint32_t adc_data = 0;
	// Turn GPS on.
	status = _GPSM_power_request(1);
	if (status != NODE_SUCCESS) goto errors;
	// Perform measurements.
	adc1_status = ADC1_perform_measurements();
	ADC1_status_check(NODE_ERROR_BASE_ADC);
	// GPS voltage.
	adc1_status = ADC1_get_data(ADC_DATA_INDEX_VGPS_MV, &adc_data);
	ADC1_status_check(NODE_ERROR_BASE_ADC);
	status = NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, GPSM_REG_ADDR_ANALOG_DATA_1, GPSM_REG_ANALOG_DATA_1_MASK_VGPS, (uint32_t) DINFOX_convert_mv(adc_data));
	if (status != NODE_SUCCESS) goto errors;
	// Active antenna voltage.
	adc1_status = ADC1_get_data(ADC_DATA_INDEX_VANT_MV, &adc_data);
	ADC1_status_check(NODE_ERROR_BASE_ADC);
	status = NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, GPSM_REG_ADDR_ANALOG_DATA_1, GPSM_REG_ANALOG_DATA_1_MASK_VANT, (uint32_t) DINFOX_convert_mv(adc_data));
	if (status != NODE_SUCCESS) goto errors;
errors:
	// Turn GPS off is possible.
	_GPSM_power_request(0);
	return status;
}

#endif /* GPSM */