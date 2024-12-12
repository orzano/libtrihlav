/*
 * @brief Essential includes and generic defines
 * @copyright Copyright © 2022-2024 Trihlav, s.r.o.
 * @license MIT License / see LICENSE file
 */

// #region Includes

#include <assert.h>

#include "trihlav.h"

// #endregion

// #region Static variables

// Pointer to application object
static TApplication *geApplication = 0;

// #endregion


// #region Exported functions

// Initialize application object.
TApplication *trh_init( TApplication *iApp, void *iExt )
{
	assert( iApp != 0 );
	if( iApp == 0 ) return 0;

	geApplication = iApp;

	iApp->time_system = trh_time();

	iApp->ext = iExt;
}

// Return pointer to application object.
TApplication *trh_get_app()
{
	return geApplication;
}

// #endregion