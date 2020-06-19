/******************************************************************************
 * This file contains functions and data structures that operate on the
 *                                BOOT SECTOR
 * of a FAT filesystem.
 *
 * By Daniel Huettner
 *****************************************************************************/




//
// INCLUDES
//

// LAYER 1: USER_INTERFACE
// (NOTHING)

// LAYER 2: FILE_SYSTEM
#include "boot_sector.h"
#include "file_system_tools.h"

// LAYER 3: STORAGE_DEVICE
#include "device_interface.h"

// ERROR HANDLING
#include "error.h"

// STANDARD C LIBRARY
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <wchar.h>





/*
 * Used to help parse the raw boot sector data.
 */
typedef struct boot_sector_raw_t boot_sector_raw_t;


/*
 * Used to read in the raw boot sector data.
 */
boot_sector_raw_t* readBootSector(boot_sector_raw_t* bootSectorRaw, FILE* storageDevice);


/*
 * Used to translate the raw boot sector data.
 */
boot_sect_t* parseBootSector(boot_sector_raw_t* bootSectorRaw);




//
// IMPLEMENTATION OF THE FUNCTIONS DEFINED IN THE HEADER (.h) FILE.
//


boot_sect_t* getBootSector(FILE* storageDevice) {

	//
	// PARAMETER CHECK.
	if (storageDevice == NULL)
		handleError(L"getBootSector", L"NULL 'storageDevice' parameter");

	//
	// READ IN THE BOOT SECTOR VIA THE STORAGE DEVICE INTERFACE.
	boot_sector_raw_t* bootSectorRaw = (boot_sector_raw_t*) malloc(DEFAULT_BYTES_PER_SECTOR);
	readBootSector(bootSectorRaw, storageDevice);
	if (bootSectorRaw == NULL)
		handleError(L"getBootSector", L"Unable to read boot sector from file");

	//
	// TRANSLATE THE RAW BOOT SECTOR.
	boot_sect_t* bootSector = parseBootSector(bootSectorRaw);
	if (bootSector == NULL)
		handleError(L"getBootSector", L"Unable to process raw boot sector contents");
	free (bootSectorRaw);

	//
	// RETURN THE BOOT SECTOR DATA STRUCTURE.
	return bootSector;

}


void printBootSector(boot_sect_t* bootSector) {

	wprintf(L"%ls	%ls\n", L"jumpCode", bootSector->jumpCode);
	wprintf(L"%ls	%ls\n", L"oemName", bootSector->oemName);
	wprintf(L"%ls	%u\n", L"bytesPerSector", bootSector->bytesPerSector);
	wprintf(L"%ls	%u\n", L"sectorsPerCluster", bootSector->sectorsPerCluster);
	wprintf(L"%ls	%u\n", L"numReservedSectors", bootSector->numReservedSectors);
	wprintf(L"%ls	%u\n", L"numFATs", bootSector->numFATs);
	wprintf(L"%ls	%u\n", L"numRootEntries_FAT12", bootSector->numRootEntries_FAT12);
	wprintf(L"%ls	%u\n", L"numSectors_FAT12", bootSector->numSectors_FAT12);
	wprintf(L"%ls	%u\n", L"mediaDescriptorType", bootSector->mediaDescriptorType);
	wprintf(L"%ls	%u\n", L"sectorsPerFAT_FAT12", bootSector->sectorsPerFAT_FAT12);
	wprintf(L"%ls	%u\n", L"sectorsPerTrack", bootSector->sectorsPerTrack);
	wprintf(L"%ls	%u\n", L"numHeads", bootSector->numHeads);
	wprintf(L"%ls	%u\n", L"numHiddenSectors", bootSector->numHiddenSectors);
	wprintf(L"%ls	%u\n", L"numSectors_FAT32", bootSector->numSectors_FAT32);
	wprintf(L"%ls	%u\n", L"sectorsPerFAT_FAT32", bootSector->sectorsPerFAT_FAT32);
	wprintf(L"%ls	%u\n", L"mirrorFlags_FAT32", bootSector->mirrorFlags_FAT32);
	wprintf(L"%ls	%u\n", L"filesystemVersion_FAT32", bootSector->filesystemVersion_FAT32);
	wprintf(L"%ls	%u\n", L"rootClusterNumber_FAT32", bootSector->rootClusterNumber_FAT32);
	wprintf(L"%ls	%u\n", L"filesystemInformationSectorNumber_FAT32", bootSector->filesystemInformationSectorNumber_FAT32);
	wprintf(L"%ls	%u\n", L"backupBootSectorNumber_FAT32", bootSector->backupBootSectorNumber_FAT32);
	wprintf(L"%ls	%ls\n", L"reserved1_FAT32", bootSector->reserved1_FAT32);
	wprintf(L"%ls	%u\n", L"driveNumber_FAT32", bootSector->driveNumber_FAT32);
	wprintf(L"%ls	%u\n", L"reserved2_FAT32", bootSector->reserved2_FAT32[0]);
	wprintf(L"%ls	%u\n", L"extendedSignature_FAT32", bootSector->extendedSignature_FAT32);
	wprintf(L"%ls	%u\n", L"partitionSerialNumber_FAT32", bootSector->partitionSerialNumber_FAT32);
	wprintf(L"%ls	%ls\n", L"volumeLabel_FAT32", bootSector->volumeLabel_FAT32);
	wprintf(L"%ls	%ls\n", L"filesystemType_FAT32", bootSector->filesystemType_FAT32);

}




