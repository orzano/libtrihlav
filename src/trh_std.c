/*
 * @brief Essential includes and generic defines
 * @copyright Copyright © 2022-2024 Trihlav, s.r.o.
 * @license MIT License / see LICENSE file
 */

// #region Includes

#include <time.h>

#include "trihlav.h"

// #endregion


// #region Exported functions

// Get system time
double trh_time()
{
	struct timespec lTime;

	clock_gettime( CLOCK_REALTIME, &lTime );

	return (double)( lTime.tv_sec ) + (double)( lTime.tv_nsec ) / 1000000000.0;
}

// Set flag 'application is now terminating'.
void trh_terminate()
{
	TApplication *lApp = trh_app();
	pthread_mutex_lock( &lApp->mutex );
	lApp->terminate = true;
	pthread_mutex_unlock( &trh_app()->mutex );
}

// Return 'true' if application is terminating.
bool trh_is_terminating()
{
	bool lResult = false;
	TApplication *lApp = trh_app();
	pthread_mutex_lock( &lApp->mutex );
	lResult = lApp->terminate;
	pthread_mutex_unlock( &lApp->mutex );
	return lResult;
}

// #endregion
