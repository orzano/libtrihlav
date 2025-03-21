/*
 * @brief Essential includes and generic defines
 * @copyright Copyright © 2022-2024 Trihlav, s.r.o.
 * @license MIT License / see LICENSE file
 */

#ifndef TRHIHLAV_H
#define TRHIHLAV_H

// #region includes

#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <pthread.h>

// #endregion


// #region Defines

// Free resource from memory
#define FREE_PTR( p ) \
	if( p != 0 ) { \
		free( p ); \
		p = 0; \
	}

// Close file descriptor
#define CLOSE_FD( fd ) \
	if( fd != -1 ) { \
		close( fd ); \
		fd = -1; \
	}

// Return codes - pls don't go above 1024 and below -1024
#define TRH_OK						1
#define TRH_WAITING					2
#define TRH_END						3

#define TRH_JSON_LOAD_FAILED		16
#define TRH_JSON_INVALID			17

#define TRH_FAILED					0
#define TRH_ARG_INVALID				-1
#define TRH_OUT_OF_MEM				-2
#define TRH_SIGNAL_FAILED			-3
#define TRH_UNINITIALIZED			-4

#define TRH_EPOLL_FAILED			-32
#define TRH_EPOLL_ERROR				-33
#define TRH_TIMER_FAILED			-34

#define TRH_DBUS_INIT_FAILED		-64
#define TRH_DBUS_ARG_FAILED			-65
#define TRH_DBUS_REPLY_FAILED		-66
#define TRH_DBUS_SEND_FAILED		-67
#define TRH_DBUS_PROCESS_FAILED		-68


#define TRH_ASSERT_ARG( cond, msg ) \
	if( ! ( cond ) ) { \
		trh_log( LOG_ERROR, "Assertion failed: " msg "\n" ); \
		assert( cond ); \
		return TRH_ARG_INVALID; \
	}


#define _cleanup_(f) __attribute__((cleanup(f)))

typedef const char* chars;

// #endregion


// #region Std (trh_std.c)

/**
 * @brief Get current system time as real number.
 */
double trh_time();

// #endregion // Std


// #region Application (trihlav.c)

/**
 * @brief Application object. Definition in trihlav.c.
 */
struct TApplication;

/**
 * @brief Application version; returned by trh_version().
 */
typedef struct TAppVersion {
	uint8_t major;
	uint8_t minor;
	uint8_t patch;
	uint32_t ver;
} TAppVersion;

/// Callback for handling signals from user space (SIGUSR1, SIGUSR2).
typedef void (*handle_signal_usr)( int );

/// Callback for handling main loop (epoll) errors
typedef int (*handle_loop_error)();

/**
 * @brief Get version of Trihlav library.
 */
void trh_version( TAppVersion *oVersion );

/**
 * @brief Initialize application object and return pointer to its address.
 * @param iExt Pointer to external data.
 * @return Pointer to application object. If null, application should terminate immediately. 
 */
struct TApplication *trh_init( void *iExt );

/**
 * @brief Set callback that will be executed on signal from user space.
 * @param iSignal Signal number. Only SIGUSR1 and SIGUSR2 are allowed.
 * @param iHandler Callback function.
 * @retval TRH_OK on success.
 * @retval TRH_ARG_INVALID invalid signal number or iHandler is null.
 * @retval TRH_SIGNAL_FAILED failed to register signal.
 */
int trh_set_signal_handler( int iSignal, handle_signal_usr iHandler );

/**
 * @brief Set callback that will be executed on main loop error.
 * 
 * To disable error handling, set iHandler to null.
 */
void trh_set_loop_error_handler( handle_loop_error iHandler );

/**
 * @brief Update application state.
 * @retval TRH_OK on success.
 * @retval TRH_WAITING No events to process.
 * @retval TRH_EPOLL_FAILED Failed to wait for events (interrupt signal received?)
 * 
 * - update time, dt
 * - epoll wait for events (non-blocking)
 * 
 */
int trh_update();

/**
 * @brief Get application time.
 */
double trh_get_app_time();

/**
 * @brief Get time difference between two calls of trh_update().
 */
double trh_get_dt();

/**
 * @brief Stop the application. Thread-safe.
 */
void trh_terminate();

/**
 * @brief Check if application is terminating.
 */
bool trh_is_terminating();

/**
 * @brief Release application resources.
 */
void trh_release();

// #endregion // Application


// #region Events

struct TTrhEvent;
struct TTrhTimer;

/// Callback function handling timer event.
typedef int (*handle_event)( struct TTrhEvent *iEvent );

/**
 * @brief Event object registered with epoll.
 */
typedef struct TTrhEvent {
	/// File descriptor registered with epoll.
	int fd;
	/// Callback function executed when event is triggered.
	handle_event handle_event;
	/// Callback function executed on error.
	handle_event handle_error;

	/// Extended data.
	union {
		/// Pointer to timer event object.
		struct TTrhTimerProperties *timer;
		/// Pointer to user data.
		void *data;
	} ext;
} TTrhEvent;


/**
 * @brief Register event with epoll
 * @param iFd File descriptor of the timer.
 * @param iTimer Pointer to the timer object.
 */
int trh_event_register( TTrhEvent *iEvent );

/**
 * @brief Unregister event from epoll.
 */
void trh_event_unregister( TTrhEvent *iEvent );


// #endregion // Events


// #region Logging (trh_log.c)

typedef enum LogSeverity
{
	LOG_DEBUG,
	LOG_NOTE,
	LOG_WARNING,
	LOG_ERROR
} LogSeverity;

/**
 * @brief Initialize logging.
 * @param iFileName Name of the log file.
 * @retval TRH_OK on success.
 * @retval THR_END iFileName is empty - logging to file is disabled.
 * @retval TRH_FAILED failed to open log file.
 *
 * If logging is disabled, messages will be still printed to stdout.
 */
int trh_log_init( chars iFileName );

/**
 * @brief Print and log a message.
 * @param iSeverity Severity of the message.
 * @param iMessage Message to be printed and logged. If null or empty, endline is printed.
 * @param ... Optional arguments, see printf documentation.
 *
 * Text writen into log file starts with a date. Date follows format "YYYY-MM-DD HH:MM:SS"
 * If message (\a iMessage) ends with end-line, log file is flushed.
 */
void trh_log( LogSeverity iSeverity, chars iMessage, ... );

/**
 * @brief Print and log a message.
 * @param iMessage Message to be printed and logged.
 * @param ... Optional arguments, see printf documentation.
 *
 * Text is not prepended with a date.
 * If message (\a iMessage) ends with end-line, log file is flushed.
 */
void trh_log_more( chars iMessage, ... );

/**
 * @brief Log end-line. Function does not prepend date.
 * 
 * To log end-line with date, use `trh_log( LOG_NOTE, 0 )`.;
 */
void trh_log_end();

/**
 * @brief Set new severity level.
 * @param iSeverity New severity level.
 *
 * Messages with lower severity level then \a iSeverity won't be written to log file.
 * Severity has no effect on stdout output.
 */
void trh_log_set_severity_level( LogSeverity iSeverity );

/**
 * @brief Close the log file.
 */
void trh_log_release();

// #endregion // Logging



#endif // TRHIHLAV_H
