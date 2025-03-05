/*
 * @brief Essential includes and generic defines
 * @copyright Copyright © 2022-2024 Trihlav, s.r.o.
 * @license MIT License / see LICENSE file
 */

// #region Includes

#include <string.h>
#include <errno.h>
#include <assert.h>
#include <signal.h>
#include <execinfo.h>
#include <sys/epoll.h>

#include "trihlav.h"

// #endregion

#define EPOLL_EVENTS 16

// #region Typedefs

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

	/// Epoll file descriptor.
	int epoll_fd;

	/// Protect application object in multi-threaded environment.
	pthread_mutex_t mutex;

	/// Callback function executed on main loop error.
	handle_loop_error handle_error;

	/// Pointer to extended object.
	void* ext;
} TApplication;

// #endregion


// #region Static functions

/**
 * @brief Called when `signal` fails to register signal handler.
 */
static int local_signal_failed( int iSignum );

/**
 * @brief Register system signals.
 * @retval TRH_OK on success.
 * @retval TRH_SIGNAL_FAILED failed to register signal.
 */
static int local_signal_register();

/**
 * @brief Initialize epoll object.
 */
static int local_epoll_init();

/**
 * @brief Executed when epoll_wait() returns -1.
 * @return TRH_WAITING An interrupt signal has been received and callback handler returned OK.
 * @return TRH_EPOLL_FAILED An error has been detected or callback handler did not return OK.
 * 
 * If callback handler is null, function always returns TRH_EPOLL_FAILED.
 * Callback handler is set (or disabled) using function \a trh_set_loop_error_handler.
 */
static int local_epoll_error();

/**
 * @brief Handle epoll event.
 */
static int local_epoll_event( struct epoll_event *iEvent );

/**
 * @brief Release epoll object.
 */
static void local_epoll_release();

// #endregion


// #region Static variables

// Pointer to application object
static TApplication gsApplication = { 0 };

// #endregion


// #region Exported functions

void trh_version(  TAppVersion *oVersion )
{
	assert( oVersion != 0 );
	if( oVersion == 0 ) return;

	if( oVersion->major != 0 ) oVersion->major = LIB_TRH_VERSION_MAJOR;
	if( oVersion->minor != 0 ) oVersion->minor = LIB_TRH_VERSION_MINOR;
	if( oVersion->patch != 0 ) oVersion->patch = LIB_TRH_VERSION_PATCH;

	// Convert major, minor and patch to version number using shift operands
	if( oVersion->ver != 0 ) oVersion->ver =
		( LIB_TRH_VERSION_MAJOR << 16 ) |
		( LIB_TRH_VERSION_MINOR << 8 ) |
		( LIB_TRH_VERSION_PATCH );
}

// Initialize application object.
TApplication *trh_init( void *iExt )
{
	bzero( &gsApplication, sizeof( gsApplication ) );

	gsApplication.time_system = trh_time();
	gsApplication.ext = iExt;

	// Register system signals.
	if( local_signal_register() != TRH_OK )
		return 0;

	// Initialize main thread mutex.
	pthread_mutex_init( &gsApplication.mutex, 0 );

	// Create epoll object
	if( local_epoll_init() != TRH_OK )
		return 0;

	return &gsApplication;
}

// Set callback that will be executed on signal from user space.
int trh_set_signal_handler( int iSignal, handle_signal_usr iHandler )
{
	if( iSignal != SIGUSR1 && iSignal != SIGUSR2 )
		return TRH_ARG_INVALID;

	if( iHandler == 0 )
		return TRH_ARG_INVALID;

	if( signal( iSignal, iHandler ) == SIG_ERR )
		return local_signal_failed( iSignal );

	return TRH_OK;
}

void trh_set_loop_error_handler( handle_loop_error iHandler )
{
	gsApplication.handle_error = iHandler;
}

// Update time
int trh_update()
{
	double lTime = trh_time();
	struct epoll_event lEvents[EPOLL_EVENTS];

	pthread_mutex_lock( &gsApplication.mutex );
	gsApplication.dt = lTime - gsApplication.time_system;
	gsApplication.time_system = lTime;
	gsApplication.time_app += gsApplication.dt;
	pthread_mutex_unlock( &gsApplication.mutex );

	int lEventCount = epoll_wait( gsApplication.epoll_fd, lEvents, EPOLL_EVENTS, 10 );

	if( lEventCount == -1 )
		local_epoll_error();

	for( int ii = 0; ii < lEventCount; ii++ )
		local_epoll_event( &lEvents[ii] );

	return TRH_OK;
}

double trh_get_app_time()
{
	double lResult = 0;
	pthread_mutex_lock( &gsApplication.mutex );
	lResult = gsApplication.time_app;
	pthread_mutex_unlock( &gsApplication.mutex );
	return lResult;
}

// Get time difference between two calls of trh_update().
double trh_get_dt()
{
	double lResult = 0;
	pthread_mutex_lock( &gsApplication.mutex );
	lResult = gsApplication.dt;
	pthread_mutex_unlock( &gsApplication.mutex );
	return lResult;
}

// Set flag 'application is now terminating'.
void trh_terminate()
{
	pthread_mutex_lock( &gsApplication.mutex );
	gsApplication.terminate = true;
	pthread_mutex_unlock( &gsApplication.mutex );
}

// Return 'true' if application is terminating.
bool trh_is_terminating()
{
	bool lResult = false;
	pthread_mutex_lock( &gsApplication.mutex );
	lResult = gsApplication.terminate;
	pthread_mutex_unlock( &gsApplication.mutex );
	return lResult;
}

void trh_release()
{
    // Destroy the mutex
    pthread_mutex_destroy( &gsApplication.mutex );
	// Release epoll object
	local_epoll_release();
}

