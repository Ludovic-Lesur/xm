/*
 * tim.h
 *
 *  Created on: 22 aug. 2020
 *      Author: Ludo
 */

#ifndef __TIM_H__
#define __TIM_H__

#include "types.h"

/*** TIM structures ***/

/*!******************************************************************
 * \enum TIM_status_t
 * \brief TIM driver error codes.
 *******************************************************************/
typedef enum {
	TIM_SUCCESS = 0,
	TIM_ERROR_NULL_PARAMETER,
	TIM_ERROR_CHANNEL,
	TIM_ERROR_DURATION_OVERFLOW,
	TIM_ERROR_BASE_LAST = 0x0100
} TIM_status_t;

#ifdef UHFM
/*!******************************************************************
 * \enum TIM2_channel_t
 * \brief TIM channels list.
 *******************************************************************/
typedef enum {
	TIM2_CHANNEL_1 = 0,
	TIM2_CHANNEL_2,
	TIM2_CHANNEL_3,
	TIM2_CHANNEL_4,
	TIM2_CHANNEL_LAST
} TIM2_channel_t;
#endif

#if ((defined LVRM) && (defined HW1_0))
/*!******************************************************************
 * \enum TIM2_channel_mask_t
 * \brief LED color bit mask defined as 0b<CH4><CH3><CH2><CH1>.
 *******************************************************************/
typedef enum {
	TIM2_CHANNEL_MASK_OFF = 0b0000,
	TIM2_CHANNEL_MASK_RED = 0b0100,
	TIM2_CHANNEL_MASK_GREEN = 0b0010,
	TIM2_CHANNEL_MASK_YELLOW = 0b0110,
	TIM2_CHANNEL_MASK_BLUE = 0b0001,
	TIM2_CHANNEL_MASK_MAGENTA = 0b0101,
	TIM2_CHANNEL_MASK_CYAN = 0b0011,
	TIM2_CHANNEL_MASK_WHITE	= 0b0111
} TIM2_channel_mask_t;
#endif
#ifdef DDRM
/*!******************************************************************
 * \enum TIM2_channel_mask_t
 * \brief LED color bit mask defined as 0b<CH4><CH3><CH2><CH1>.
 *******************************************************************/
typedef enum {
	TIM2_CHANNEL_MASK_OFF = 0b0000,
	TIM2_CHANNEL_MASK_RED = 0b0100,
	TIM2_CHANNEL_MASK_GREEN = 0b0001,
	TIM2_CHANNEL_MASK_YELLOW = 0b0101,
	TIM2_CHANNEL_MASK_BLUE = 0b0010,
	TIM2_CHANNEL_MASK_MAGENTA = 0b0110,
	TIM2_CHANNEL_MASK_CYAN = 0b0011,
	TIM2_CHANNEL_MASK_WHITE	= 0b0111
} TIM2_channel_mask_t;
#endif
#if ((defined LVRM) && (defined HW2_0)) || (defined RRM)
/*!******************************************************************
 * \enum TIM2_channel_mask_t
 * \brief LED color bit mask defined as 0b<CH4><CH3><CH2><CH1>.
 *******************************************************************/
typedef enum {
	TIM2_CHANNEL_MASK_OFF = 0b0000,
	TIM2_CHANNEL_MASK_RED = 0b010,
	TIM2_CHANNEL_MASK_GREEN = 0b0100,
	TIM2_CHANNEL_MASK_YELLOW = 0b0110,
	TIM2_CHANNEL_MASK_BLUE = 0b0001,
	TIM2_CHANNEL_MASK_MAGENTA = 0b0011,
	TIM2_CHANNEL_MASK_CYAN = 0b0101,
	TIM2_CHANNEL_MASK_WHITE	= 0b0111
} TIM2_channel_mask_t;
#endif

/*** TIM functions ***/

#ifdef UHFM
/*!******************************************************************
 * \fn void TIM2_init(void)
 * \brief Init TIM2 peripheral for Sigfox library MCU API timers.
 * \param[in]  	none
 * \param[out] 	none
 * \retval		none
 *******************************************************************/
void TIM2_init(void);
#endif

#ifdef UHFM
/*!******************************************************************
 * \fn TIM_status_t TIM2_start(TIM2_channel_t channel, uint32_t duration_ms)
 * \brief Start a timer channel.
 * \param[in]  	channel: Channel to start.
 * \param[in]	duration_ms: Timer duration in ms.
 * \param[out] 	none
 * \retval		Function execution status.
 *******************************************************************/
