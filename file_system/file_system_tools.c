/******************************************************************************
 * This file contains functions and data structures that pertain to the
 *                                FILE SYSTEM
 * 
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
#include "directory.h"
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
 * Used to determine the number of clusters in a sequence.
 */
uint32_t getClusterSequenceLength(uint32_t firstCluster,
                                  boot_sect_t* bootSector,
								  uint32_t* fileAllocationTable);


/*
 * Used to determine if an entry in the file allocation table indicates whether
 * or not the given cluster is the last cluster in the sequence.
 */
uint8_t hasNext(uint32_t clusterNumber,
				boot_sect_t* bootSector,
				uint32_t* fileAllocationTable);


/*
 * Used to determine if a given cluster number is valid.
 */
uint8_t isValidClusterNumber(uint32_t clusterNumber,
							 boot_sect_t* bootSector);


/*
 * Recursive helper function for getAbsolutePathName.
 */
void getAbsolutePathNameRecursive(file_t* file, wchar_t* absolutePathName);




//
// IMPLEMENTATION OF THE FUNCTIONS DEFINED IN THE HEADER (.h) FILE.
//


uint32_t translateLittleEndian(uint8_t* byteArray, uint8_t length) {

	// THE FOLLOWING WILL TRANSLATE VALUES STORED IN LITTLE ENDIAN ORDER, FOR
	// VALUES RANGING FROM 1 BYTE TO 4 BYTES IN LENGTH.
	switch (length) {
		case 1:
			return   (uint32_t) byteArray[0];
			break;
		case 2:
			return (((uint32_t) byteArray[1]) <<  8) |
                   (((uint32_t) byteArray[0]) <<  0);
			break;
		case 3:
			return (((uint32_t) byteArray[2]) << 16) |
                   (((uint32_t) byteArray[1]) <<  8) |
                   (((uint32_t) byteArray[0]) <<  0);
			break;
		case 4:
			return (((uint32_t) byteArray[3]) << 24) |
                   (((uint32_t) byteArray[2]) << 16) |
                   (((uint32_t) byteArray[1]) <<  8) |
                   (((uint32_t) byteArray[0]) <<  0);
			break;
		default:
			return 0;
	}

}


uint32_t getFatVersion(boot_sect_t* bootSector) {

	//
	// THE FOLLOWING IS HOW MICROSOFT DISTINGUISHES BETWEEN FAT12, FAT16, & FAT32.
	//

	//
	// PART 1: COMPUTE THE NUMBER OF CLUSTERS IN THE DATA AREA.
	//

	//
	// PART 1a: GET THE TOTAL NUMBER OF SECTORS ON DISK.
	uint32_t numSectorsTotal = bootSector->numSectors_FAT12;
	if (numSectorsTotal == 0)
		numSectorsTotal = bootSector->numSectors_FAT32;

	//
	// PART 1b: GET THE NUMBER OF SECTORS PER FAT.
	uint32_t sectorsPerFAT = bootSector->sectorsPerFAT_FAT12;
	if (sectorsPerFAT == 0)
		sectorsPerFAT = bootSector->sectorsPerFAT_FAT32;

	//
	// PART 1c: GET THE NUMBER OF RESERVED SECTORS.
	uint32_t numSectorsReserved = bootSector->numReservedSectors;

	//
	// PART 1d: GET THE NUMBER OF SECTORS FOR ALL FATS.
	uint32_t numSectorsFATs = bootSector->numFATs * sectorsPerFAT;

	//
	// PART 1e: GET THE NUMBER OF SECTORS FOR ROOT DIRECTORY.
	uint32_t numSectorsRoot = (bootSector->numRootEntries_FAT12 * 32)
							/  bootSector->bytesPerSector;

	//
	// PART 1f: COMPUTE THE NUMBER OF SECTORS IN DATA AREA.
	uint32_t numSectorsData = numSectorsTotal
							- numSectorsReserved
							- numSectorsFATs
							- numSectorsRoot;

	//
	// PART 1g: COMPUTE THE NUMBER OF CLUSTERS IN DATA AREA.
	uint32_t numClustersData = numSectorsData / bootSector->sectorsPerCluster;

	//
	// PART 2: USE THE VALUE FROM PART 1 TO DETERMINE THE FAT VERSION.
	//

	//
	// IF THE RESULT IS LESS THAN 4085, THEN IT IS FAT12.
	if (numClustersData < 4085)
		return FAT12;
		
	//
	// IF THE RESULT IF GREATER THAN OR EQUAL TO 4085,
	// BUT LESS THAN 65525, THEN IT IS FAT16.
	if (numClustersData < 65525)
		return FAT16;
	
	//
	// OTHERWISE, IT IS FAT32.
	return FAT32;
		
}


