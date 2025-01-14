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
	handle_event handle_timer_event;
} TTrhTimerProperties;


/**
 * @brief Create a new timer.
 * @retval TRH_OK
 * 
 * - allocate memory for timer event
 * - initialize timer 
 * - register timer with epoll
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
