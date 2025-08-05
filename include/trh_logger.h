/*
 * @brief Application timers
 * @copyright Copyright © 2022-2024 Trihlav, s.r.o.
 * @license MIT License / see LICENSE file
 */

#ifndef TRH_LOGGER_H
#define TRH_LOGGER_H

// c++ compatibility
#ifdef __cplusplus
extern "C" {
#endif

#define TRH_ASSERT_ARG( cond, msg ) \
	if( ! ( cond ) ) { \
		trh_log( LOG_ERROR, "Assertion failed: " msg "\n" ); \
		assert( cond ); \
		return TRH_ARG_INVALID; \
	}

#define TRH_ASSERT_RET( cond, msg, ret ) \
	if( ! ( cond ) ) { \
		trh_log( LOG_ERROR, "Assertion failed: " msg "\n" ); \
		assert( cond ); \
		return ret; \
	}

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
 * @brief Log library version.
 */
void trh_log_version();

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

// c++ compatibility
#ifdef __cplusplus
}
#endif

#endif // TRH_LOGGER_H
 