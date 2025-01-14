/*
 * @brief Application timers
 * @copyright Copyright © 2022-2024 Trihlav, s.r.o.
 * @license MIT License / see LICENSE file
 */

// #region Includes

#include "trh_timer.h"

#include <string.h>
#include <errno.h>
#include <sys/timerfd.h>

// #region Local types

// #region Exported functions

static int local_timer_init( TTrhTimerProperties *iProperties, TTrhEvent *iEvent )
{
	struct itimerspec lSpec = {
		.it_interval = { .tv_sec = 0, .tv_nsec = 0 },
		.it_value = {
			.tv_sec = iProperties->sec,
			.tv_nsec = iProperties->nsec
		}
	};

	// Create timer fd
	iEvent->fd = timerfd_create( CLOCK_MONOTONIC, TFD_NONBLOCK );
	if( iEvent->fd == -1 ) {
		trh_log( LOG_ERROR, "Failed to create timer. Error: %s\n", strerror( errno ) );
		return TRH_TIMER_FAILED;
	}

	// Create timer properties - clone timer object in memory
	iEvent->ext.timer = (TTrhTimerProperties*)malloc( sizeof( TTrhTimerProperties ) );
	if( iEvent->ext.timer == 0 ) return TRH_OUT_OF_MEM;
	memcpy( iEvent->ext.timer, iProperties, sizeof( TTrhTimerProperties ) );

	// Set timer properties
	if( timerfd_settime( iEvent->fd, 0, &lSpec, 0 ) == -1 ) {
		trh_log( LOG_ERROR, "Failed to set timer properties. Error: %s\n", strerror( errno ) );
		return TRH_TIMER_FAILED;
	}

	return TRH_OK;
}

static int local_timer_event( TTrhEvent *iEvent )
{
	if( iEvent == 0 ) return TRH_ARG_INVALID;
	if( iEvent->ext.timer == 0 ) return TRH_ARG_INVALID;

	// Call timer event handler
	iEvent->ext.timer->handle_timer_event( iEvent );

	// If timer is not repeating, unregister it
	if( iEvent->ext.timer->repeat == false ) {
		trh_timer_release( iEvent );
		return TRH_END;
	}

	// Renew the timer if it is repeating

	struct itimerspec lSpec = {
		.it_interval = { .tv_sec = 0, .tv_nsec = 0 },
		.it_value = {
			.tv_sec = iEvent->ext.timer->sec,
			.tv_nsec = iEvent->ext.timer->nsec
		}
	};

	if( timerfd_settime( iEvent->fd, 0, &lSpec, 0 ) == -1 ) {
		trh_log( LOG_ERROR, "Failed to reset timer: %s", strerror( errno ) );
		trh_timer_release( iEvent );
		return TRH_TIMER_FAILED;
	}

	return TRH_OK;
}

static int local_timer_error( TTrhEvent *iEvent )
{
	trh_log( LOG_ERROR, "Timer error\n" );
	return TRH_TIMER_FAILED;
}

int trh_timer_init( TTrhTimerProperties *iProperties, TTrhEvent **oEvent )
{
	TTrhEvent *lEvent = 0;
	int lCode = TRH_OK;

	// Validate input arguments
	if( iProperties == 0 ) {
		trh_log( LOG_ERROR, "Invalid argument: timer roperties\n" );
		return TRH_ARG_INVALID;
	}
	
	if( oEvent == 0 ) {
		trh_log( LOG_ERROR, "Invalid argument: timer object is null\n" );
		return TRH_ARG_INVALID;
	}

	// Allocate memory for timer object
	lEvent = (TTrhEvent*)malloc( sizeof( TTrhEvent ) );
	if( lEvent == 0 ) return TRH_OUT_OF_MEM;
	bzero( lEvent, sizeof( TTrhEvent ) );
	lEvent->fd = -1;
	lEvent->handle_event = local_timer_event;
	lEvent->handle_error = local_timer_error;

	// Create timer
	if( ( lCode = local_timer_init( iProperties, lEvent ) ) != TRH_OK ) {
		trh_timer_release( lEvent );
		return lCode;
	}

	// Return timer object
	*oEvent = lEvent;

	return TRH_OK;
}

/**
 * @brief Unregister timer from epoll and release it.
 */
void trh_timer_release( TTrhEvent *iEvent )
{
	if( iEvent == 0 )
		return;

	// Unregister timer from epoll
	trh_event_unregister( iEvent );

	// Close timer socket
	CLOSE_FD( iEvent->fd );

	// Free timer object
	FREE_PTR( iEvent->ext.timer );

	// Free memory
	free( iEvent );
}

// #endregion