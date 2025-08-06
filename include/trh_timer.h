/*
 * @brief Application timers
 * @copyright Copyright © 2022-2024 Trihlav, s.r.o.
 * @license MIT License / see LICENSE file
 */

#ifndef TRH_TIMER_H
#define TRH_TIMER_H

// c++ compatibility
#ifdef __cplusplus
extern "C" {
#endif

struct TTrhEvent;
struct TTrhTimer;

typedef enum TTrhTimerState {
	TRH_TIMER_STOPPED,
	TRH_TIMER_RUNNING,
	TRH_TIMER_EXPIRED
} TTrhTimerState;

typedef struct TTrhTimerProperties {
	/// Timer duration.
	time_t sec;
	time_t nsec;

	/// If true, timer will be repeated.
	bool repeat;

	/// - RUNNING state is managed internally.
	/// - STOPPED state is set when timer is stopped in \a trh_timer_stop() or \a repeat is false.
	/// - EXPIRED state can be set in timer event handler ( \a handle_timer_event) to release the timer resources.
	TTrhTimerState state;

	// Attached user data.
	void *ext;

	// Called when timer event is triggered.
	handle_event handle_timer_event;
	// Called when timer event is released from memory.
	handle_event handle_timer_stopped;
} TTrhTimerProperties;


/**
 * @brief Create a new timer. New timer is by default created in enabled state.
 * @retval TRH_INVALID_ARG iProperties or oEvent is null.
 * @retval TRH_OK
 *
 * - allocate memory for timer event
 * - initialize timer 
 * - register timer with epoll
 * 
 * If iProperties has been allocated in memory, it can be released after this function.
 * oEvent must be released with trh_timer_release().
 */
int trh_timer_init( TTrhTimerProperties *iProperties, TTrhEvent **oEvent );

/**
 * @brief Start timer. Timer will be removed from epoll
 * @retval TRH_INVALID_ARG iEvent is null.
 * @retval TRH_OK
 */
int trh_timer_start( TTrhEvent *oEvent );

/**
 * @brief Stop the timer. Timer will be added to epoll.
 */
void trh_timer_stop( TTrhEvent *oEvent );

/**
 * @brief Release timer resources.
 * 
 * - unregister timer from epoll
 * - close timer
 * - free memory
 */
void trh_timer_release( TTrhEvent *iEvent );

// c++ compatibility
#ifdef __cplusplus
}
#endif

#endif // TRH_TIMER_H
