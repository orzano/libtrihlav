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

// #endregion
