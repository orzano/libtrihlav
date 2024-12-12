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

void trh_version( uint8_t *oMajor, uint8_t *oMinor, uint8_t *oPatch, uint32_t *oVer )
{
	if( oMajor != 0 ) *oMajor = APP_VERSION_MAJOR;
	if( oMinor != 0 ) *oMinor = APP_VERSION_MINOR;
	if( oPatch != 0 ) *oPatch = APP_VERSION_PATCH;

	// Convert major, minor and patch to version number using shift operands
	if( oVer != 0 ) *oVer =
		( APP_VERSION_MAJOR << 16 ) |
		( APP_VERSION_MINOR << 8 ) |
		( APP_VERSION_PATCH );
}

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