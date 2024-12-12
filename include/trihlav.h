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
#include <pthread.h>

// #endregion


// #region Defines

// Free resource from memory
#define FREE_PTR( p ) \
	if( p != 0 ) { \
		free( p ); \
		p = 0; \
	}


// Return codes
#define TRH_OK						0

#define TRH_WAITING					1
#define TRH_END						2
#define TRH_ARG_INVALID				3
#define TRH_OUT_OF_MEM				4
#define TRH_SIGNAL_FAILED			5
#define TRH_UNINITIALIZED			6
#define TRH_EPOLL_FAILED			7

#define TRH_JSON_LOAD_FAILED		16
#define TRH_JSON_INVALID			17

// #endregion


// #region Typedefs

typedef const char* chars;

/**
 * @brief Application properties.
 */
typedef struct TApplication {
	/// System time (unix timestamp)
	double time_system;

	/// Time (in seconds) since application started
	double time_app;

	/// Delta time since last iteration of main application loop.
	double dt;

	/// If true, application is terminating.
	bool terminate;

	/// Protect application object in multi-threaded environment.
	pthread_mutex_t mutex;

	/// Pointer to extended object.
	void* ext;
} TApplication;

// #endregion


// #region Exported functions

/**
 * @brief Get application object.
 */
TApplication *trh_init( TApplication *iApp, void *iExt );

/**
 * @brief Return reference to application object.
 */
TApplication *trh_app();

/**
 * @brief Get current system time as real number.
 */
double trh_time();

/**
 * @brief Stop the application. Thread-safe set geApplication.terminate to false.
 */
void trh_terminate();

/**
 * @brief Check if application is terminating.
 */
bool trh_is_terminating();

// #endregion

#endif // TRHIHLAV_H