//
// IMPLEMENTATION OF THE HELPER FUNCTIONS AND DATA STRUCTURES DEFINED ABOVE.
//


/*
 * A data structure used to store the *RAW* boot sector contents.
 * This is only used when reading in the raw boot sector data, and it is
 * translated into the boot sector struct defined above then discarded.
 */
struct boot_sector_raw_t {

	uint8_t jumpCode[3];                                // Jump to bootstrap.
	char    oemName[8];                                 // OEM name/version.
	uint8_t bytesPerSector[2];                          // Number of bytes per sector
	uint8_t sectorsPerCluster[1];                       // Number of sectors per cluster
	uint8_t numReservedSectors[2];                      // Number of reserved sectors
	uint8_t numFATs[1];                                 // Number of file allocation tables
	uint8_t numRootEntries_FAT12[2];                    // Number of Root directory entries (FAT12 only)
	uint8_t numSectors_FAT12[2];                        // Number of sectors in the file system (FAT12 only)
	uint8_t mediaDescriptorType[1];                     // Media descriptor type
	uint8_t sectorsPerFAT_FAT12[2];                     // Number of sectors per FAT (FAT12 only)
	uint8_t sectorsPerTrack[2];                         // Number of sectors per track
	uint8_t numHeads[2];                                // Number of heads
	uint8_t numHiddenSectors[4];                        // Number of sectors before FS partition (FAT12 only uses first 2 bytes)
	uint8_t numSectors_FAT32[4];                        // Number of sectors in the file system (FAT32 only)
	uint8_t sectorsPerFAT_FAT32[4];                     // Number of sectors per FAT (FAT32 only)
	uint8_t mirrorFlags_FAT32[2];                       // Ignore this (FAT32 only)
	uint8_t filesystemVersion_FAT32[2];                 // Ignore this (FAT32 only)
	uint8_t rootClusterNumber_FAT32[4];                 // The first cluster of the root directory (FAT32 only)
	uint8_t filesystemInformationSectorNumber_FAT32[2]; // Ignore this (FAT32 only)
	uint8_t backupBootSectorNumber_FAT32[2];            // Ignore this (FAT32 only)
	uint8_t reserved1_FAT32[12];                        // Ignore this (FAT32 only)
	uint8_t driveNumber_FAT32[1];                       // Ignore this (FAT32 only)
	uint8_t reserved2_FAT32[1];                         // Ignore this (FAT32 only)
	uint8_t extendedSignature_FAT32[1];                 // Ignore this (FAT32 only)
	uint8_t partitionSerialNumber_FAT32[4];             // Partition serial number (FAT32 only)
	char    volumeLabel_FAT32[11];                      // Volume label (FAT32 only)
	char    filesystemType_FAT32[8];                    // L"FAT32   " (FAT32 only)
	uint8_t restOfBootSector[422];                      // Ignore this

};


