/*
 * @brief Application Logger
 * @copyright Copyright © 2022-2025 Trihlav, s.r.o.
 * @license MIT License / see LICENSE file
 */

// #region Includes

#include <stdarg.h>
#include <string.h>
#include <time.h>

#include "trihlav.h"
#include "trh_std.h"
#include "trh_logger.h"

// #endregion

#define TRH_LOG_DEBUG			"[DEBUG]  "
#define TRH_LOG_NOTE			"[...]    "
#define TRH_LOG_WARN			"[WARN]   "
#define TRH_LOG_ERROR			"[ERROR]  "

#define TRH_LOG_DEBUG_CLI		"\033[0;37m[DEBUG]  \033[0m"
#define TRH_LOG_NOTE_CLI		"[...]    "
#define TRH_LOG_WARN_CLI		"\033[0;33m[WARN]   \033[0m"
#define TRH_LOG_ERROR_CLI		"\033[0;91m[ERROR]  \033[0m"

#define TRH_LOG_OK				"[OK]"
#define TRH_LOG_FAILED			"[FAILED]"

// #region Structs

typedef struct TAppLog {
	FILE *file;
	LogSeverity severity;

	LogSeverity current_message_severity;

	double time;
} TAppLog;

// #endregion


// #region Static globals

// Application object
static TAppLog gsLog = {
	.file = 0,
	.severity = LOG_NOTE,
	.current_message_severity = LOG_DEBUG,
	.time = 0
};

// #endregion


// #region Exported functions

int trh_log_init( chars iFilename )
{
	if( iFilename == 0 || *iFilename == 0 ) {
		printf( TRH_LOG_NOTE "Logging disabled.\n" );
		return TRH_END;
	}

	gsLog.time = trh_time();

	if( ( gsLog.file = fopen( iFilename, "a" ) ) == 0 ) {
		printf( TRH_LOG_WARN "Failed to open log file '%s'. Logging disabled.\n", iFilename );
		return TRH_FAILED;
	}

	printf( TRH_LOG_NOTE "Logging to file '%s'.\n", iFilename );

	return TRH_OK;
}

void trh_log_version()
{
	TAppVersion lAppVersion = { 0 };
	trh_version( &lAppVersion );
	trh_log(
		LOG_NOTE, "libtrihlav v.%u.%u.%u (0x%08x)\n",
		lAppVersion.major,
		lAppVersion.minor,
		lAppVersion.patch,
		lAppVersion.ver
	);
}

void trh_log( LogSeverity iSeverity, chars iMessage, ... )
{
	const int TimeLength = strlen( "YYYY-MM-DD HH:MM:SS" );

	if( iMessage == 0 || *iMessage == 0 )
		iMessage = "\n";

	va_list args;
	double lTime = trh_time();
	chars lTextSeverityFile = 0;
	chars lTextSeverityCli = 0;

	switch( iSeverity )
	{
	case LOG_DEBUG:   lTextSeverityFile = TRH_LOG_DEBUG; lTextSeverityCli = TRH_LOG_DEBUG_CLI; break;
	case LOG_NOTE:    lTextSeverityFile = TRH_LOG_NOTE;  lTextSeverityCli = TRH_LOG_NOTE_CLI;  break;
	case LOG_WARNING: lTextSeverityFile = TRH_LOG_WARN;  lTextSeverityCli = TRH_LOG_WARN_CLI;  break;
	case LOG_ERROR:   lTextSeverityFile = TRH_LOG_ERROR; lTextSeverityCli = TRH_LOG_ERROR_CLI; break;
	}

	va_start( args, iMessage );
	if( lTime - gsLog.time > 0.1 )
		printf( "\033[0;33m%06.3f\033[0m ", lTime - gsLog.time );
	else
		printf( "%06.3f ", lTime - gsLog.time );
	printf( "%s", lTextSeverityCli );
	vprintf( iMessage, args );
	gsLog.time = lTime;
	va_end( args );

	if( gsLog.file != 0 && gsLog.severity <= iSeverity ) {
		char lTimeBuffer[TimeLength+1]; // YYYY-MM-DD HH:MM:SS
		time_t lUnixTimestamp = time(0);

		memset( lTimeBuffer, 0, TimeLength+1 );
		strftime( lTimeBuffer, sizeof(lTimeBuffer), "%Y-%m-%d %T", localtime( &lUnixTimestamp ) );

		va_start( args, iMessage );

		fprintf( gsLog.file, "%s %s", lTimeBuffer, lTextSeverityFile );
		vfprintf( gsLog.file, iMessage, args );

		va_end( args );

		if( strchr( iMessage, '\n' ) != 0 )
			fflush( gsLog.file );
	}

	gsLog.current_message_severity = iSeverity;
}

void trh_log_more( chars iMessage, ... )
{
	if( iMessage == 0 || *iMessage == 0 ) return;

	va_list args;

	va_start( args, iMessage );

	vprintf( iMessage, args );

	va_end( args );

	if( gsLog.file != 0 && gsLog.severity <= gsLog.current_message_severity ) {
		va_start( args, iMessage );
		vfprintf( gsLog.file, iMessage, args );

		if( strchr( iMessage, '\n' ) != 0 )
			fflush( gsLog.file );

		va_end( args );
	}
}

void trh_log_end()
{
	printf( "\n" );

	if( gsLog.file != 0 && gsLog.severity <= gsLog.current_message_severity ) {
		fprintf( gsLog.file, "\n" );
		fflush( gsLog.file );
	}
}

void trh_log_set_severity_level( LogSeverity iSeverity )
{
	gsLog.severity = iSeverity;

	switch( iSeverity )
	{
	case LOG_DEBUG: printf( TRH_LOG_OK "Log severity set to DEBUG.\n" ); break;
	case LOG_NOTE: printf( TRH_LOG_OK "Log severity set to NOTE.\n" ); break;
	case LOG_WARNING: printf( TRH_LOG_OK "Log severity set to WARNING.\n" ); break;
	case LOG_ERROR: printf( TRH_LOG_OK "Log severity set to ERROR.\n" ); break;
	}
}

void trh_log_release()
{
	if( gsLog.file != 0 ) {
		fclose( gsLog.file );
		gsLog.file = 0;
	}
}

// #endregion
