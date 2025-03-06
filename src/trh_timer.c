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

static int local_timer_init( TTrhTimerProperties *iProperties, TTrhEvent *iEvent );
static int local_timer_set( TTrhEvent *iEvent, time_t iSec, time_t iNsec );
static int local_timer_event( TTrhEvent *iEvent );
static int local_timer_error( TTrhEvent *iEvent );


// #region Exported functions

int trh_timer_init( TTrhTimerProperties *iProperties, TTrhEvent **oEvent )
{
	TTrhEvent *lEvent = 0;
	int lCode = TRH_OK;

	// Validate input arguments
	TRH_ASSERT_ARG( iProperties != 0, "Failed to init timer - invalid setup." );
	TRH_ASSERT_ARG( oEvent != 0, "Failed to init timer - invalid output argument." );

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

	// Register timer with epoll
	if( ( lCode = trh_event_register( lEvent ) ) != TRH_OK ) {
		trh_timer_release( lEvent );
		return lCode;
	}

	// Return timer object
	*oEvent = lEvent;

	return TRH_OK;
}

int trh_timer_start( TTrhEvent *iEvent )
{
	TRH_ASSERT_ARG( iEvent != 0, "Failed to start timer" );
	int lCode = local_timer_set( iEvent, iEvent->ext.timer->sec, iEvent->ext.timer->nsec );
	iEvent->ext.timer->running = lCode == TRH_OK;
	return TRH_OK;
}

void trh_timer_stop( TTrhEvent *iEvent )
{
	assert( iEvent != 0 );
	iEvent->ext.timer->running = false;
	local_timer_set( iEvent, 0, 0 );
}

// Unregister timer from epoll and release it.
void trh_timer_release( TTrhEvent *iEvent )
{
	if( iEvent == 0 )
		return;

	// Unregister timer from epoll
	trh_event_unregister( iEvent );

	// Close timer socket
	CLOSE_FD( iEvent->fd );

	// Free timer object.
	if( iEvent->ext.timer != 0 )
	{
		if( iEvent->ext.timer->handle_timer_stopped )
			iEvent->ext.timer->handle_timer_stopped( iEvent );
	
		FREE_PTR( iEvent->ext.timer );
	}

	// Free memory
	free( iEvent );
}

// #endregion


// #region Local types

int local_timer_init( TTrhTimerProperties *iProperties, TTrhEvent *iEvent )
{
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

	// Timer should be initialized in started state.
	return trh_timer_start( iEvent );
}

int local_timer_set( TTrhEvent *iEvent, time_t iSec, time_t iNsec )
{
	struct itimerspec lSpec = {
		.it_interval = { .tv_sec = 0, .tv_nsec = 0 },
		.it_value = { .tv_sec = iSec, .tv_nsec = iNsec }
	};

	// Set timer properties
	if( timerfd_settime( iEvent->fd, 0, &lSpec, 0 ) == -1 ) {
		trh_log( LOG_ERROR, "Failed to set timer. Error: %s\n", strerror( errno ) );
		return TRH_TIMER_FAILED;
	}

	return TRH_OK;
}

int local_timer_event( TTrhEvent *iEvent )
{
	int lCode = TRH_OK;

	TRH_ASSERT_ARG( iEvent != 0 && iEvent->ext.timer != 0, "Timer event handler received null argument." );

	// Call timer event handler
	if( iEvent->ext.timer->handle_timer_event )
		iEvent->ext.timer->handle_timer_event( iEvent );

	// If timer is not repeating, unregister it
	if( iEvent->ext.timer->repeat == false ) {
		trh_timer_stop( iEvent );
		return TRH_OK;
	}

	// Renew the timer if it is repeating
	else if( ( lCode = trh_timer_start( iEvent ) ) != TRH_OK ) {
		trh_timer_release( iEvent );
		return lCode;
	}
}

int local_timer_error( TTrhEvent *iEvent )
{
	trh_log( LOG_ERROR, "Timer error\n" );
	return TRH_TIMER_FAILED;
}

// #endregion
