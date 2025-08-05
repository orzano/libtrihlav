/*
 * @brief Common functions and definitions
 * @copyright Copyright © 2022-2025 Trihlav, s.r.o.
 * @license MIT License / see LICENSE file
 */

#ifndef TRH_STD_H
#define TRH_STD_H

// c++ compatibility
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief File type is mandatory argument passed to \a trh_file_exists.
 */
typedef enum FileType {
	TRH_FILE,
	TRH_DIRECTORY,
	TRH_LINK,
	TRH_SOCKET
} FileType;

/**
 * @brief Path type is mandatory argument passed to \a trh_get_path.
 */
typedef enum PathType {
	/// Path to HOME directory.
	TRH_HOME,
	/// Path to temporary directory $XDG_CACHE_HOME/PRJ_NAME/ or /tmp/PRJ_NAME/
	TRH_TEMP,
	/// Path to run-time directory ($XDG_RUNTIME_DIR/PRJ_NAME/ or "/run/PRJ_NAME/")
	TRH_RUNTIME,
	/// Path to config dir ($XDG_CONFIG_HOME/PRJ_NAME/ or $HOME/.config/PRJ_NAME/)
	TRH_CONFIG,
	/// Path to data dir ($XDG_DATA_HOME/PRJ_NAME/ or $HOME/.local/share/PRJ_NAME/)
	TRH_DATA,
	/// Path to assets. "../share/PRJ_NAME/".
	TRH_ASSETS
} PathType;

/**
 * @brief Clear application state.
 * @return TRH_OK on success.
 */
int trh_std_init();

/**
 * @brief Get current system time as real number.
 */
double trh_time();

/**
 * @brief Return system or local path.
 * @param iProjectName Name of the project. If empty, default path will be used.
 * @param iType Type of the requested path.
 *
 * Acctepted paths:
 * - \a HOME = /home/{username}/ - path to user's home directory
 * - \a TEMP = $XDG_CACHE_HOME/PRJ_NAME/ or "/tmp/PRJ_NAME/"
 * - \a RUNTIME - $XDG_RUNTIME_DIR/PRJ_NAME/ ("/run/user/1000" on arch) or "/tmp/PRJ_NAME/"
 * - \a CONFIG - $XDG_CONFIG_HOME/PRJ_NAME/ or $HOME/.config/PRJ_NAME/
 * - \a DATA - $XDK_DATA_HOME/PRJ_NAME/ or $HOME/.local/share/PRJ_NAME/
 * - \a ASSETS - ../share/PRJ_NAME/ 
 */
void trh_get_path( chars iProjectName, PathType iType, chars oPath );

/**
 * @brief Test if file exists (at specified path).
 * @param iFilePath Filename, including path.
 * @param iFileType File type (file, directory, link, socket).
 * @retval true File exists.
 * @retval false File not found.
 */
bool trh_file_exists( chars iFilePath, FileType iFileType );

/**
 * @brief Copy file to a new destination.
 * @param iSourceFileName Name of the file that we want to copy.
 * @param iDestFileName Name of the destination file.
 * @retval TRH_OK File has been successfully copied.
 * @retval TRH_ARG_INVALID One of the arguments is invalid.
 * @retval TRH_SKIP Source file does not exist.
 * @retval TRH_FILE_ERROR if iSourceFile does not exists or iDestFile is not accessible.
 */
int trh_copy_file( chars iSourceFileName, chars iDestFileName );

/**
 * @brief Delete a file.
 * @param iFilepath Name of the file to delete.
 * @retval TRH_OK File has been successfully deleted.
 * @retval TRH_ARG_INVALID \a iFilepath is invalid.
 * @retval TRH_FILE_ERROR File not found or not accessible.
 */
int trh_delete_file( chars iFilepath );

/**
 * @brief Try to create a directory.
 * @param iPath Dir name. Can include path.
 * @param iLog If true, log message if directory already exists.
 * @retval TRH_OK Directory has been successfully created.
 * @retval TRH_ARG_INVALID \a iPath is invalid.
 * @retval TRH_SKIP Directory already exists.
 * @retval TRH_FILE_ERROR Failed to create directory.
 *
 * Only directory at the end of the path will be created. If path to this directory does not exist,
 * it won't be created.
 */
int trh_create_directory( chars iPath, bool iLog );

/**
 * @brief Try to create a directory path.
 * @param iPath Directory path.
 * @retval TRH_OK Path has been successfully created.
 * @retval TRH_ARG_ERROR \a iPath is invalid.
 * @retval TRH_SKIP Path already exists.
 * @retval TRH_FILE_ERROR Insuficient privileges.
 */
int trh_create_path( chars iPath );

/**
 * @brief Clear std resources.
 */
void trh_std_release();

// c++ compatibility
#ifdef __cplusplus
}
#endif

#endif // TRH_STD_H
 