uint32_t* getClusterSequence(uint32_t  firstCluster,
                             boot_sect_t* bootSector,
                             uint32_t* fileAllocationTable,
                             uint32_t* numClusters) {


	//
	// DETERMINE THE NUMBER OF CLUSTERS IN THE SEQUENCE.
	*numClusters = getClusterSequenceLength(firstCluster,
                                            bootSector,
	                                        fileAllocationTable);

	//
	// CREATE AN EMPTY ARRAY TO STORE THE SEQUENCE OF CLUSTER NUMBERS.
	uint32_t* clusterSequence =
					(uint32_t*) malloc(sizeof(uint32_t) * (*numClusters));
	if (clusterSequence == NULL)
		handleError(L"getClusterSequence",
		            L"Unable to allocate memory for a cluster number sequence");

	//
	// GETTING THE SEQUENCE.
	uint32_t clusterCount  = 0;
	uint32_t clusterNumber = firstCluster;
	while (clusterCount < (*numClusters)) {

		//
		// SAVE CURRENT CLUSTER NUMBER TO SEQUENCE ARRAY.
		clusterSequence[clusterCount] = clusterNumber;

		//
		// GET NEXT CLUSTER NUMBER.
		if (clusterCount < (*numClusters) - 1)
			clusterNumber = fileAllocationTable[clusterNumber];

		//
		// INCREMENT THE CLUSTER COUNT.
		clusterCount++;
		
	}

	//
	// RETURN THE LIST OF CLUSTER NUMBERS.
	return clusterSequence;

}



uint32_t getSectorNumber_FileAllocationTable(boot_sect_t* bootSector) {

	//
	// DETERMINE THE FIRST SECTOR OF THE FILE ALLOCATION TABLE.
	// THIS IS THE SAME FOR BOTH FAT12 AND FAT32 FILE SYSTEMS.
	return bootSector->numReservedSectors;

}


uint32_t getSectorNumber_RootDirectory(boot_sect_t* bootSector) {

	//
	// DETERMINE THE FIRST SECTOR OF THE ROOT DIRECTORY.
	// THIS DIFFERS BETWEEN FAT12 AND FAT32 FILE SYSTEMS.
	switch (getFatVersion(bootSector)) {

		case FAT12:
			return getSectorNumber_FileAllocationTable(bootSector)
				 + (bootSector->sectorsPerFAT_FAT12 * bootSector->numFATs);
			break;

		case FAT32:
			return getSectorNumber_FileAllocationTable(bootSector)
				 + (bootSector->sectorsPerFAT_FAT32 * bootSector->numFATs)
				 + (bootSector->sectorsPerCluster * (bootSector->rootClusterNumber_FAT32 - 2));
			break;

		default:
			return 0;

	}

}


uint32_t getSectorNumber_DataCluster(boot_sect_t* bootSector,
									 uint32_t clusterNumber) {

	//
	// CHECK FOR INVALID CLUSTER NUMBERS.
	if (!isValidClusterNumber(clusterNumber, bootSector))
		handleError(L"getSectorNumber_DataCluster",
		            L"Request made for the sector number of an invalid cluster number");

	//
	// DETERMINE THE FIRST SECTOR IN DATA AREA.
	// THIS DIFFERS BETWEEN FAT12 AND FAT32 FILE SYSTEMS.
	uint32_t firstSectorInDataArea = 0;
	switch (getFatVersion(bootSector)) {

		case FAT12:
			firstSectorInDataArea = getSectorNumber_FileAllocationTable(bootSector)
							+ (bootSector->sectorsPerFAT_FAT12 * bootSector->numFATs)
							+ ((bootSector->numRootEntries_FAT12 * 32) / bootSector->bytesPerSector);
			break;

		case FAT32:
			firstSectorInDataArea = getSectorNumber_FileAllocationTable(bootSector)
							+ (bootSector->sectorsPerFAT_FAT32 * bootSector->numFATs);
			break;

		default:
			return 0;

	}

	uint32_t sectorNumber = firstSectorInDataArea
						  + (clusterNumber - 2) * bootSector->sectorsPerCluster;

	return sectorNumber;

}


uint8_t* readClusters(uint8_t*     buffer,
                      uint32_t*    clusterNumbers,
                      uint32_t     numClusters,
					  boot_sect_t* bootSector,
					  FILE*        storageDevice) {

	//
	// GET THE LOCATION OF THE FIRST SECTOR FOR EACH CLUSTER IN THE SEQUENCE.
	uint32_t* sectorLocations = (uint32_t*) malloc(numClusters * sizeof(uint32_t));
	uint32_t  clusterCount = 0;
	while (clusterCount < numClusters) {
		sectorLocations[clusterCount] =
			getSectorNumber_DataCluster(bootSector, clusterNumbers[clusterCount]);
		clusterCount++;
	}

	//
	// READ IN THE CLUSTERS.
	readSectors((uint8_t*) buffer,
	            sectorLocations,
	            numClusters,
	            bootSector->bytesPerSector,
	            bootSector->sectorsPerCluster,
	            storageDevice);

	free(sectorLocations);

	//
	// RETURN THE BUFFER.
	return buffer;

}


