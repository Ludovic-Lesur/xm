/*
 * tim.c
 *
 *  Created on: 22 aug. 2020
 *      Author: Ludo
 */

#include "tim.h"

#include "iwdg.h"
#include "mapping.h"
#include "nvic.h"
#include "pwr.h"
#include "rcc.h"
#include "rcc_reg.h"
#include "tim_reg.h"

/*** TIM local macros ***/

#define TIM_TIMEOUT_COUNT				1000000

#define TIM2_CNT_VALUE_MAX				0xFFFF
#define TIM2_ETRF_PRESCALER				16
#define TIM2_ETRF_CLOCK_HZ				(RCC_LSE_FREQUENCY_HZ / TIM2_ETRF_PRESCALER)

#define TIM2_CLOCK_SWITCH_LATENCY_MS	2

#define TIM2_TIMER_DURATION_MS_MIN		1
#define TIM2_TIMER_DURATION_MS_MAX		((TIM2_CNT_VALUE_MAX * 1000) / (TIM2_ETRF_CLOCK_HZ))

#define TIM2_NUMBER_OF_CHANNELS			4

#define TIM2_CCRX_MASK_OFF				0xFFFF
#define TIM2_PWM_FREQUENCY_HZ			10000
#define TIM2_ARR_VALUE					((RCC_HSI_FREQUENCY_KHZ * 1000) / (TIM2_PWM_FREQUENCY_HZ))

#define TIM21_PRESCALER					8
#define TIM21_DIMMING_LUT_LENGTH		100

/*** TIM local structures ***/

#if (defined LVRM) || (defined DDRM) || (defined RRM)
/*******************************************************************/
typedef struct {
	volatile uint32_t dimming_lut_idx;
	volatile uint8_t dimming_lut_direction;
	volatile uint8_t single_blink_done;
} TIM21_context_t;
#endif

/*** TIM local global variables ***/

#ifdef UHFM
static volatile uint8_t tim2_channel_running[TIM2_CHANNEL_LAST];
#endif
#if (defined LVRM) || (defined DDRM) || (defined RRM)
static const uint16_t TIM21_DIMMING_LUT[TIM21_DIMMING_LUT_LENGTH] = {
	1601, 1601, 1601, 1601, 1601, 1601, 1600, 1600, 1600, 1600,
	1600, 1600, 1600, 1599, 1599, 1599, 1599, 1598, 1598, 1598,
	1598, 1597, 1597, 1596, 1596, 1596, 1595, 1595, 1594, 1593,
	1593, 1592, 1591, 1590, 1589, 1588, 1587, 1586, 1585, 1584,
	1582, 1581, 1579, 1577, 1575, 1573, 1571, 1569, 1566, 1563,
	1560, 1557, 1554, 1550, 1546, 1542, 1537, 1532, 1527, 1521,
	1514, 1508, 1500, 1493, 1484, 1475, 1465, 1454, 1443, 1431,
	1418, 1403, 1388, 1371, 1353, 1334, 1314, 1291, 1267, 1241,
	1213, 1183, 1151, 1116, 1078, 1038, 994, 947, 896, 842,
	783, 720, 651, 578, 498, 413, 321, 222, 115, 0,
};
static uint16_t tim2_ccrx_mask[TIM2_NUMBER_OF_CHANNELS];
static TIM21_context_t tim21_ctx;
#endif

/*** TIM local functions ***/

#ifdef UHFM
/*******************************************************************/
void __attribute__((optimize("-O0"))) TIM2_IRQHandler(void) {
	// Local variables.
	uint8_t channel_idx = 0;
	// Channels loop.
	for (channel_idx=0 ; channel_idx<TIM2_CHANNEL_LAST ; channel_idx++) {
		// Check flag.
		if (((TIM2 -> SR) & (0b1 << (channel_idx + 1))) != 0) {
			// Reset flag.
			tim2_channel_running[channel_idx] = 0;
			// Clear flag.
			TIM2 -> SR &= ~(0b1 << (channel_idx + 1));
		}
	}
}
#endif

