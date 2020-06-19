/******************************************************************************
 * This file contains functions and data structures that operate on the
 *                                DIRECTORIES
 * of a FAT filesystem.
 *
 * By Daniel Huettner
 *****************************************************************************/

#ifndef DIRECTORY_H_
#define DIRECTORY_H_




//
// INCLUDES
//

// LAYER 1: USER_INTERFACE
// (NOTHING)

// LAYER 2: FILE_SYSTEM
#include "boot_sector.h"

// LAYER 3: STORAGE_DEVICE
// (NOTHING)

// ERROR HANDLING
#include "error.h"

// STANDARD C LIBRARY
#include <stdint.h>
#include <stdio.h>
#include <wchar.h>




//
// CONSTANTS
//

// DEFINES THE STANDARD RAW DIRECTORY ENTRY SIZE FOR A FAT FILE SYSTEM.
#define BYTES_PER_DIRECTORY_ENTRY 32

// DEFINES THE MAXIMUM NUMBER OF DIRECTORY ENTRIES PER VFAT SEQUENCE.
#define MAX_ENTRIES_PER_VFAT_SEQUENCE 21




/*
 * A data structure used to store the information for a file or directory.
 */
typedef struct file_t file_t;
struct file_t {

	wchar_t*  name;                // The name of the file (empty string for root).
	uint8_t   type;                // Set to 1 (TRUE) if this is a directory.
	uint32_t  size;                // The file size (0 for directories).
	uint32_t* clusters;            // The file's sequence of cluster numbers.
	uint32_t  numClusters;         // The number of cluster numbers in the sequence.
	file_t*   parentDirectory;     // The parent directory (NULL for root).
	file_t*   children;            // The child directories and files.
	uint32_t  numChildren;         // The number of child directories.

};




/*
 * Returns the full directory tree.  This is a tree data structure containing a
 * file_t for every file and directory stored in the device.
 */
file_t* getDirectoryTree(boot_sect_t* bootSector,
                         uint32_t*    fileAllocationTable,
                         FILE*        storageDevice);




#endif

