/*
 * led.c
 *
 *  Created on: 22 aug. 2020
 *      Author: Ludo
 */

#include "led.h"

#include "gpio.h"
#include "mapping.h"
#include "tim.h"
#include "types.h"

/*** LED local functions ***/

#if (defined LVRM) || (defined DDRM) || (defined RRM) || (defined GPSM)
/*******************************************************************/
static void _LED_off(void) {
	// Configure pins as output high.
	GPIO_write(&GPIO_LED_RED, 1);
	GPIO_write(&GPIO_LED_GREEN, 1);
	GPIO_write(&GPIO_LED_BLUE, 1);
	GPIO_configure(&GPIO_LED_RED, GPIO_MODE_OUTPUT, GPIO_TYPE_PUSH_PULL, GPIO_SPEED_LOW, GPIO_PULL_NONE);
	GPIO_configure(&GPIO_LED_GREEN, GPIO_MODE_OUTPUT, GPIO_TYPE_PUSH_PULL, GPIO_SPEED_LOW, GPIO_PULL_NONE);
	GPIO_configure(&GPIO_LED_BLUE, GPIO_MODE_OUTPUT, GPIO_TYPE_PUSH_PULL, GPIO_SPEED_LOW, GPIO_PULL_NONE);
}
#endif

/*** LED functions ***/

#if (defined LVRM) || (defined DDRM) || (defined RRM) || (defined GPSM)
/*******************************************************************/
LED_status_t LED_init(void) {
	// Local variables.
	LED_status_t status = LED_SUCCESS;
#if (defined LVRM) || (defined DDRM) || (defined RRM)
	TIM_status_t tim2_status = TIM_SUCCESS;
	// Init timers.
	tim2_status = TIM2_init();
	TIM2_exit_error(LED_ERROR_BASE_TIM2);
	TIM21_init();
errors:
#endif
	// Turn LED off.
	_LED_off();
	return status;
}
#endif

#if (defined LVRM) || (defined DDRM) || (defined RRM)
/*******************************************************************/
LED_status_t LED_start_single_blink(uint32_t blink_duration_ms, LED_color_t color) {
	// Local variables.
	LED_status_t status = LED_SUCCESS;
	TIM_status_t tim21_status = TIM_SUCCESS;
	// Check parameters.
	if (blink_duration_ms == 0) {
		status = LED_ERROR_NULL_DURATION;
		goto errors;
	}
	if (color >= LED_COLOR_LAST) {
		status = LED_ERROR_COLOR;
		goto errors;
	}
	// Link GPIOs to timer.
	GPIO_configure(&GPIO_LED_RED, GPIO_MODE_ALTERNATE_FUNCTION, GPIO_TYPE_PUSH_PULL, GPIO_SPEED_LOW, GPIO_PULL_NONE);
	GPIO_configure(&GPIO_LED_GREEN, GPIO_MODE_ALTERNATE_FUNCTION, GPIO_TYPE_PUSH_PULL, GPIO_SPEED_LOW, GPIO_PULL_NONE);
	GPIO_configure(&GPIO_LED_BLUE, GPIO_MODE_ALTERNATE_FUNCTION, GPIO_TYPE_PUSH_PULL, GPIO_SPEED_LOW, GPIO_PULL_NONE);
	// Start blink.
	TIM2_start((TIM2_channel_mask_t) color);
	tim21_status = TIM21_start(blink_duration_ms);
	TIM21_exit_error(LED_ERROR_BASE_TIM21);
errors:
	return status;
}
#endif

#if (defined LVRM) || (defined DDRM) || (defined RRM)
/*******************************************************************/
uint8_t LED_is_single_blink_done(void) {
	return TIM21_is_single_blink_done();
}
#endif

#if (defined LVRM) || (defined DDRM) || (defined RRM)
/*******************************************************************/
void LED_stop_blink(void) {
	// Stop timers.
	TIM2_stop();
	TIM21_stop();
	// Turn LED off.
	_LED_off();
}
#endif

#ifdef GPSM
/*******************************************************************/
LED_status_t LED_set(LED_color_t color) {
	// Local variables.
	LED_status_t status = LED_SUCCESS;
	// Check color.
	switch (color) {
	case LED_COLOR_OFF:
		GPIO_write(&GPIO_LED_RED, 1);
		GPIO_write(&GPIO_LED_GREEN, 1);
		GPIO_write(&GPIO_LED_BLUE, 1);
		break;
	case LED_COLOR_RED:
		GPIO_write(&GPIO_LED_RED, 0);
		GPIO_write(&GPIO_LED_GREEN, 1);
		GPIO_write(&GPIO_LED_BLUE, 1);
		break;
	case LED_COLOR_GREEN:
		GPIO_write(&GPIO_LED_RED, 1);
		GPIO_write(&GPIO_LED_GREEN, 0);
		GPIO_write(&GPIO_LED_BLUE, 1);
		break;
	case LED_COLOR_YELLOW:
		GPIO_write(&GPIO_LED_RED, 0);
		GPIO_write(&GPIO_LED_GREEN, 0);
		GPIO_write(&GPIO_LED_BLUE, 1);
		break;
	case LED_COLOR_BLUE:
		GPIO_write(&GPIO_LED_RED, 1);
		GPIO_write(&GPIO_LED_GREEN, 1);
		GPIO_write(&GPIO_LED_BLUE, 0);
		break;
	case LED_COLOR_MAGENTA:
		GPIO_write(&GPIO_LED_RED, 0);
		GPIO_write(&GPIO_LED_GREEN, 1);
		GPIO_write(&GPIO_LED_BLUE, 0);
		break;
	case LED_COLOR_CYAN:
		GPIO_write(&GPIO_LED_RED, 1);
		GPIO_write(&GPIO_LED_GREEN, 0);
		GPIO_write(&GPIO_LED_BLUE, 0);
		break;
	case LED_COLOR_WHITE:
		GPIO_write(&GPIO_LED_RED, 0);
		GPIO_write(&GPIO_LED_GREEN, 0);
		GPIO_write(&GPIO_LED_BLUE, 0);
		break;
	default:
		status = LED_ERROR_COLOR;
		break;
	}
	return status;
}
#endif

#ifdef GPSM
/*******************************************************************/
LED_status_t LED_toggle(LED_color_t color) {
	// Local variables.
	LED_status_t status = LED_SUCCESS;
	// Check color.
	switch (color) {
	case LED_COLOR_OFF:
		// Nothing to do.
		break;
	case LED_COLOR_RED:
		GPIO_toggle(&GPIO_LED_RED);
		break;
	case LED_COLOR_GREEN:
		GPIO_toggle(&GPIO_LED_GREEN);
		break;
	case LED_COLOR_YELLOW:
		GPIO_toggle(&GPIO_LED_RED);
		GPIO_toggle(&GPIO_LED_GREEN);
		break;
	case LED_COLOR_BLUE:
		GPIO_toggle(&GPIO_LED_BLUE);
		break;
	case LED_COLOR_MAGENTA:
		GPIO_toggle(&GPIO_LED_RED);
		GPIO_toggle(&GPIO_LED_BLUE);
		break;
	case LED_COLOR_CYAN:
		GPIO_toggle(&GPIO_LED_GREEN);
		GPIO_toggle(&GPIO_LED_BLUE);
		break;
	case LED_COLOR_WHITE:
		GPIO_toggle(&GPIO_LED_RED);
		GPIO_toggle(&GPIO_LED_GREEN);
		GPIO_toggle(&GPIO_LED_BLUE);
		break;
	default:
		status = LED_ERROR_COLOR;
		break;
	}
	return status;
}
#endif