#if (defined LVRM) || (defined DDRM) || (defined RRM)
/*******************************************************************/
void __attribute__((optimize("-O0"))) TIM21_IRQHandler(void) {
	// Check update flag.
	if (((TIM21 -> SR) & (0b1 << 0)) != 0) {
		// Update duty cycles.
		TIM2 -> CCR1 = (TIM21_DIMMING_LUT[tim21_ctx.dimming_lut_idx] | tim2_ccrx_mask[0]);
		TIM2 -> CCR2 = (TIM21_DIMMING_LUT[tim21_ctx.dimming_lut_idx] | tim2_ccrx_mask[1]);
		TIM2 -> CCR3 = (TIM21_DIMMING_LUT[tim21_ctx.dimming_lut_idx] | tim2_ccrx_mask[2]);
		// Manage index and direction.
		if (tim21_ctx.dimming_lut_direction == 0) {
			// Increment index.
			tim21_ctx.dimming_lut_idx++;
			// Invert direction at end of table.
			if (tim21_ctx.dimming_lut_idx >= (TIM21_DIMMING_LUT_LENGTH - 1)) {
				tim21_ctx.dimming_lut_direction = 1;
			}
		}
		else {
			// Decrement index.
			tim21_ctx.dimming_lut_idx--;
			// Invert direction at the beginning of table.
			if (tim21_ctx.dimming_lut_idx == 0) {
				// Single blink done.
				TIM2_stop();
				TIM21_stop();
				tim21_ctx.dimming_lut_direction = 0;
				tim21_ctx.single_blink_done = 1;
			}
		}
		// Clear flag.
		TIM21 -> SR &= ~(0b1 << 0);
	}
}
#endif

#if (defined LVRM) || (defined DDRM) || (defined RRM)
/*******************************************************************/
void _TIM2_reset_channels(void) {
	// Local variables.
	uint8_t idx = 0;
	// Reset masks.
	for (idx=0 ; idx<TIM2_NUMBER_OF_CHANNELS ; idx++) tim2_ccrx_mask[idx] = TIM2_CCRX_MASK_OFF;
	// Disable all channels.
	TIM2 -> CCR1 = (TIM2_ARR_VALUE + 1);
	TIM2 -> CCR2 = (TIM2_ARR_VALUE + 1);
	TIM2 -> CCR3 = (TIM2_ARR_VALUE + 1);
	// Reset counter.
	TIM2 -> CNT = 0;
}
#endif

/*** TIM functions ***/

#ifdef UHFM
/*******************************************************************/
void TIM2_init(void) {
	// Local variables.
	uint8_t channel_idx = 0;
	// Init context.
	for (channel_idx=0 ; channel_idx<TIM2_CHANNEL_LAST ; channel_idx++) {
		tim2_channel_running[channel_idx] = 0;
	}
	// Enable peripheral clock.
	RCC -> APB1ENR |= (0b1 << 0); // TIM2EN='1'.
	RCC -> APB1SMENR |= (0b1 << 0); // TIM2SMEN='1'.
	// Use LSE/16 = 2048Hz as trigger (external clock mode 2).
	TIM2 -> PSC = (TIM2_ETRF_PRESCALER - 1);
	TIM2 -> SMCR |= (0b1 << 14) | (0b111 << 4);
	TIM2 -> OR |= (0b101 << 0);
	// Configure channels 1-4 in output compare mode.
	TIM2 -> CCMR1 &= 0xFFFF0000;
	TIM2 -> CCMR2 &= 0xFFFF0000;
	TIM2 -> CCER &= 0xFFFF0000;
	// Generate event to update registers.
	TIM2 -> EGR |= (0b1 << 0); // UG='1'.
	// Enable interrupt.
	NVIC_enable_interrupt(NVIC_INTERRUPT_TIM2, NVIC_PRIORITY_TIM2);
}
#endif

#ifdef UHFM
/*******************************************************************/
void TIM2_de_init(void) {
	// Disable interrupt.
	NVIC_disable_interrupt(NVIC_INTERRUPT_TIM2);
	// Disable peripheral clock.
	RCC -> APB1ENR &= ~(0b1 << 0); // TIM2EN='0'.
}
#endif

#ifdef UHFM
/*******************************************************************/
TIM_status_t TIM2_start(TIM2_channel_t channel, uint32_t duration_ms, TIM_waiting_mode_t waiting_mode) {
	// Local variables.
	TIM_status_t status = TIM_SUCCESS;
	uint32_t compare_value = 0;
	uint32_t local_duration_ms = duration_ms;
	uint32_t duration_min_ms = TIM2_TIMER_DURATION_MS_MIN;
	// Check parameters.
	if (waiting_mode >= TIM_WAITING_MODE_LAST) {
		status = TIM_ERROR_WAITING_MODE;
		goto errors;
	}
	// Check waiting mode.
	if (waiting_mode == TIM_WAITING_MODE_LOW_POWER_SLEEP) {
		// Compensate clock switch latency.
		duration_min_ms += TIM2_CLOCK_SWITCH_LATENCY_MS;
	}
	// Check parameters.
	if (channel >= TIM2_CHANNEL_LAST) {
		status = TIM_ERROR_CHANNEL;
		goto errors;
	}
	if (duration_ms < duration_min_ms) {
		status = TIM_ERROR_DURATION_UNDERFLOW;
		goto errors;
	}
	if (duration_ms > TIM2_TIMER_DURATION_MS_MAX) {
		status = TIM_ERROR_DURATION_OVERFLOW;
		goto errors;
	}
	// Compute compare value.
	if (waiting_mode == TIM_WAITING_MODE_LOW_POWER_SLEEP) {
		local_duration_ms -= TIM2_CLOCK_SWITCH_LATENCY_MS;
	}
	compare_value = ((TIM2 -> CNT) + ((local_duration_ms * TIM2_ETRF_CLOCK_HZ) / (1000))) % TIM2_CNT_VALUE_MAX;
	TIM2 -> CCRx[channel] = compare_value;
	// Update flag.
	tim2_channel_running[channel] = 1;
	// Clear flag.
	TIM2 -> SR &= ~(0b1 << (channel + 1));
	// Enable channel.
	TIM2 -> DIER |= (0b1 << (channel + 1));
	TIM2 -> CCER |= (0b1 << (4 * channel));
	// Enable counter.
	TIM2 -> CR1 |= (0b1 << 0);
errors:
	return status;
}
#endif