boot_sector_raw_t* readBootSector(boot_sector_raw_t* bootSectorRaw, FILE* storageDevice) {

	uint32_t* sectorLocation = (uint32_t*) malloc(sizeof(uint32_t));
	sectorLocation[0] = 0;

	readSectors((uint8_t*) bootSectorRaw,
                sectorLocation,
                1,
                DEFAULT_BYTES_PER_SECTOR,
                1,
                storageDevice);
	free(sectorLocation);

	//
	// RETURN THE RAW BOOT SECTOR.
	return bootSectorRaw;

}


boot_sect_t* parseBootSector(boot_sector_raw_t* bootSectorRaw) {

	//
	// ALLOCATE MEMORY FOR THE NEW STRUCT.
	boot_sect_t* bootSector = (boot_sect_t*) malloc(sizeof(boot_sect_t));

	//
	// TRANSLATE VALUES.
	memcpy(bootSector->jumpCode, bootSectorRaw->jumpCode, 3);
	memcpy(bootSector->oemName, bootSectorRaw->oemName, 8);
	bootSector->bytesPerSector = translateLittleEndian(bootSectorRaw->bytesPerSector, 2);
	bootSector->sectorsPerCluster = translateLittleEndian(bootSectorRaw->sectorsPerCluster, 1);
	bootSector->numReservedSectors = translateLittleEndian(bootSectorRaw->numReservedSectors, 2);
	bootSector->numFATs = translateLittleEndian(bootSectorRaw->numFATs, 1);
	bootSector->numRootEntries_FAT12 = translateLittleEndian(bootSectorRaw->numRootEntries_FAT12, 2);
	bootSector->numSectors_FAT12 = translateLittleEndian(bootSectorRaw->numSectors_FAT12, 2);
	bootSector->mediaDescriptorType = translateLittleEndian(bootSectorRaw->mediaDescriptorType, 1);
	bootSector->sectorsPerFAT_FAT12 = translateLittleEndian(bootSectorRaw->sectorsPerFAT_FAT12, 2);
	bootSector->sectorsPerTrack = translateLittleEndian(bootSectorRaw->sectorsPerTrack, 2);
	bootSector->numHeads = translateLittleEndian(bootSectorRaw->numHeads, 2);
	bootSector->numHiddenSectors = translateLittleEndian(bootSectorRaw->numHiddenSectors, 4);
	bootSector->numSectors_FAT32 = translateLittleEndian(bootSectorRaw->numSectors_FAT32, 4);
	bootSector->sectorsPerFAT_FAT32 = translateLittleEndian(bootSectorRaw->sectorsPerFAT_FAT32, 4);
	bootSector->mirrorFlags_FAT32 = translateLittleEndian(bootSectorRaw->mirrorFlags_FAT32, 2);
	bootSector->filesystemVersion_FAT32 = translateLittleEndian(bootSectorRaw->filesystemVersion_FAT32, 2);
	bootSector->rootClusterNumber_FAT32 = translateLittleEndian(bootSectorRaw->rootClusterNumber_FAT32, 4);
	bootSector->filesystemInformationSectorNumber_FAT32 = translateLittleEndian(bootSectorRaw->filesystemInformationSectorNumber_FAT32, 2);
	bootSector->backupBootSectorNumber_FAT32 = translateLittleEndian(bootSectorRaw->backupBootSectorNumber_FAT32, 2);
	memcpy(bootSector->reserved1_FAT32, bootSectorRaw->reserved1_FAT32, 12);
	bootSector->driveNumber_FAT32 = translateLittleEndian(bootSectorRaw->driveNumber_FAT32, 1);
	memcpy(bootSector->reserved2_FAT32, bootSectorRaw->reserved2_FAT32, 1);
	bootSector->extendedSignature_FAT32 = translateLittleEndian(bootSectorRaw->extendedSignature_FAT32, 1);
	bootSector->partitionSerialNumber_FAT32 = translateLittleEndian(bootSectorRaw->partitionSerialNumber_FAT32, 4);
	memcpy(bootSector->volumeLabel_FAT32, bootSectorRaw->volumeLabel_FAT32, 11);
	memcpy(bootSector->filesystemType_FAT32, bootSectorRaw->filesystemType_FAT32, 8);
	memcpy(bootSector->restOfBootSector, bootSectorRaw->restOfBootSector, 422);

	//
	// RETURN THE NEW STRUCT.
	return bootSector;

}

