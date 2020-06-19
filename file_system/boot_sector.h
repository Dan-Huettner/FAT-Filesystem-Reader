/******************************************************************************
 * This file contains functions and data structures that operate on the
 *                                BOOT SECTOR
 * of a FAT filesystem.
 *
 * By Daniel Huettner
 *****************************************************************************/

#ifndef BOOT_SECTOR_H_
#define BOOT_SECTOR_H_




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
// (NOTHING)

// STANDARD C LIBRARY
#include <stdint.h>
#include <stdio.h>




/*
 * A data structure used to store the boot sector contents.
 * Elements that are specifically FAT12 end with "_FAT12", and those that are
 * specifically FAT32 end with "_FAT32".
 */
typedef struct {

	uint8_t  jumpCode[3];                               // Jump to bootstrap
	char     oemName[8];                                // OEM name/version
	uint32_t bytesPerSector;                            // Number of bytes per sector
	uint32_t sectorsPerCluster;                         // Number of sectors per cluster
	uint32_t numReservedSectors;                        // Number of reserved sectors
	uint32_t numFATs;                                   // Number of file allocation tables
	uint32_t numRootEntries_FAT12;                      // Number of Root directory entries (FAT12 only)
	uint32_t numSectors_FAT12;                          // Number of sectors in the file system (FAT12 only)
	uint32_t mediaDescriptorType;                       // Media descriptor type
	uint32_t sectorsPerFAT_FAT12;                       // Number of sectors per FAT (FAT12 only)
	uint32_t sectorsPerTrack;                           // Number of sectors per track
	uint32_t numHeads;                                  // Number of heads
	uint32_t numHiddenSectors;                          // Number of sectors before FS partition (FAT12 only uses first 2 bytes)
	uint32_t numSectors_FAT32;                          // Number of sectors in the file system (FAT32 only)
	uint32_t sectorsPerFAT_FAT32;                       // Number of sectors per FAT (FAT32 only)
	uint32_t mirrorFlags_FAT32;                         // Ignore this (FAT32 only)
	uint32_t filesystemVersion_FAT32;                   // Ignore this (FAT32 only)
	uint32_t rootClusterNumber_FAT32;                   // The first cluster of the root directory (FAT32 only)
	uint32_t filesystemInformationSectorNumber_FAT32;   // Ignore this (FAT32 only)
	uint32_t backupBootSectorNumber_FAT32;              // Ignore this (FAT32 only)
	uint8_t  reserved1_FAT32[12];                       // Ignore this (FAT32 only)
	uint32_t driveNumber_FAT32;                         // Ignore this (FAT32 only)
	uint8_t  reserved2_FAT32[1];                        // Ignore this (FAT32 only)
	uint32_t extendedSignature_FAT32;                   // Ignore this (FAT32 only)
	uint32_t partitionSerialNumber_FAT32;               // Partition serial number (FAT32 only)
	char     volumeLabel_FAT32[11];                     // Volume label (FAT32 only)
	char     filesystemType_FAT32[8];                   // "FAT32   " (FAT32 only)
	uint8_t  restOfBootSector[422];                     // Ignore this

} boot_sect_t;




/*
 * Extracts the file system's boot sector, and stores the information in a
 * boot_sect_t data structure.
 */
boot_sect_t* getBootSector(FILE* storageDevice);




/*
 * Prints the boot sector contents to the console.
 * Useful for debugging purposes.
 */
void printBootSector(boot_sect_t* bootSector);




#endif