#ifdef UHFM
/*******************************************************************/
TIM_status_t TIM2_stop(TIM2_channel_t channel) {
	// Local variables.
	TIM_status_t status = TIM_SUCCESS;
	uint8_t channel_idx = 0;
	uint8_t running_count = 0;
	// Check parameter.
	if (channel >= TIM2_CHANNEL_LAST) {
		status = TIM_ERROR_CHANNEL;
		goto errors;
	}
	// Disable channel.
	TIM2 -> CCER &= ~(0b1 << (4 * channel));
	TIM2 -> DIER &= ~(0b1 << (channel + 1));
	// Clear flag.
	TIM2 -> SR &= ~(0b1 << (channel + 1));
	// Update flag.
	tim2_channel_running[channel] = 0;
	// Disable counter if all channels are stopped.
	for (channel_idx=0 ; channel_idx<TIM2_CHANNEL_LAST ; channel_idx++) {
		running_count += tim2_channel_running[channel_idx];
	}
	if (running_count == 0) {
		TIM2 -> CR1 &= ~(0b1 << 0);
	}
errors:
	return status;
}
#endif

#ifdef UHFM
/*******************************************************************/
TIM_status_t TIM2_get_status(TIM2_channel_t channel, uint8_t* timer_has_elapsed) {
	// Local variables.
	TIM_status_t status = TIM_SUCCESS;
	// Check parameters.
	if (channel >= TIM2_CHANNEL_LAST) {
		status = TIM_ERROR_CHANNEL;
		goto errors;
	}
	if (timer_has_elapsed == NULL) {
		status = TIM_ERROR_NULL_PARAMETER;
		goto errors;
	}
	// Update flag.
	(*timer_has_elapsed) = (tim2_channel_running[channel] == 0) ? 1 : 0;
errors:
	return status;
}
#endif

#ifdef UHFM
/*******************************************************************/
TIM_status_t TIM2_wait_completion(TIM2_channel_t channel, TIM_waiting_mode_t waiting_mode) {
	// Local variables.
	TIM_status_t status = TIM_SUCCESS;
	RCC_status_t rcc_status = RCC_SUCCESS;
	// Check parameters.
	if (channel >= TIM2_CHANNEL_LAST) {
		status = TIM_ERROR_CHANNEL;
		goto errors;
	}
	// Sleep until channel is not running.
	switch (waiting_mode) {
	case TIM_WAITING_MODE_ACTIVE:
		// Active loop.
		while (tim2_channel_running[channel] != 0) {
			IWDG_reload();
		}
		break;
	case TIM_WAITING_MODE_SLEEP:
		// Enter sleep mode.
		while (tim2_channel_running[channel] != 0) {
			PWR_enter_sleep_mode();
			IWDG_reload();
		}
		break;
	case TIM_WAITING_MODE_LOW_POWER_SLEEP:
		// Switch to MSI.
		rcc_status = RCC_switch_to_msi(RCC_MSI_RANGE_1_131KHZ);
		RCC_check_status(TIM_ERROR_BASE_RCC);
		// Enter low power sleep mode.
		while (tim2_channel_running[channel] != 0) {
			PWR_enter_low_power_sleep_mode();
			IWDG_reload();
		}
		// Go back to HSI.
		rcc_status = RCC_switch_to_hsi();
		RCC_check_status(TIM_ERROR_BASE_RCC);
		break;
	default:
		status = TIM_ERROR_WAITING_MODE;
		goto errors;
	}
errors:
	return status;
}
#endif

