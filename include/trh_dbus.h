/*
 * @brief Essential includes and generic defines
 * @copyright Copyright © 2022-2024 Trihlav, s.r.o.
 * @license MIT License / see LICENSE file
 */

#ifndef TRH_DBUS_H
#define TRH_DBUS_H

#include "trihlav.h"
#include <systemd/sd-bus.h>


/**
 * @brief IPC message executed in different process.
 */
typedef struct TTrhDbusMessage {
	// Dbus (IPC) destination of target process
	chars destination;
	// Dbus (IPC) path of executed method
	chars path;
	// Dbus (IPC) interface registered by target process
	chars interface;
	// Name of the executed method
	chars member;
	// Types of arguments passed to the method
	chars types;
	// Response from the method
	sd_bus_message *response;
} TTrhDbusMessage;

/**
 * @brief Initialize dbus interface.
 * @param iDestination Destination of the dbus.
 * @param iPath Path of the dbus.
 * @param iInterface Interface of the dbus.
 * @param iVtable Vtable of the dbus (const sd_bus_vtable*).
 * @retval TRH_OK
 * @retval TRH_DBUS_INIT_FAILED
 */
int trh_dbus_init( chars iDestination, chars iPath, chars iInterface, const sd_bus_vtable *iVtable );

/**
 * @brief Initialize dbus signal subscription.
 * @param iMatch Match string.
 * @param iCallback Callback function.
 * @param iWarning Warning message.
 */
void trh_dbus_subscribe_signal( chars iMatch, sd_bus_message_handler_t iCallback, chars iWarning );

/**
 * @brief Process all incoming messages; send penging calls and signals.
 * @retval TRH_OK
 * @retval TRH_DBUS_PROCESS_FAILED
 */
int trh_dbus_process();

/**
 * @brief Get pointer to dbus object (sd_bus*). 
 */
void* trh_dbus_ptr();

// /**
//  * @brief Call dbus method in a different service.
//  * @param iMsg Data identifying the target process, executed method, types of arguments.
//  * @param ... Arguments.
//  */
// int trh_dbus_method( TTrhDbusMessage *iMsg, ... );

/**
 * @brief Send response to received dbus message.
 * @param iMsg Response must be created using function sd_bus_message_new_method_return.
 */
int trh_dbus_reply( sd_bus_message *iMsg );

/**
 * @brief Send error response to received dbus message.
 * @param iMsg Message received from dbus (eg function executed from different process)
 * @param iText Error message (printed to log file).
 * @param iErrno Error number (printed to log file).
 * @return TRH_DBUS_ARG_FAILED
 */
int trh_dbus_reply_error( sd_bus_message *iMsg, chars iText, int iErrno );

/**
 * @brief Close dbus link.
 */
void trh_dbus_release();

#endif // TRH_DBUS_H
