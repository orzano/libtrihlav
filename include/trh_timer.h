/*
 * @brief Application timers
 * @copyright Copyright © 2022-2024 Trihlav, s.r.o.
 * @license MIT License / see LICENSE file
 */

#ifndef TRH_TIMER_H
#define TRH_TIMER_H

#include "trihlav.h"

struct TTrhTimer;

typedef struct TTrhTimerProperties {
	time_t sec;
	time_t nsec;
	bool repeat;
	void *ext;

	handle_event handle_timer_event;
} TTrhTimerProperties;


/**
 * @brief Create a new timer.
 * @retval TRH_INVALID_ARG iProperties or oEvent is null.
 * @retval TRH_OK
 *
 * - allocate memory for timer event
 * - initialize timer 
 * - register timer with epoll
 * 
 * If iProperties has been allocated in memory, it can be released after this function.
 * oEvent must be released with trh_timer_release().
 */
int trh_timer_init( TTrhTimerProperties *iProperties, TTrhEvent **oEvent );

/**
 * @brief Release timer resources.
 * 
 * - unregister timer from epoll
 * - close timer
 * - free memory
 */
void trh_timer_release( TTrhEvent *iEvent );

#endif // TRH_DBUS_H