// #endregion

// #region Events

int trh_event_register( TTrhEvent *iEvent )
{
	if( iEvent == 0 ) {
		trh_log( LOG_ERROR, "Failed to register event. Event is null.\n" );
		return TRH_ARG_INVALID;
	}

	struct epoll_event lEvent = {
		.events = EPOLLIN,
		.data.ptr = iEvent
	};

	if( epoll_ctl( gsApplication.epoll_fd, EPOLL_CTL_ADD, iEvent->fd, &lEvent ) == -1 ) {
		trh_log( LOG_ERROR, "Failed to register event: %s.", strerror( errno ) );
		return TRH_EPOLL_FAILED;
	}

	return TRH_OK;
}

void trh_event_unregister( TTrhEvent *iEvent )
{
	if( iEvent == 0 ) {
		trh_log( LOG_ERROR, "Failed to unregister event. Event is null.\n" );
		return;
	}

	if( gsApplication.epoll_fd == -1 )
		return;

	epoll_ctl( gsApplication.epoll_fd, EPOLL_CTL_DEL, iEvent->fd, 0 );
}

// #endregion // Events


// #region Static functions

// #region Signal handling

int local_signal_failed( int iSignum )
{
	trh_log( LOG_ERROR, "Failed to register a signal %d. Error: %s\n", iSignum, strerror( errno ) );
	return TRH_SIGNAL_FAILED;
}

static void local_signal_handle_exit( int iSignum )
{
	trh_log_end();
	trh_log( LOG_NOTE, "SIGNAL %d HAS BEEN RECEIVED. APPLICATION WILL NOW STOP.\n", iSignum );

	trh_terminate();
}

static void local_signal_handle_error( int iSignum )
{
	trh_log_end();
	trh_log( LOG_ERROR, "SIGNAL %d HAS BEEN RECEIVED. SEGFAULT AT %lu\n", iSignum, (uint64_t)time(0) );

	#define SIZE 100
	void *buffer[SIZE];
	int nptrs = backtrace(buffer, SIZE);
	char **strings = backtrace_symbols(buffer, nptrs);

	if( strings != 0 ) {
		printf( "BACKTRACE (%i): \n", nptrs );
		for( int jj = 0; jj < nptrs; jj++ )
			printf( "%i %s\n", jj, strings[jj] );
		free(strings);
	}
	else {
		printf( "NO BAKCTRACE.\n" );
	}
}

static int local_signal_register()
{
	// When a child process stops or terminates, SIGCHLD is sent to the parent process. Ignore this signal.
	if( signal( SIGCHLD, SIG_IGN ) == SIG_ERR )
		return local_signal_failed( SIGCHLD );

	// ctrl+c (process should close)
	if( signal( SIGINT, local_signal_handle_exit ) == SIG_ERR )
		return local_signal_failed( SIGINT );

	if( signal( SIGILL, local_signal_handle_error ) == SIG_ERR )
		return local_signal_failed( SIGILL );

	if( signal( SIGABRT, local_signal_handle_error ) == SIG_ERR )
		return local_signal_failed( SIGABRT );

	if( signal( SIGFPE, local_signal_handle_error ) == SIG_ERR )
		return local_signal_failed( SIGFPE );

	if( signal( SIGSEGV, local_signal_handle_error ) == SIG_ERR )
		return local_signal_failed( SIGSEGV );

	// request to terminate
	if( signal( SIGTERM, local_signal_handle_exit ) == SIG_ERR )
		return local_signal_failed( SIGTERM );

	// Historical signals		

	// hang-up - device is shuting down
	if( signal( SIGHUP, local_signal_handle_exit ) == SIG_ERR )
		return local_signal_failed( SIGHUP );
	
	if( signal( SIGQUIT, local_signal_handle_exit ) == SIG_ERR )
		return local_signal_failed( SIGQUIT );

	return TRH_OK;
}

// #endregion // Signal handling


// #region Epoll

// Initialize epoll object.
int local_epoll_init()
{
	gsApplication.epoll_fd = epoll_create1( 0 );

	if( gsApplication.epoll_fd == -1 ) {
		trh_log( LOG_ERROR, "Failed to create epoll. Error: %s\n", strerror( errno ) );
		return TRH_EPOLL_FAILED;
	}

	return TRH_OK;
}

int local_epoll_error()
{
	if( errno == EINTR && gsApplication.handle_error != 0 && gsApplication.handle_error() == TRH_OK )
		return TRH_WAITING;

	if( errno == EINTR )
		trh_log( LOG_WARNING, "Application loop terminated by interrupt signal.\n" );
	else
		trh_log( LOG_ERROR, "Error while checking for epoll events. Error: %s\n", strerror( errno ) );

	return TRH_EPOLL_FAILED;
}

int local_epoll_event( struct epoll_event *iEvent )
{
	TTrhEvent *lEvent = (TTrhEvent*)iEvent->data.ptr;

	// Check if file descriptor has been closed, or network connection has been lost.
	if( iEvent->events & EPOLLERR ) {
		trh_log( LOG_WARNING, "EPOLLERR on fd %d\n", iEvent->data.fd );
		if( lEvent->handle_error != 0 )
			lEvent->handle_error( lEvent );
		return TRH_EPOLL_ERROR;
	}

	// Check if file descriptor is ready for reading.
	if( iEvent->events & EPOLLIN ) {
		assert( lEvent->handle_event != 0 );
		lEvent->handle_event( lEvent );
	}

	return TRH_OK;
}

// Release epoll object.
void local_epoll_release()
{
	CLOSE_FD( gsApplication.epoll_fd );
}

// #endregion // Epoll


// #endregion // Static functions