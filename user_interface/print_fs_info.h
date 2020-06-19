/******************************************************************************
 * This file contains functions that pertain to the
 *                               USER INTERFACE
 * of the program.
 *
 * By Daniel Huettner
 *****************************************************************************/

#ifndef PRINT_FS_INFO_H_
#define PRINT_FS_INFO_H_




//
// INCLUDES
//

// LAYER 1: USER_INTERFACE
#include "user_interface_tools.h"

// LAYER 2: FILE_SYSTEM
#include "boot_sector.h"

// LAYER 3: STORAGE_DEVICE
// (NOTHING)

// ERROR HANDLING
#include "error.h"

// STANDARD C LIBRARY
// (NOTHING)




//
// CONSTANTS
//

// THE NUMBER OF "|" SEPARATOR CHARACTERS PER ROW.
#define SEPARATORS_PER_ROW_FS 3

// THE WIDTH OF THE LEFT COLUMN IN THE FILE SYSTEM INFORMATION BOX.
#define LEFT_COLUMN_WIDTH_FS  19

// THE WIDTH OF THE RIGHT COLUMN IN THE FILE SYSTEM INFORMATION BOX.
#define RIGHT_COLUMN_WIDTH_FS (getTermWidth() - LEFT_COLUMN_WIDTH_FS - SEPARATORS_PER_ROW_FS)




/*
 * Prints information about the file system to the console.
 */
void printFileSystemInformation(char* deviceFileName,
                                boot_sect_t* bootSector);




#endif

