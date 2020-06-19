/******************************************************************************
 * This file contains functions and data structures that pertain to the
 *                                FILE SYSTEM
 * 
 *
 * By Daniel Huettner
 *****************************************************************************/

#ifndef FILE_SYSTEM_TOOLS_H_
#define FILE_SYSTEM_TOOLS_H_




//
// INCLUDES
//

// LAYER 1: USER_INTERFACE
// (NOTHING)

// LAYER 2: FILE_SYSTEM
#include "boot_sector.h"
#include "file_allocation_table.h"
#include "directory.h"

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

#define FAT12 12
#define FAT16 16
#define FAT32 32




/*
 * Returns the FAT version (FAT12, FAT16, or FAT32).
 */
uint32_t getFatVersion(boot_sect_t* bootSector);




/*
 * Returns the first sector number of the file allocation table.
 */
uint32_t getSectorNumber_FileAllocationTable(boot_sect_t* bootSector);




/*
 * Returns the first sector number of the root directory.
 */
uint32_t getSectorNumber_RootDirectory(boot_sect_t* bootSector);




/*
 * Returns the first sector number of the specified cluster number.
 * If the cluster number is invalid, this function will return 0.
 */
uint32_t getSectorNumber_DataCluster(boot_sect_t* bootSector,
									 uint32_t clusterNumber);




/*
 * Reads the specified sequence of clusters from the storage device.
 * The contents of the clusters is returned as one long array of bytes.
 */
uint8_t* readClusters(uint8_t*     buffer,
                      uint32_t*    clusterNumbers,
                      uint32_t     numClusters,
					  boot_sect_t* bootSector,
					  FILE*        storageDevice);




/*
 * Determines the cluster sequence starting from the given cluster using the
 * file allocation table provided.
 *
 * THIS RETURNS THE CLUSTER *NUMBERS*, NOT THE CLUSTER CONTENTS!!
 *
 * NOTE: The number of clusters in the result is saved in the numClusters
 *       parameter.  To call this function, first create a new uint32_t
 *       variable:
 *           uint32_t numClusters;
 *       then pass it to the function as a pointer:
 *           getClusterSequence(..., ..., &numClusters);
 */
uint32_t* getClusterSequence(uint32_t  firstCluster,
                             boot_sect_t* bootSector,
                             uint32_t* fileAllocationTable,
							 uint32_t* numClusters);




/*
 * Returns a 32-bit unsigned integer containing the unsigned translation of the
 * value who's bytes were arranged in little-endian order.
 */
uint32_t translateLittleEndian(uint8_t* byteArray, uint8_t length);




/*
 * Get the absolute pathname of the file.
 */
wchar_t* getAbsolutePathName(file_t* file);




#endif

