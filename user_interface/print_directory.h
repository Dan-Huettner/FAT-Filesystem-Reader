/******************************************************************************
 * This file contains functions that pertain to the
 *                               USER INTERFACE
 * of the program.
 *
 * By Daniel Huettner
 *****************************************************************************/

#ifndef PRINT_DIRECTORY_H_
#define PRINT_DIRECTORY_H_




//
// INCLUDES
//

// LAYER 1: USER_INTERFACE
#include "user_interface_tools.h"

// LAYER 2: FILE_SYSTEM
#include "directory.h"

// LAYER 3: STORAGE_DEVICE
// (NOTHING)

// ERROR HANDLING
#include "error.h"

// STANDARD C LIBRARY
#include <stdint.h>




//
// CONSTANTS
//

// THE WIDTH OF THE LEFT COLUMN (NOT COUNTING THE "|" SEPARATORS).
#define CHARACTERS_PER_ROW_LEFT_COLUMN     8

// THE NUMBER OF "|" SEPARATORS.
#define NUMBER_OF_SEPARATORS               3

// THE WIDTH OF THE RIGHT COLUMN (NOT COUNTING THE "|" SEPARATORS).
#define CHARACTERS_PER_ROW_RIGHT_COLUMN    (getTermWidth() - CHARACTERS_PER_ROW_LEFT_COLUMN - NUMBER_OF_SEPARATORS)

// THE WIDTH OF EACH FAT12 CLUSTER NUMBER PRINTED TO THE CONSOLE (i.e. 0xfff).
#define CHARACTERS_PER_FAT12_CLUSTER_NUMBER 5

// THE WIDTH OF EACH FAT32 CLUSTER NUMBER PRINTED TO THE CONSOLE (i.e. 0xfffffff).
#define CHARACTERS_PER_FAT32_CLUSTER_NUMBER 9

// THE NUMBER OF FAT12 CLUSTERS THAT CAN FIT IN A ROW IN THE RIGHT COLUMN.
#define CLUSTERS_PER_ROW_FAT12              ((CHARACTERS_PER_ROW_RIGHT_COLUMN + 1) / (CHARACTERS_PER_FAT12_CLUSTER_NUMBER + 1))

// THE NUMBER OF FAT32 CLUSTERS THAT CAN FIT IN A ROW IN THE RIGHT COLUMN.
#define CLUSTERS_PER_ROW_FAT32              ((CHARACTERS_PER_ROW_RIGHT_COLUMN + 1) / (CHARACTERS_PER_FAT32_CLUSTER_NUMBER + 1))




/*
 * Prints a directory, or a directory tree.  Set the 'recursive' parameter to
 * 1 if you want the entire directory tree to be printed (from the given
 * directory, downward).  In this case, if the root directory is given, then
 * then every file and directory in the file system is printed.
 */
void printDirectory(file_t*      directory,
                    uint8_t      recursive,
                    boot_sect_t* bootSector,
                    uint32_t*    fileAllocationTable);




/*
 * Prints the header for the directory tree.
 */
void printDirectoryTreeHeader();




#endif

