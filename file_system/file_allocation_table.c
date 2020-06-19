/******************************************************************************
 * This file contains functions and data structures that operate on the
 *                        FILE ALLOCATION TABLE (FAT)
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
#include "file_allocation_table.h"
#include "file_system_tools.h"

// LAYER 3: STORAGE_DEVICE
#include "device_interface.h"

// ERROR HANDLING
#include "error.h"

// STANDARD C LIBRARY
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <wchar.h>




/*
 * Used to translate the raw file allocation table.
 */
uint32_t* translateFileAllocationTable(boot_sect_t* bootSector,
                                       uint8_t* fileAllocationTableRaw);


/*
 * A FAT12 helper function for translating the raw file allocation table.
 */
void translateFileAllocationTable_FAT12(boot_sect_t* bootSector,
                                        uint32_t* fileAllocationTable,
										uint8_t* fileAllocationTableRaw);


/*
 * A FAT32 helper function for translating the raw file allocation table.
 */
void translateFileAllocationTable_FAT32(boot_sect_t* bootSector,
                                        uint32_t* fileAllocationTable,
										uint8_t* fileAllocationTableRaw);




//
// IMPLEMENTATION OF THE FUNCTIONS DEFINED IN THE HEADER (.h) FILE.
//


uint32_t* getFileAllocationTable(boot_sect_t* bootSector,
                                 FILE* storageDevice) {

	//
	// PARAMETER CHECK.
	if (bootSector == NULL)
		handleError(L"getFileAllocationTable", L"NULL 'bootSector' parameter");
	if (storageDevice == NULL)
		handleError(L"getFileAllocationTable", L"NULL 'storageDevice' parameter");

	//
	// ALLOCATE THE BUFFER FOR THE RAW DATA.
	uint32_t numSectors = (getFatVersion(bootSector) == FAT12) ?
							bootSector->sectorsPerFAT_FAT12 :
							bootSector->sectorsPerFAT_FAT32;
	uint32_t bufferSize = bootSector->bytesPerSector * numSectors;
	uint8_t* buffer = (uint8_t*) malloc(bufferSize);
	if (buffer == NULL)
		handleError(L"getFileAllocationTable", L"Unable to allocate memory to read the file allocation table");

	//
	// GENERATE AN ARRAY OF SECTOR LOCATIONS TO BE READ IN.
	// THE LIST ONLY CONTAINS ONE ITEM -- THE STARTING SECTOR NUMBER OF THE FAT.
	// THE ARRAY IS NEEDED FOR THE readSectors FUNCTION.
	uint32_t* sectorNumber = (uint32_t*) malloc(sizeof(uint32_t));
	sectorNumber[0] = getSectorNumber_FileAllocationTable(bootSector);

	//
	// READ IN THE RAW FILE ALLOCATION TABLE.
	readSectors(buffer,
	            sectorNumber,
	            1,
	            bootSector->bytesPerSector,
	            getFatVersion(bootSector) == FAT12 ? bootSector->sectorsPerFAT_FAT12 :  bootSector->sectorsPerFAT_FAT32,
	            storageDevice);

	//
	// FREE THE SECTOR NUMBERS ARRAY WE CREATED FOR THE readSectors FUNCTION.
	free(sectorNumber);

	//
	// TRANSLATE THE RAW FILE ALLOCATION TABLE.
	uint32_t* fileAllocationTable =
			translateFileAllocationTable(bootSector, buffer);

	//
	// FREE THE RAW FILE ALLOCATION TABLE DATA.
	free(buffer);

	//
	// RETURN THE FILE ALLOCATION TABLE.
	return fileAllocationTable;

}


uint32_t getNumFATEntries(boot_sect_t* bootSector) {

	//
	// THE APPROACH TO DETERMINING THE NUMBER OF ENTRIES IN THE FAT TABLE
	// DIFFERS BETWEEN FAT12 AND FAT32.
	switch (getFatVersion(bootSector)) {

		case FAT12:
			return bootSector->numSectors_FAT12 / bootSector->sectorsPerCluster;
			break;

		case FAT32:
			return bootSector->numSectors_FAT32 / bootSector->sectorsPerCluster;
			break;

		default:
			return 0;

	}

}




