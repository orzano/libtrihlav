/*
 * @brief Essential includes and generic defines
 * @copyright Copyright © 2022-2024 Trihlav, s.r.o.
 * @license MIT License / see LICENSE file
 */

// #region Includes

#include <string.h>
#include <errno.h>
#include <limits.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <sys/file.h>

#include "trihlav.h"
#include "trh_logger.h"
#include "trh_std.h"

// #endregion

#define PATH_SEP				"/"
#define PATH_SEP_C				'/'


// #region Exported functions

// Get system time
double trh_time()
{
	struct timespec lTime;

	clock_gettime( CLOCK_REALTIME, &lTime );

	return (double)( lTime.tv_sec ) + (double)( lTime.tv_nsec ) / 1000000000.0;
}


// Check if file/directory/socket/link exists.
bool trh_file_exists( chars iFilepath, FileType iFileType )
{
	TRH_ASSERT_RET( iFilepath != 0 && iFilepath[0] != 0, "File path is null.\n", false );

	struct stat lBuf;

	if( stat( iFilepath, &lBuf ) != 0 ) 
		return false;

	switch( iFileType ) {
		case TRH_FILE: return S_ISREG( lBuf.st_mode );
		case TRH_DIRECTORY: return S_ISDIR( lBuf.st_mode );
		case TRH_LINK: return S_ISLNK( lBuf.st_mode );
		case TRH_SOCKET: return S_ISSOCK( lBuf.st_mode );
		default: return false;
	}
}

// Skopiruje subor na nove miesto.
int trh_copy_file( chars iSourceFileName, chars iDestFileName )
{
 	TRH_ASSERT_ARG( iSourceFileName != 0 && iSourceFileName[0] != 0, "Failed to copy file, source file name is invalid" );
 	TRH_ASSERT_ARG( iDestFileName != 0 && iDestFileName[0] != 0, "Failed to copy file, destination is invalid" );

	if( ! trh_file_exists( iSourceFileName, TRH_FILE ) ) {
		trh_log( LOG_WARNING, "File '%s' does not exists.\n", iSourceFileName );
		return TRH_SKIP;
	}

	struct stat lStatBuf;
	int lFileSrc;
	int lFileDst;
	ssize_t lTransferred;

	// Try open the source file
	if( ( lFileSrc = open( iSourceFileName, O_RDONLY ) ) < 0 ) {
		trh_log( LOG_WARNING, "Failed to open file '%s'. Error: %s\n", iSourceFileName, strerror(errno) );
		return TRH_FILE_ERROR;
	}

 	// Stat the input file to obtain its size.
 	fstat( lFileSrc, &lStatBuf );

 	// Try open the output file for writing, with the same permissions as the source file.
 	if( ( lFileDst = open( iDestFileName, O_WRONLY | O_CREAT | O_TRUNC, lStatBuf.st_mode ) ) < 0 ) {
		trh_log( LOG_WARNING, "Failed to open file '%s'. Error: %s\n", iDestFileName, strerror(errno) );
 		close( lFileSrc );
 		return TRH_FILE_ERROR;
 	}

	// Blast the bytes from one file to the other.
	do {
		lTransferred = sendfile( lFileDst, lFileSrc, 0, lStatBuf.st_size );

		if( lTransferred < 0 || lTransferred > lStatBuf.st_size ) {
			trh_log( LOG_WARNING, "Failed to copy file '%s' to '%s'. Error: %s\n", iSourceFileName, iDestFileName, strerror(errno) );
			break;
		}

		lStatBuf.st_size -= lTransferred;
	} while( lStatBuf.st_size > 0 );

	// Close the files.
	close( lFileSrc );
	close( lFileDst );

	return TRH_OK;
}

int trh_delete_file( chars iFilePath )
{
	TRH_ASSERT_ARG( iFilePath != 0 && iFilePath[0] != 0, "Failed to delete file - file name invalid" );

	if( unlink( iFilePath ) < 0 ) {
		strerror( errno );
		trh_log( LOG_WARNING, "Failed to delete file '%s'.\n", iFilePath );
		return TRH_FILE_ERROR;
	}

	return TRH_OK;
}

// Pokusi sa vytvorit adresar; ak zlyha, skusi zistit ci uz dir existuje (vrati true)
// alebo ci ma locknute prava (vrati false).
int trh_create_directory( chars iPath, bool iLog  )
{
	TRH_ASSERT_ARG( iPath != 0 && iPath[0] != 0, "Failed to create directory - path invalid" );

	if( mkdir( iPath, 0750) != 0 ) {
		if( errno == EEXIST ) {
			if( iLog ) trh_log( LOG_NOTE, "Can't create directory '%s'. Directory already exists.\n", iPath );
			return trh_file_exists( iPath, TRH_DIRECTORY ) ? TRH_SKIP : TRH_FILE_ERROR;
		} else {
			trh_log( LOG_WARNING, "Failed to create directory '%s'. Error: %s\n", iPath, strerror(errno) );
			return TRH_FILE_ERROR;
		}
	}

	return TRH_OK;
}

int trh_create_path( chars iPath )
{
	TRH_ASSERT_ARG( iPath != 0 && strlen( iPath ) > 1, "Failed to create path - path invalid" );

	// Cesta uz existuje. Vrat true.
	if( trh_file_exists( iPath, TRH_DIRECTORY ) )
		return TRH_SKIP;

	char lPathCopy[PATH_MAX];
	char *lPathIdx;
	int32_t lCode;

	// Create a copy of the path, so we can modify it.
	strncpy( lPathCopy, iPath, PATH_MAX );
	// Last character of the path must be a directory separator.
	if( lPathCopy[ strlen( lPathCopy ) - 1 ] != PATH_SEP_C )
		strcat( lPathCopy, PATH_SEP );
	// Create index.
	lPathIdx = strchr( lPathCopy + 1, PATH_SEP_C );

	// Create every directory in the path.
	while( lPathIdx != 0 )
	{
		lPathIdx[0] = 0;

		// If top levels of path already exist, trh_create_directory will return TRH_SKIP:
		// Do not exit loop, but continue creating the rest of the path.
		if( ( lCode = trh_create_directory( lPathCopy, false ) ) < TRH_OK )
			return lCode;

		lPathIdx[0] = PATH_SEP_C;
		lPathIdx = strchr( lPathIdx + 1, PATH_SEP_C );
	}

	return TRH_OK;
}


// #endregion