TIM_status_t TIM2_start(TIM2_channel_t channel, uint32_t duration_ms);
#endif

#ifdef UHFM
/*!******************************************************************
 * \fn TIM_status_t TIM2_stop(TIM2_channel_t channel)
 * \brief Stop a timer channel.
 * \param[in]  	channel: Channel to stop.
 * \param[out] 	none
 * \retval		Function execution status.
 *******************************************************************/
TIM_status_t TIM2_stop(TIM2_channel_t channel);
#endif

#ifdef UHFM
/*!******************************************************************
 * \fn TIM_status_t TIM2_get_status(TIM2_channel_t channel, uint8_t* timer_has_elapsed)
 * \brief Get the status of a timer channel.
 * \param[in]  	channel: Channel to read.
 * \param[out]	timer_has_elapsed: Pointer to bit that will contain the timer status (0 for running, 1 for complete).
 * \retval		Function execution status.
 *******************************************************************/
TIM_status_t TIM2_get_status(TIM2_channel_t channel, uint8_t* timer_has_elapsed);
#endif

#ifdef UHFM
/*!******************************************************************
 * \fn TIM_status_t TIM2_wait_completion(TIM2_channel_t channel)
 * \brief Blocking function waiting for a timer channel completion.
 * \param[in]  	channel: Channel to wait for.
 * \param[out] 	none
 * \retval		Function execution status.
 *******************************************************************/
TIM_status_t TIM2_wait_completion(TIM2_channel_t channel);
#endif

#if (defined LVRM) || (defined DDRM) || (defined RRM)
/*!******************************************************************
 * \fn void TIM2_init(void)
 * \brief Init TIM2 peripheral for RGB LED blinking operation.
 * \param[in]  	none
 * \param[out] 	none
 * \retval		none
 *******************************************************************/
void TIM2_init(void);
#endif

#if (defined LVRM) || (defined DDRM) || (defined RRM)
/*!******************************************************************
 * \fn void TIM2_start(TIM2_channel_mask_t led_color)
 * \brief Start PWM signal for a given color.
 * \param[in]  	led_color: LED color.
 * \param[out] 	none
 * \retval		none
 *******************************************************************/
void TIM2_start(TIM2_channel_mask_t led_color);
#endif

#if (defined LVRM) || (defined DDRM) || (defined RRM)
/*!******************************************************************
 * \fn void TIM2_stop(void)
 * \brief Stop PWM signal.
 * \param[in]  	none
 * \param[out] 	none
 * \retval		none
 *******************************************************************/
void TIM2_stop(void);
#endif

#if (defined LVRM) || (defined DDRM) || (defined RRM)
/*!******************************************************************
 * \fn void TIM21_init(void)
 * \brief Init TIM21 peripheral for RGB LED blinking operation.
 * \param[in]  	none
 * \param[out] 	none
 * \retval		none
 *******************************************************************/
void TIM21_init(void);
#endif

#if (defined LVRM) || (defined DDRM) || (defined RRM)
/*!******************************************************************
 * \fn void TIM21_start(uint32_t led_blink_period_ms)
 * \brief Start LED blink duration timer.
 * \param[in]  	led_blink_period_ms: Blink duration in ms.
 * \param[out] 	none
 * \retval		none
 *******************************************************************/
void TIM21_start(uint32_t led_blink_period_ms);
#endif

#if (defined LVRM) || (defined DDRM) || (defined RRM)
/*!******************************************************************
 * \fn void TIM21_stop(void)
 * \brief Stop LED blink timer.
 * \param[in]  	none
 * \param[out] 	none
 * \retval		none
 *******************************************************************/
void TIM21_stop(void);
#endif

#if (defined LVRM) || (defined DDRM) || (defined RRM)
/*!******************************************************************
 * \fn uint8_t TIM21_is_single_blink_done(void)
 * \brief Get the LED blink status.
 * \param[in]  	none
 * \param[out] 	none
 * \retval		0 if the LED blink id running, 1 if it is complete.
 *******************************************************************/
uint8_t TIM21_is_single_blink_done(void);
#endif

/*******************************************************************/
#define TIM2_check_status(error_base) { if (tim2_status != TIM_SUCCESS) { status = error_base + tim2_status; goto errors; } }

/*******************************************************************/
#define TIM2_stack_error() { ERROR_stack_error(tim2_status, TIM_SUCCESS, ERROR_BASE_TIM2); }

/*******************************************************************/
#define TIM2_print_error() { ERROR_print_error(tim2_status, TIM_SUCCESS, ERROR_BASE_TIM2); }

#endif /* __TIM_H__ */