wchar_t* getAbsolutePathName(file_t* file) {

	//
	// FIRST NEED TO CALCULATE THE LENGTH.
	uint32_t length = wcslen(file->name);
	file_t* ancestor = file->parentDirectory;
	while (ancestor != NULL) {
		length += wcslen(ancestor->name);
		length += 1; // ADDING THE "/"
		ancestor = ancestor->parentDirectory;
	}
	length = length + 1; // TERMINATING NULL CHARACTER.

	//
	// ALLOCATE MEMORY FOR THE STRING.
	wchar_t* absolutePathName = (wchar_t*) calloc(length, sizeof(wchar_t));

	//
	// CALL THE RECURSIVE HELPER FUNCTION.
	getAbsolutePathNameRecursive(file, absolutePathName);

	//
	// RETURN THE RESULT.
	return absolutePathName;

}



//
// IMPLEMENTATION OF THE HELPER FUNCTIONS AND DATA STRUCTURES DEFINED ABOVE.
//


uint32_t getClusterSequenceLength(uint32_t     firstCluster,
                                  boot_sect_t* bootSector,
								  uint32_t*    fileAllocationTable) {

	//
	// CHECKING IF THE FILE IS EMPTY OR OTHERWISE INVALID.
	if (!isValidClusterNumber(firstCluster, bootSector))
		return 0;

	//
	// THERE IS AT LEAST ONE VALID CLUSTER IN THE SEQUENCE (THE FIRST CLUSTER).
	uint32_t numClusters = 1;

	//
	// MOVE FROM ONE CLUSTER TO THE NEXT (USING THE FILE ALLOCATION TABLE)
	// AND COUNT THE TOTAL NUMBER OF CLUSTERS IN THE SEQUENCE.
	uint32_t currentCluster = firstCluster;
	while (hasNext(currentCluster, bootSector, fileAllocationTable)) {	
		numClusters++;
		currentCluster = fileAllocationTable[currentCluster];
	}
	
	//
	// RETURN THE NUMBER OF CLUSTERS IN THE SEQUENCE.
	return numClusters;
	
}


uint8_t hasNext(uint32_t clusterNumber,
				boot_sect_t* bootSector,
				uint32_t* fileAllocationTable) {

	//
	// CHECK IF THE CLUSTER NUMBER IS INVALID.  IF SO, THEN THERE IS NOT A
	// NEXT CLUSTER.
	if (!isValidClusterNumber(clusterNumber, bootSector))
		return 0;

	//
	// OTHERWISE, GET THE NEXT CLUSTER NUMBER, AND RETURN TRUE IF IT IS VALID.
	uint32_t nextCluster = fileAllocationTable[clusterNumber];
	return isValidClusterNumber(nextCluster, bootSector);

}


uint8_t isValidClusterNumber(uint32_t clusterNumber, boot_sect_t* bootSector) {

	//
	// DETERMINE WHETHER OR NOT THE GIVEN CLUSTER NUMBER IS VALID.
	// THIS DIFFERS BETWEEN FAT12 AND FAT32 FILE SYSTEMS.
	switch (getFatVersion(bootSector)) {

		case FAT12:
			if (clusterNumber != 0x000 &&
				clusterNumber != 0x001 &&
				clusterNumber != 0xff8 &&
				clusterNumber != 0xff9 &&
				clusterNumber != 0xffa &&
				clusterNumber != 0xffb &&
				clusterNumber != 0xffc &&
				clusterNumber != 0xffd &&
				clusterNumber != 0xffe &&
				clusterNumber != 0xfff)
				return 1;
			else
				return 0;
			break;

		case FAT32:
			if (clusterNumber >= bootSector->rootClusterNumber_FAT32 &&
				clusterNumber != 0x00000ff8 &&
				clusterNumber != 0x00000ff9 &&
				clusterNumber != 0x00000ffa &&
				clusterNumber != 0x00000ffb &&
				clusterNumber != 0x00000ffc &&
				clusterNumber != 0x00000ffd &&
				clusterNumber != 0x00000ffe &&
				clusterNumber != 0x00000fff &&
				clusterNumber != 0x0000fff8 &&
				clusterNumber != 0x0000fff9 &&
				clusterNumber != 0x0000fffa &&
				clusterNumber != 0x0000fffb &&
				clusterNumber != 0x0000fffc &&
				clusterNumber != 0x0000fffd &&
				clusterNumber != 0x0000fffe &&
				clusterNumber != 0x0000ffff &&
				clusterNumber != 0x0fffffff)
				return 1;
			else
				return 0;
			break;

		default:
			return 0;

	}
	
}


void getAbsolutePathNameRecursive(file_t* file, wchar_t* absolutePathName) {

	//
	// BASE CASE.
	if (file == NULL || wcscmp(file->name, L"") == 0)
		return;

	//
	// RECURSIVE CALL.
	getAbsolutePathNameRecursive(file->parentDirectory, absolutePathName);

	//
	// CONCATENATE "/" AND THIS DIRECTORY'S NAME TO THE ABSOLUTE PATH.
	wcscat(absolutePathName, L"/");
	wcscat(absolutePathName, file->name);

}