#if (defined LVRM) || (defined DDRM) || (defined RRM)
/*******************************************************************/
void TIM2_init(void) {
	// Enable peripheral clock.
	RCC -> APB1ENR |= (0b1 << 0); // TIM2EN='1'.
	// Set PWM frequency.
	TIM2 -> ARR = TIM2_ARR_VALUE;
	// Configure channels 1-4 in PWM mode 1 (OCxM='110' and OCxPE='1').
	TIM2 -> CCMR1 |= (0b110 << 12) | (0b1 << 11) | (0b110 << 4) | (0b1 << 3);
	TIM2 -> CCMR2 |= (0b110 << 12) | (0b1 << 11) | (0b110 << 4) | (0b1 << 3);
	TIM2 -> CR1 |= (0b1 << 7);
	TIM2 -> CCER |= 0x00000111;
	// Disable all channels by default.
	_TIM2_reset_channels();
	// Generate event to update registers.
	TIM2 -> EGR |= (0b1 << 0); // UG='1'.
}
#endif

#if (defined LVRM) || (defined DDRM) || (defined RRM)
/*******************************************************************/
void TIM2_start(TIM2_channel_mask_t led_color) {
	// Local variables.
	uint8_t idx = 0;
	// Disable all channels.
	_TIM2_reset_channels();
	// Link GPIOs to timer.
	GPIO_configure(&GPIO_LED_RED, GPIO_MODE_ALTERNATE_FUNCTION, GPIO_TYPE_PUSH_PULL, GPIO_SPEED_LOW, GPIO_PULL_NONE);
	GPIO_configure(&GPIO_LED_GREEN, GPIO_MODE_ALTERNATE_FUNCTION, GPIO_TYPE_PUSH_PULL, GPIO_SPEED_LOW, GPIO_PULL_NONE);
	GPIO_configure(&GPIO_LED_BLUE, GPIO_MODE_ALTERNATE_FUNCTION, GPIO_TYPE_PUSH_PULL, GPIO_SPEED_LOW, GPIO_PULL_NONE);
	// Enable required channels.
	for (idx=0 ; idx<TIM2_NUMBER_OF_CHANNELS ; idx++) {
		if ((led_color & (0b1 << idx)) != 0) {
			tim2_ccrx_mask[idx] = 0;
		}
	}
	// Enable counter.
	TIM2 -> CR1 |= (0b1 << 0); // CEN='1'.
}
#endif

#if (defined LVRM) || (defined DDRM) || (defined RRM)
/*******************************************************************/
void TIM2_stop(void) {
	// Disable all channels.
	_TIM2_reset_channels();
	// Disable and reset counter.
	TIM2 -> CR1 &= ~(0b1 << 0); // CEN='0'.
}
#endif

#if (defined LVRM) || (defined DDRM) || (defined RRM)
/*******************************************************************/
void TIM21_init(void) {
	// Init context.
	tim21_ctx.dimming_lut_idx = 0;
	tim21_ctx.dimming_lut_direction = 0;
	tim21_ctx.single_blink_done = 1;
	// Enable peripheral clock.
	RCC -> APB2ENR |= (0b1 << 2); // TIM21EN='1'.
	// Configure period.
	TIM21 -> PSC = (TIM21_PRESCALER - 1); // Timer is clocked on (SYSCLK / 2) .
	// Generate event to update registers.
	TIM21 -> EGR |= (0b1 << 0); // UG='1'.
	// Enable interrupt.
	TIM21 -> DIER |= (0b1 << 0);
}
#endif

#if (defined LVRM) || (defined DDRM) || (defined RRM)
/*******************************************************************/
void TIM21_start(uint32_t led_blink_period_ms) {
	// Reset LUT index and flag.
	tim21_ctx.dimming_lut_idx = 0;
	tim21_ctx.dimming_lut_direction = 0;
	tim21_ctx.single_blink_done = 0;
	// Set period.
	TIM21 -> CNT = 0;
	TIM21 -> ARR = (led_blink_period_ms * RCC_HSI_FREQUENCY_KHZ) / (TIM21_PRESCALER * 2 * TIM21_DIMMING_LUT_LENGTH);
	// Clear flag and enable interrupt.
	TIM21 -> SR &= ~(0b1 << 0); // Clear flag (UIF='0').
	NVIC_enable_interrupt(NVIC_INTERRUPT_TIM21, NVIC_PRIORITY_TIM21);
	// Enable TIM21 peripheral.
	TIM21 -> CR1 |= (0b1 << 0); // Enable TIM21 (CEN='1').
}
#endif

#if (defined LVRM) || (defined DDRM) || (defined RRM)
/*******************************************************************/
void TIM21_stop(void) {
	// Disable interrupt.
	NVIC_disable_interrupt(NVIC_INTERRUPT_TIM21);
	// Stop TIM21.
	TIM21 -> CR1 &= ~(0b1 << 0); // CEN='0'.
}
#endif

#if (defined LVRM) || (defined DDRM) || (defined RRM)
/*******************************************************************/
uint8_t TIM21_is_single_blink_done(void) {
	return (tim21_ctx.single_blink_done);
}
#endif
