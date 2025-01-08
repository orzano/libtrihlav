/*
 * @brief Essential includes and generic defines
 * @copyright Copyright © 2022-2024 Trihlav, s.r.o.
 * @license MIT License / see LICENSE file
 */

// #region Includes

#include "trh_dbus.h"

// #endregion

#define _cleanup_(f) __attribute__((cleanup(f)))


// #region Structures

typedef struct TTrhDbus {
	// Pointer to dbus object.
	sd_bus *ptr;

	// Slot for filtered dbus signals.
	sd_bus_slot *slot;

	chars destination;
	chars obj_path;
	chars interface;

} TTrhDbus;

// #endregion


// #region Static globals

static TTrhDbus gsBus = {
	.ptr = 0,
	.slot = 0
};

// #endregion


// #region Exported functions

// Initialize dbus interface.
int trh_dbus_init( chars iDestination, chars iPath, chars iInterface, const sd_bus_vtable *iVtable )
{
	int lRetCode = TRH_OK;

	if( iDestination == 0 || iPath == 0 || iInterface == 0 || iVtable == 0 )
		return TRH_ARG_INVALID;

	gsBus.destination = iDestination;
	gsBus.obj_path = iPath;
	gsBus.interface = iInterface;

	if( sd_bus_open_system( &gsBus.ptr ) < 0 ) {
		trh_log( LOG_ERROR, "Failed to initialize dbus.\n" );
		return TRH_DBUS_INIT_FAILED;
	}

	sd_bus_add_object_manager( gsBus.ptr, 0, iDestination );

	// Install object
	if( ( lRetCode = sd_bus_add_object_vtable( gsBus.ptr, 0, iPath, iInterface, iVtable, 0 ) ) < 0 ) {
		trh_log( LOG_ERROR, "SDBUS failed to add VTABLE. Error: %s\n", strerror( -lRetCode ) );
		return TRH_DBUS_INIT_FAILED;
	}

	if( ( lRetCode = sd_bus_request_name( gsBus.ptr, iDestination, 0 ) ) < 0 ) {
		trh_log( LOG_ERROR, "SDBUS failed to request name. Error: %s\n", strerror( -lRetCode ) );
		return TRH_DBUS_INIT_FAILED;
	}

	return lRetCode;
}

void trh_dbus_subscribe_signal( chars iMatch, sd_bus_message_handler_t iCallback, chars iWarning )
{
	int lRetCode = 0;

	if( ( lRetCode = sd_bus_add_match( gsBus.ptr, 0, iMatch, iCallback, 0 ) ) < 0 ) {
		trh_log( LOG_ERROR, "Failed to subscribe to signal %s. Error: %s\n", iWarning, strerror( -lRetCode ) );
	}
}

// Process all incoming messages; send penging calls and signals.
int trh_dbus_process()
{
	int lRetCode = 0;

	if( gsBus.ptr == 0 )
		return TRH_OK;

	do {
		if( ( lRetCode = sd_bus_process( gsBus.ptr, 0 ) ) < 0 ) {
			trh_log( LOG_ERROR, "SDBUS failed to process. Error: %s\n", strerror( -lRetCode ) );
			return TRH_DBUS_PROCESS_FAILED;
		}
	} while( lRetCode > 0 );

	return TRH_OK;
}

void* trh_dbus_ptr()
{
	return gsBus.ptr;
}

// static int local_dbus_validate_message( TTrhDbusMessage *iMsg )
// {
// 	if( iMsg == 0 || iMsg->destination == 0 || iMsg->path == 0 || iMsg->interface == 0 || iMsg->member == 0 || iMsg->response == 0 )
// 		return TRH_ARG_INVALID;

// 	return TRH_OK;
// }

// int trh_dbus_method( TTrhDbusMessage *iMsg, ... )
// {
// 	int lCode = TRH_OK;

// 	if( ( lCode = local_dbus_validate_message( iMsg ) ) != TRH_OK )
// 		return lCode;

// 	_cleanup_(sd_bus_error_free) sd_bus_error lError = SD_BUS_ERROR_NULL;

// 	trh_log( LOG_DEBUG, "DBUS method %s... \n", iMsg->member );

// 	va_list args;
// 	va_start( args, iMsg );
// 	lCode = sd_bus_call_methodv(
// 		gsBus.ptr,							// sdbus object
// 		iMsg->destination,					// destination
// 		iMsg->path,							// object path
// 		iMsg->interface,					// interface
// 		iMsg->member,						// member
// 		&lError,							// return error
// 		&iMsg->response,					// reply
// 		iMsg->types,						// types of the following arguments
// 		args								// arguments
// 	);
// 	va_end(args);

// 	if( lCode < 0 ) {
// 		trh_log( LOG_ERROR, "DBUS method %s failed.\n", iMsg->member );
// 		return TRH_DBUS_REPLY_FAILED;
// 	}

// 	return TRH_OK;
// }

int trh_dbus_reply( sd_bus_message *iMsg )
{
	int lCode = sd_bus_send( gsBus.ptr, iMsg, 0 );

	if( lCode < 0 ) {
		trh_log( LOG_ERROR, "SDBUS Failed to send response. Error: %s\n", strerror( -lCode ) );
		return TRH_DBUS_SEND_FAILED;
	}

	return TRH_OK;
}

int trh_dbus_reply_error( sd_bus_message *iMsg, chars iText, int iErrno )
{
	//int local_reply_error( sd_bus_message *iMessage, chars iText, int iErrno, int iRetCode )
	sd_bus_error lError = SD_BUS_ERROR_NULL;
	trh_log( LOG_ERROR, "SDBUS %s. Error: %s\n", iText, strerror( -iErrno ) );
	sd_bus_error_set_errno( &lError, iErrno );
	sd_bus_reply_method_error( iMsg, &lError );
	return TRH_DBUS_ARG_FAILED;
}

// Close dbus link.
void trh_dbus_release()
{
	trh_log( LOG_DEBUG, "Releasing dbus...\n" );

	if( gsBus.slot != 0 ) {
		sd_bus_slot_unref( gsBus.slot );
		gsBus.slot = 0;
	}

	if( gsBus.ptr != 0 ) {
		sd_bus_release_name( gsBus.ptr, gsBus.destination );
		sd_bus_close( gsBus.ptr );
		gsBus.ptr = 0;
	}
}

// #endregion