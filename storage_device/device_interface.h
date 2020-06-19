/******************************************************************************
 * This file contains functions and data structures that operate on the
 *                               STORAGE DEVICE
 * 
 *
 * By Daniel Huettner
 *****************************************************************************/

#ifndef DEVICE_INTERFACE_H_
#define DEVICE_INTERFACE_H_




//
// INCLUDES
//

// LAYER 1: USER_INTERFACE
// (NOTHING)

// LAYER 2: FILE_SYSTEM
// (NOTHING)

// LAYER 3: STORAGE_DEVICE
// (NOTHING)

// ERROR HANDLING
#include "error.h"

// STANDARD C LIBRARY
#include <stdint.h>
#include <stdio.h>




//
// CONSTANTS
//

// THE DEFAULT SECTOR SIZE TO USE IF THE SECTOR SIZE IS NOT YET KNOWN (I.E. LIKE
// WHEN READING THE BOOT SECTOR).
#define DEFAULT_BYTES_PER_SECTOR 512




/*
 * Reads the specified sequence of sectors from the storage device.
 * The contents of the sectors is returned as one long array of bytes
 * in the buffer provided (this function does NOT allocate the buffer).
 * The sectors numbers in "sectorLocations" are the sectors to be read if
 * the value for "sectorsPerLocation" is 1.  If, however, the value for
 * "sectorsPerLocation" is more than one, then multiple sectors are read
 * in at a time, and the sector numbers in "sectorLocations" are the
 * first sectors to be read in for each chunk.
 */
uint8_t* readSectors(uint8_t*  buffer,
                     uint32_t* sectorLocations,
                     uint32_t  numLocations,
					 uint32_t  bytesPerSector,
					 uint32_t  sectorsPerLocation,
					 FILE*     storageDevice);




/*
 * Opens the specified storage device for reading.  The device is specified via
 * the absolute path of its device or image file.
 */
FILE* openStorageDevice(char* deviceFileName);




/*
 * Closes the storage device.
 */
void closeStorageDevice(FILE* storageDevice);




#endif