//
// IMPLEMENTATION OF THE HELPER FUNCTIONS AND DATA STRUCTURES DEFINED ABOVE.
//


uint32_t* translateFileAllocationTable(boot_sect_t* bootSector,
                                       uint8_t* fileAllocationTableRaw) {

	//
	// CALCULATE HOW MUCH MEMORY TO ALLOCATE FOR THE ARRAY OF FAT ENTRIES.
	uint32_t  tableSize = getNumFATEntries(bootSector) * sizeof(uint32_t);

	//
	// ALLOCATE MEMORY FOR THE ARRAY OF FAT ENTRIES.
	uint32_t* fileAllocationTable =	(uint32_t*) malloc(tableSize);
	if (fileAllocationTable == NULL)
		handleError(L"translateFileAllocationTable",
					L"Unable to allocate memory for the file allocation table");

	//
	// TRANSLATE THE RAW DATA.
	// THE APPROACH TO TRANSLATING THE FAT ENTRIES DIFFERS BETWEEN FAT12
	// AND FAT32.
	switch (getFatVersion(bootSector)) {

		case FAT12:
			translateFileAllocationTable_FAT12(bootSector,
                                               fileAllocationTable,
											   fileAllocationTableRaw);
			break;

		case FAT32:
			translateFileAllocationTable_FAT32(bootSector,
                                               fileAllocationTable,
											   fileAllocationTableRaw);
			break;

	}

	//
	// RETURN THE ARRAY OF FAT ENTRIES.
	return fileAllocationTable;
	
}


void translateFileAllocationTable_FAT12(boot_sect_t* bootSector,
                                        uint32_t* fileAllocationTable,
										uint8_t* fileAllocationTableRaw) {

	//
	// DECODES THE UNUSUAL 12-BIT FAT ENTRIES.
	uint32_t byteNumber  = 0;
	uint32_t entryNumber = 0;
	uint32_t combined24BitValue;
	uint32_t numFATEntries = getNumFATEntries(bootSector);
	while (entryNumber < numFATEntries) {
		combined24BitValue = (uint32_t)
							 (((uint32_t) fileAllocationTableRaw[byteNumber+2]) << 16) |
							 (((uint32_t) fileAllocationTableRaw[byteNumber+1]) <<  8) |
							 (((uint32_t) fileAllocationTableRaw[byteNumber+0]) <<  0);
		fileAllocationTable[entryNumber]   = (uint32_t) (combined24BitValue % 4096);
		fileAllocationTable[entryNumber+1] = (uint32_t) (combined24BitValue / 4096);
		
		entryNumber = entryNumber + 2;
		byteNumber  = byteNumber  + 3;
	}

}


void translateFileAllocationTable_FAT32(boot_sect_t* bootSector,
                                        uint32_t* fileAllocationTable,
										uint8_t* fileAllocationTableRaw) {

	//
	// DECODES THE 32-BIT FAT ENTRIES.
	uint32_t byteNumber  = 0;
	uint32_t entryNumber = 0;
	uint32_t numFATEntries = getNumFATEntries(bootSector);
	while (entryNumber < numFATEntries) {
		fileAllocationTable[entryNumber] = (uint32_t)
					(((uint32_t) fileAllocationTableRaw[byteNumber+3] & 0b00001111) << 24) |
					(((uint32_t) fileAllocationTableRaw[byteNumber+2] & 0b11111111) << 16) |
					(((uint32_t) fileAllocationTableRaw[byteNumber+1] & 0b11111111) <<  8) |
					(((uint32_t) fileAllocationTableRaw[byteNumber+0] & 0b11111111) <<  0);
		
		entryNumber = entryNumber + 1;
		byteNumber  = byteNumber  + 4;
	}

}

