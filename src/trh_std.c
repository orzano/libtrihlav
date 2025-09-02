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


// #region Static variables

/// Application paths
char *gsPaths[TRH_ASSETS];

// #endregion


// #region Exported functions

// Reset all local data.
int trh_std_init()
{
	bzero( gsPaths, sizeof( gsPaths ) );
	return TRH_OK;
}


// Get system time
double trh_time()
{
	struct timespec lTime;

	clock_gettime( CLOCK_REALTIME, &lTime );

	return (double)( lTime.tv_sec ) + (double)( lTime.tv_nsec ) / 1000000000.0;
}


// #region GET PATH

static chars local_get_path_home()
{
	if( gsPaths[TRH_HOME] != 0 ) return gsPaths[TRH_HOME];

	chars lHomeEnv = getenv( "HOME" );

	if( lHomeEnv == 0 || lHomeEnv[0] == 0 ) {
		trh_log( LOG_ERROR, "Environment variable HOME is not set.\n" );
		return "";
	}

	size_t lHomeLen = strlen( lHomeEnv );
	gsPaths[TRH_HOME] = malloc( lHomeLen + 2 );
	strcpy( gsPaths[TRH_HOME], lHomeEnv );

	if( gsPaths[TRH_HOME][lHomeLen - 1 ] != PATH_SEP_C )
		strcat( gsPaths[TRH_HOME], PATH_SEP );

	return gsPaths[TRH_HOME];
}

static chars local_get_path_xdg( chars iProjectName, chars iEnvVar, chars iDefaultPath, char **oPath )
{
	// HOME or XDG_* env variable.
	char* lEnvPath = 0;

	// If path is already defined, return its value.
	if( *oPath != 0 )
		return *oPath;

	// - Try get value of provided env variable. XDG_* is not set up on most systems by default.
	// - The second condition checks if the combined path is less than PATH_MAX chars long.
	if( ( lEnvPath = getenv( iEnvVar ) ) != 0 ) {
		*oPath = malloc( strlen( lEnvPath ) + strlen( iProjectName ) + 3 );
		sprintf( *oPath, "%s" PATH_SEP "%s" PATH_SEP, lEnvPath, iProjectName );
	}

	// If XDG_* is not defined, and iDefaultPath points to an absolute path, set iDefaultPath.
	else if( iDefaultPath[0] == PATH_SEP_C ) {
		*oPath = malloc( strlen( iDefaultPath ) + strlen( iProjectName ) + 2 );
		sprintf( *oPath, "%s%s" PATH_SEP, iDefaultPath, iProjectName );
	}

	// If XDG_* is not defined, and iDefaultPath is relative, generate path pointing to $HOME/iDefaultPath.
	else {
		chars lPathHome = local_get_path_home();
		*oPath = malloc( strlen( lPathHome ) + strlen( iDefaultPath ) + strlen( iProjectName ) + 2 );
		sprintf( *oPath, "%s%s%s" PATH_SEP, lPathHome, iDefaultPath, iProjectName );
	}

	trh_create_path( *oPath );

	return *oPath;
}

static chars local_get_path_assets( chars iProjectName )
{
	if( gsPaths[TRH_ASSETS] != 0 ) return gsPaths[TRH_ASSETS];
	chars lPath = "../share/";
	gsPaths[TRH_ASSETS] = malloc( strlen( lPath ) + strlen( iProjectName ) + 2 );
	sprintf( gsPaths[TRH_ASSETS], "%s%s" PATH_SEP, lPath, iProjectName );
	return gsPaths[TRH_ASSETS];
}

// Return path to HOME, TEMP, RUNTIME, CONFIG, DATA or ASSETS directory.
void trh_get_path( chars iProjectName, PathType iType, chars *oPath )
{
	if( oPath == 0 )
		return;

	switch( iType ) {
		case TRH_HOME: *oPath = local_get_path_home(); break;
		case TRH_TEMP: *oPath = local_get_path_xdg( iProjectName, "XDG_CACHE_HOME", ".cache/", &gsPaths[TRH_TEMP] ); break;
		case TRH_RUNTIME: *oPath = local_get_path_xdg( iProjectName, "XDG_RUNTIME_DIR", "/run/", &gsPaths[TRH_RUNTIME] ); break;
		case TRH_CONFIG: *oPath = local_get_path_xdg( iProjectName, "XDG_CONFIG_HOME", ".config/", &gsPaths[TRH_CONFIG] ); break;
		case TRH_DATA: *oPath = local_get_path_xdg( iProjectName, "XDG_DATA_HOME", ".local/share/", &gsPaths[TRH_DATA] ); break;
		case TRH_ASSETS: *oPath = local_get_path_assets( iProjectName ); break;//"../share/" PRJ_NAME PATH_SEP; break;
		default: *oPath = local_get_path_xdg( iProjectName, "XDG_CACHE_HOME", ".cache/", &gsPaths[TRH_TEMP] ); break;
	}
}

// #endregion // GET PATH


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

void trh_std_release()
{
	for( int ii = 0; ii < TRH_ASSETS; ii++ ) {
		if( gsPaths[ii] != 0 ) {
			FREE_PTR( gsPaths[ii] );
		}
	}
}


// #endregion
