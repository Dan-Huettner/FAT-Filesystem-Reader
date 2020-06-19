/******************************************************************************
 * This file contains functions and data structures that operate on the
 *                        FILE ALLOCATION TABLE (FAT)
 * of a FAT filesystem.
 *
 * By Daniel Huettner
 *****************************************************************************/

#ifndef FILE_ALLOCATION_TABLE_H_
#define FILE_ALLOCATION_TABLE_H_




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




/*
 * Extracts the entries from the file allocation table and stores them in an
 * array of integers, the returns the array.
 */
uint32_t* getFileAllocationTable(boot_sect_t* bootSector,
                                 FILE* storageDevice);




/*
 * Returns the number of file allocation table entries, which is equal to the
 * total number of clusters on the storage device.
 */
uint32_t getNumFATEntries(boot_sect_t* bootSector);




#endif

