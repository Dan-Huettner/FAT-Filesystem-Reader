/******************************************************************************
 * This file contains functions and data structures that operate on the
 *                                DIRECTORIES
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
#include "directory.h"
#include "file_allocation_table.h"
#include "file_system_tools.h"

// LAYER 3: STORAGE_DEVICE
#include "device_interface.h"

// ERROR HANDLING
#include "error.h"

// STANDARD C LIBRARY
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <wchar.h>




/*
 * Used to help extract directory entry contents from the raw data.
 */
typedef struct directory_entry_raw_t directory_entry_raw_t;


/*
 * A recursive helper function for getting the directory tree.  This function
 * can start from any directory.
 *
 * The children of the given directory are expected to have at least the
 * following information:
 *     - name
 *     - type
 *     - size
 *     - cluster sequence
 */
void getDirectoryTreeRecursive(file_t* root,
                               boot_sect_t* bootSector,
                               uint32_t*    fileAllocationTable,
                               FILE*        storageDevice);


/*
 * Used to parse the raw directory entries.
 * The variable numEntries will contain the number of directory entries when
 * the function returns.
 */
file_t* parseDirectoryEntries(directory_entry_raw_t* directoryEntriesRaw,
							  uint32_t* numEntries,
							  uint32_t maxDirectoryEntries,
							  uint32_t* fileAllocationTable,
							  boot_sect_t* bootSector);


/*
 * Used to parse a raw directory entry.
 */
void parseDirectoryEntry(file_t* directoryEntry,
                         directory_entry_raw_t* directoryEntryRaw,
						 uint32_t* fileAllocationTable,
                         boot_sect_t* bootSector);


/*
 * Used to parse a series of raw VFAT directory entries.
 */
void parseDirectoryEntry_VFAT(file_t* directoryEntry,
							 directory_entry_raw_t* vfatRawEntrySequence,
							 uint32_t vfatSequenceCount,
							 uint32_t* fileAllocationTable,
							 boot_sect_t* bootSector);


/*
 * Used to extract the entry name from the raw directory entry.
 */
void extractEntryName(file_t* directoryEntry,
					  directory_entry_raw_t* directoryEntryRaw);


/*
 * Used to extract the entry name from a series of raw VFAT entries.
 */
void extractEntryName_VFAT(file_t* directoryEntry,
						   directory_entry_raw_t* vfatRawEntrySequence,
						   uint32_t vfatSequenceCount);


/*
 * Used to extract the type (FILE or DIRECTORY) from the raw directory entry.
 */
void extractEntrytype(file_t* directoryEntry,
					   directory_entry_raw_t* directoryEntryRaw);


/*
 * Used to extract the first cluster number from the raw directory entry.
 * NOTE: this function will then use that cluster number to get the full
 * cluster sequence, which is then stored in the file_t struct.
 */
void extractEntryFirstCluster(file_t* directoryEntry,
							  directory_entry_raw_t* directoryEntryRaw,
							  boot_sect_t* bootSector,
							  uint32_t* fileAllocationTable);


/*
 * Used to extract the file size from the raw directory entry.
 */
void extractEntrySize(file_t* directoryEntry,
					  directory_entry_raw_t* directoryEntryRaw);




//
// IMPLEMENTATION OF THE FUNCTIONS DEFINED IN THE HEADER (.h) FILE.
//


file_t* getDirectoryTree(boot_sect_t* bootSector,
                         uint32_t*    fileAllocationTable,
                         FILE*        storageDevice) {

	//
	// PARAMETER CHECK.
	if (bootSector == NULL)
		handleError(L"getDirectory",
					L"NULL 'bootSector' parameter");
	if (fileAllocationTable == NULL)
		handleError(L"getDirectory",
					L"NULL 'fileAllocationTable' parameter");
	if (storageDevice == NULL)
		handleError(L"getDirectory",
					L"NULL 'storageDevice' parameter");

	//
	// CREATE A ROOT DIRECTORY AND FILL IN WHAT WE ALREADY KNOW.
	file_t* rootDirectory = (file_t*) malloc(sizeof(file_t));
	rootDirectory->name = L"";
	rootDirectory->type = 1;
	rootDirectory->size = 0;
	rootDirectory->parentDirectory = NULL;

	//
	// THE ROOT DIRECTORY MUST BE READ IN MANUALLY FOR A FAT12 SYSTEM, BECAUSE
	// IT DOES NOT HAVE A CLUSTER SEQUENCE (IT IS STORED BEFORE THE DATA ARE).
	// FOR FAT32, HOWEVER, THE ROOT DIRECTORY CAN BE READ IN LIKE ANY OTHER
	// DIRECTORY ON DISK, BECAUSE IT IS IN THE DATA AREA AND, THEREFORE, HAS A
	// CLUSTER SEQUENCE.
	directory_entry_raw_t* rootDirectoryRaw;
	uint32_t maxRootDirectoryEntries;
	uint32_t bufferSize;
	switch (getFatVersion(bootSector)) {

		case FAT12:

			//
			// THERE ARE NO CLUSTERS FOR THE ROOT IN FAT12, BECAUSE IN FAT12,
			// THE ROOT DIRECTORY COMES BEFORE THE START OF THE DATA AREA.
			rootDirectory->clusters = NULL;
			rootDirectory->numClusters = 0;

			//
			// CALCULATE THE SIZE OF THE BUFFER TO ALLOCATE.
			bufferSize = BYTES_PER_DIRECTORY_ENTRY
			           * bootSector->numRootEntries_FAT12;

			//
			// CREATE THE MEMORY BUFFER.
			rootDirectoryRaw = (directory_entry_raw_t*) malloc(bufferSize);
			if (rootDirectoryRaw == NULL)
				handleError(L"getDirectoryTree", L"Unable to allocate memory to read the root directory");

			//
			// GET THE STARTING SECTOR NUMBER.
			uint32_t* sectorNumber = (uint32_t*) malloc(sizeof(uint32_t));
			sectorNumber[0] = getSectorNumber_RootDirectory(bootSector);

			//
			// GET THE NUMBER OF SECTORS IN THE ROOT DIRECTORY.
			uint32_t numSectors = bufferSize / bootSector->bytesPerSector;

			//
			// GET THE RAW ROOT DIRECTORY DATA.
			readSectors((uint8_t*) rootDirectoryRaw,
						sectorNumber,
			            1,
			            bootSector->bytesPerSector,
			            numSectors,
			            storageDevice);
			free(sectorNumber);

			//
			// GET THE MAXIMUM POSSIBLE NUMBER OF DIRECTORY ENTRIES (USED BY THE PARSER).
			maxRootDirectoryEntries = bootSector->numRootEntries_FAT12;
			break;

		case FAT32:

			//
			// THERE ARE CLUSTERS FOR THE ROOT IN FAT32, JUST LIKE WITH ANY
			// OTHER DIRECTORY, BECAUSE IN FAT32, THE ROOT DIRECTORY IS PART OF
			// THE DATA AREA.
			// THE FOLLOWING FUNCTION CALL WILL GET THE SEQUENCE OF ROOT
			// CLUSTERS, AS WELL AS SET THE numClusters ENTRY IN THE ROOT
			// DIRECTORY STRUCT AT THE SAME TIME.
			rootDirectory->clusters =
			                getClusterSequence(bootSector->rootClusterNumber_FAT32,
											   bootSector,
											   fileAllocationTable,
											   &(rootDirectory->numClusters));

			//
			// CALCULATE THE SIZE OF THE BUFFER TO ALLOCATE.
			bufferSize = rootDirectory->numClusters
			           * bootSector->sectorsPerCluster
			           * bootSector->bytesPerSector;

			//
			// CREATE THE MEMORY BUFFER.
			rootDirectoryRaw = (directory_entry_raw_t*) malloc(bufferSize);
			if (rootDirectoryRaw == NULL)
				handleError(L"getDirectoryTree", L"Unable to allocate memory to read the root directory");
			readClusters((uint8_t*) rootDirectoryRaw,
			             rootDirectory->clusters,
			             rootDirectory->numClusters,
			             bootSector,
			             storageDevice);

			//
			// GET THE MAXIMUM POSSIBLE NUMBER OF DIRECTORY ENTRIES (USED BY THE PARSER).
			maxRootDirectoryEntries = bufferSize / BYTES_PER_DIRECTORY_ENTRY;
			break;

	}

	//
	// GET THE PARSED ROOT ENTRIES.
	rootDirectory->children = parseDirectoryEntries(rootDirectoryRaw,
										  &(rootDirectory->numChildren),
										  maxRootDirectoryEntries,
										  fileAllocationTable,
										  bootSector);

	//
	// FREE THE RAW DATA BUFFER.
	free(rootDirectoryRaw);

	//
	// CALL THE RECURSIVE FUNCTION.
	getDirectoryTreeRecursive(rootDirectory, bootSector,
	                          fileAllocationTable, storageDevice);

	//
	// RETURN THE DIRECTORY TREE.
	return rootDirectory;

}


void getDirectoryTreeRecursive(file_t* root,
                               boot_sect_t* bootSector,
                               uint32_t*    fileAllocationTable,
                               FILE*        storageDevice) {

	if (root == NULL)
		return;

	uint32_t childIndex = 0;
	directory_entry_raw_t* childRaw;
	while (childIndex < root->numChildren) {

		//
		// SETTING THE PARENT OF THE CHILDREN TO root.
		root->children[childIndex].parentDirectory = root;

		//
		// RECURSIVE CALL ON DIRECTORIES.
		if(root->children[childIndex].type) {

			//
			// GET CHILD'S RAW DIRECTORY CONTENTS.
			childRaw = (directory_entry_raw_t*)
						malloc(root->children[childIndex].numClusters
							   * bootSector->sectorsPerCluster
							   * bootSector->bytesPerSector);
			if (childRaw == NULL)
				handleError(L"getDirectoryTreeRecursive", L"Unable to allocate memory to read a directory");
			readClusters((uint8_t*) childRaw,
						 root->children[childIndex].clusters,
			             root->children[childIndex].numClusters,
			             bootSector,
			             storageDevice);

			//
			// GET THE PARSED ENTRIES FOR THE CHILD DIRECTORY.
			uint32_t maxEntries = (root->children[childIndex].numClusters
			                           * bootSector->sectorsPerCluster
								       * bootSector->bytesPerSector
								   ) / BYTES_PER_DIRECTORY_ENTRY;
			root->children[childIndex].children =
			    parseDirectoryEntries(childRaw,
			                          &(root->children[childIndex].numChildren),
			                          maxEntries,
									  fileAllocationTable,
			                          bootSector);

			//
			// FREE THE RAW DATA BUFFER.
			free(childRaw);

			//
			// CALL THE RECURSIVE FUNCTION.
			getDirectoryTreeRecursive(&(root->children[childIndex]), bootSector,
									  fileAllocationTable, storageDevice);

		}
		childIndex++;
	}
}




//
// IMPLEMENTATION OF THE HELPER FUNCTIONS AND DATA STRUCTURES DEFINED ABOVE.
//


struct directory_entry_raw_t {

	char name[8];                /* The name, not including the file extension */
	char extension[3];           /* The file extension */
	uint8_t attributes[1];       /* Stores useful bit values that represent various attributes */
	uint8_t reserved[10];        /* Ignore this */
	uint8_t time[2];             /* The time of day in which the file was last modified */
	uint8_t date[2];             /* The date in which the file was last modified */
	uint8_t firstCluster[2];     /* The number of the first cluster in the directory file */
	uint8_t fileSize[4];         /* The file size (in bytes) */

};


file_t* parseDirectoryEntries(directory_entry_raw_t* directoryEntriesRaw,
							  uint32_t* numEntries,
							  uint32_t maxDirectoryEntries,
							  uint32_t* fileAllocationTable,
							  boot_sect_t* bootSector) {

	//
	// CREATE THE EMPTY DIRECTORY ENTRY STRUCTS.
	file_t* directoryEntries = (file_t*)
			malloc(maxDirectoryEntries * sizeof(file_t));

	//
	// VARIABLES USED IN LOOP.
	uint32_t indexSrc = 0;
	*numEntries = 0;
	uint32_t vfatSequenceCount = 0;
	directory_entry_raw_t* vfatRawEntrySequence = (directory_entry_raw_t*)
			malloc(MAX_ENTRIES_PER_VFAT_SEQUENCE * BYTES_PER_DIRECTORY_ENTRY);

	//
	// ITERATE THROUGH THE RAW ENTRIES, DECIDE WHICH ENTRIES TO KEEP AND WHICH TO
	// SKIP, AND FOR THOSE WE KEEP, PARSE THEM.
	while (indexSrc < maxDirectoryEntries) {

		//
		// GET POINTERS TO THE CURRENT SOURCE AND DESTINATION ENTRIES.
		file_t* dstEntry = &(directoryEntries[*numEntries]);
		directory_entry_raw_t* srcEntry = (directory_entry_raw_t*)
				(((uint8_t*) directoryEntriesRaw) + (indexSrc * BYTES_PER_DIRECTORY_ENTRY));

		//
		// CHECK IF THIS ENTRY MARKS THE END OF THE DIRECTORY.
		if (((uint8_t*) srcEntry)[0x00] == 0)
			break;

		//
		// IF THIS ENTRY IS MARKED FOR DELETION.
		if (((uint8_t*) srcEntry)[0x00] == 0x05 || ((uint8_t*) srcEntry)[0x00] == 0xe5) {
			indexSrc = indexSrc + 1;      // SKIP THIS RAW ENTRY.
			continue;
		}

		//
		// IF THIS ENTRY IS MARKED AS A VOLUME LABEL, BUT IS NOT A VFAT ENTRY.
		if (((((uint8_t*) srcEntry)[0x0b]) & 0x08) != 0 && ((((uint8_t*) srcEntry)[0x0b]) & 0x0f) != 0x0f) {
			indexSrc = indexSrc + 1;      // SKIP THIS RAW ENTRY.
			continue;
		}
		
		//
		// IF THIS ENTRY IS A '.' OR '..' DIRECTORY ENTRY.
		if ((((uint8_t*) srcEntry)[0x0b] & 0x10) != 0 &&
			((uint8_t*) srcEntry)[0x00] == '.' && (((uint8_t*) srcEntry)[0x01] == '.' || ((uint8_t*) srcEntry)[0x01] == ' ') &&
			((uint8_t*) srcEntry)[0x02] == ' ' &&  ((uint8_t*) srcEntry)[0x03] == ' ' &&
			((uint8_t*) srcEntry)[0x04] == ' ' &&  ((uint8_t*) srcEntry)[0x05] == ' ' &&
			((uint8_t*) srcEntry)[0x06] == ' ' &&  ((uint8_t*) srcEntry)[0x07] == ' ' &&
			((uint8_t*) srcEntry)[0x08] == ' ' &&  ((uint8_t*) srcEntry)[0x09] == ' ' &&
			((uint8_t*) srcEntry)[0x0a] == ' ') {
			indexSrc = indexSrc + 1;      // SKIP THIS RAW ENTRY.
			continue;
		}

		//
		// CHECK IF THIS ENTRY IS PART OF A SERIES OF VFAT ENTRIES.
		if (((((uint8_t*) srcEntry)[0x0b]) & 0x0f) == 0x0f) {
			memcpy(&(vfatRawEntrySequence[vfatSequenceCount]),
					 srcEntry, BYTES_PER_DIRECTORY_ENTRY);
			vfatSequenceCount = vfatSequenceCount + 1;
			indexSrc = indexSrc + 1;
			continue;
		}
		
		//
		// CHECK IF THIS IS THE LAST ENTRY IN A VFAT SEQUENCE.
		if (vfatSequenceCount > 0) {
			memcpy(&(vfatRawEntrySequence[vfatSequenceCount]),
					 srcEntry, BYTES_PER_DIRECTORY_ENTRY);
			vfatSequenceCount = vfatSequenceCount + 1;
			parseDirectoryEntry_VFAT(dstEntry,
			                         vfatRawEntrySequence,
			                         vfatSequenceCount,
									 fileAllocationTable,
			                         bootSector);
			vfatSequenceCount = 0;
			indexSrc = indexSrc + 1;
			*numEntries = *numEntries + 1;
			continue;
		}

		//
		// IF THE CURRENT ITERATION MADE IT THIS FAR, THEN THIS ENTRY IS
		// JUST AN ORDINARY DIRECTORY ENTRY.
		parseDirectoryEntry(dstEntry, srcEntry, fileAllocationTable, bootSector);
		indexSrc = indexSrc + 1;
		*numEntries = *numEntries + 1;

	}
	
	return directoryEntries;
}


void parseDirectoryEntry(file_t* directoryEntry,
                         directory_entry_raw_t* directoryEntryRaw,
						 uint32_t* fileAllocationTable,
                         boot_sect_t* bootSector) {

	//
	// EXTRACT THE FILE'S NAME, TYPE, FIRST CLUSTER, AND SIZE FROM DIRECTORY ENTRY.
	extractEntryName(directoryEntry, directoryEntryRaw);
	extractEntrytype(directoryEntry, directoryEntryRaw);
	extractEntryFirstCluster(directoryEntry, directoryEntryRaw, bootSector, fileAllocationTable);
	extractEntrySize(directoryEntry, directoryEntryRaw);

}


void parseDirectoryEntry_VFAT(file_t* directoryEntry,
                              directory_entry_raw_t* vfatRawEntrySequence,
                              uint32_t vfatSequenceCount,
							  uint32_t* fileAllocationTable,
                              boot_sect_t* bootSector) {

	//
	// EXTRACT THE FILE'S NAME, TYPE, FIRST CLUSTER, AND SIZE FROM DIRECTORY ENTRY.
	extractEntryName_VFAT(directoryEntry, vfatRawEntrySequence, vfatSequenceCount);
	extractEntrytype(directoryEntry, &(vfatRawEntrySequence[vfatSequenceCount-1]));
	extractEntryFirstCluster(directoryEntry, &(vfatRawEntrySequence[vfatSequenceCount-1]), bootSector, fileAllocationTable);
	extractEntrySize(directoryEntry, &(vfatRawEntrySequence[vfatSequenceCount-1]));

}


void extractEntryName(file_t* directoryEntry,
					  directory_entry_raw_t* directoryEntryRaw) {

	//
	// ALLOCATE MEMORY FOR NAME.
	directoryEntry->name = (wchar_t*) calloc(13, sizeof(wchar_t));

	int end;

	//
	// REPRESENTS THE CURRENT BYTE/CHARACTER INDEX IN THE RAW ENTRY DATA AND
	// THE DIRECTORYENTRY'S NAME STRING, RESPECTIVELY.
	uint8_t rawEntryIndex = 0;
	uint8_t nameIndex = 0;

	//
	// PART 1: EXTRACT THE NAME.
	//

	//
	// DETERMINING THE FIRST AND LAST BYTES IN THE RAW DATA THAT CONTAIN THE
	// NAME OF THE FILE.
	end = 7;
	while (end != -1) {
		if (directoryEntryRaw->name[end] == 0x20)
			end--;
		else
			break;
	}

	//
	// COPYING THE NAME INTO THE NAME STRING STORED IN THE DIRECTORYENTRY.
	rawEntryIndex = 0;
	while (rawEntryIndex <= end) {
		directoryEntry->name[nameIndex] =
				(wchar_t) directoryEntryRaw->name[rawEntryIndex];
		rawEntryIndex++;
		nameIndex++;
	}

	//
	// PART 2: EXTRACT THE EXTENSION.
	//

	//
	// DETERMINING THE FIRST AND LAST BYTES IN THE RAW DATA THAT CONTAIN THE
	// EXTENSION OF THE FILE.
	end   = 2;
	while (end != -1) {
		if (directoryEntryRaw->extension[end] == 0x20)
			end--;
		else
			break;
	}

	//
	// CHECK IF THERE'S AN EXTENSION.  IF SO, ADD A PERIOD.
	if (end >= 0) {
		directoryEntry->name[nameIndex] = L'.';
		nameIndex++;
	}

	//
	// COPYING THE EXTENSION INTO THE NAME STRING STORED IN THE DIRECTORYENTRY.
	rawEntryIndex = 0;
	while (rawEntryIndex <= end) {
		directoryEntry->name[nameIndex] =
				(wchar_t) directoryEntryRaw->extension[rawEntryIndex];
		rawEntryIndex++;
		nameIndex++;
	}

	//
	// ADD A TERMINATING NULL CHARACTER.
	directoryEntry->name[nameIndex] = L'\0';
	
}


void extractEntryName_VFAT(file_t* directoryEntry,
					  directory_entry_raw_t* vfatRawEntrySequence,
					  uint32_t vfatSequenceCount) {

	//
	// ALLOCATE ENOUGH CHARACTERS FOR THE DIRECTORY NAME.
	// EACH ENTRY CAN STORE UP TO 26 CHARACTERS.
	char* name = (char*) calloc((26 * vfatSequenceCount) + 2, 1);
	int nameLength = 0;

	//
	// COPY CHARACTERS TO TEMPORARY ARRAY.
	int vfatSequenceIndex = vfatSequenceCount - 2;
	int iterationCount = 0;
	while (vfatSequenceIndex >= 0) {
		name[(iterationCount * 26) +  0] = ((char*) vfatRawEntrySequence)[(vfatSequenceIndex * 32) +  1];
		name[(iterationCount * 26) +  1] = ((char*) vfatRawEntrySequence)[(vfatSequenceIndex * 32) +  2];
		name[(iterationCount * 26) +  2] = ((char*) vfatRawEntrySequence)[(vfatSequenceIndex * 32) +  3];
		name[(iterationCount * 26) +  3] = ((char*) vfatRawEntrySequence)[(vfatSequenceIndex * 32) +  4];
		name[(iterationCount * 26) +  4] = ((char*) vfatRawEntrySequence)[(vfatSequenceIndex * 32) +  5];
		name[(iterationCount * 26) +  5] = ((char*) vfatRawEntrySequence)[(vfatSequenceIndex * 32) +  6];
		name[(iterationCount * 26) +  6] = ((char*) vfatRawEntrySequence)[(vfatSequenceIndex * 32) +  7];
		name[(iterationCount * 26) +  7] = ((char*) vfatRawEntrySequence)[(vfatSequenceIndex * 32) +  8];
		name[(iterationCount * 26) +  8] = ((char*) vfatRawEntrySequence)[(vfatSequenceIndex * 32) +  9];
		name[(iterationCount * 26) +  9] = ((char*) vfatRawEntrySequence)[(vfatSequenceIndex * 32) + 10];
		
		name[(iterationCount * 26) + 10] = ((char*) vfatRawEntrySequence)[(vfatSequenceIndex * 32) + 14];
		name[(iterationCount * 26) + 11] = ((char*) vfatRawEntrySequence)[(vfatSequenceIndex * 32) + 15];
		name[(iterationCount * 26) + 12] = ((char*) vfatRawEntrySequence)[(vfatSequenceIndex * 32) + 16];
		name[(iterationCount * 26) + 13] = ((char*) vfatRawEntrySequence)[(vfatSequenceIndex * 32) + 17];
		name[(iterationCount * 26) + 14] = ((char*) vfatRawEntrySequence)[(vfatSequenceIndex * 32) + 18];
		name[(iterationCount * 26) + 15] = ((char*) vfatRawEntrySequence)[(vfatSequenceIndex * 32) + 19];
		name[(iterationCount * 26) + 16] = ((char*) vfatRawEntrySequence)[(vfatSequenceIndex * 32) + 20];
		name[(iterationCount * 26) + 17] = ((char*) vfatRawEntrySequence)[(vfatSequenceIndex * 32) + 21];
		name[(iterationCount * 26) + 18] = ((char*) vfatRawEntrySequence)[(vfatSequenceIndex * 32) + 22];
		name[(iterationCount * 26) + 19] = ((char*) vfatRawEntrySequence)[(vfatSequenceIndex * 32) + 23];
		name[(iterationCount * 26) + 20] = ((char*) vfatRawEntrySequence)[(vfatSequenceIndex * 32) + 24];
		name[(iterationCount * 26) + 21] = ((char*) vfatRawEntrySequence)[(vfatSequenceIndex * 32) + 25];
		
		name[(iterationCount * 26) + 22] = ((char*) vfatRawEntrySequence)[(vfatSequenceIndex * 32) + 28];
		name[(iterationCount * 26) + 23] = ((char*) vfatRawEntrySequence)[(vfatSequenceIndex * 32) + 29];
		name[(iterationCount * 26) + 24] = ((char*) vfatRawEntrySequence)[(vfatSequenceIndex * 32) + 30];
		name[(iterationCount * 26) + 25] = ((char*) vfatRawEntrySequence)[(vfatSequenceIndex * 32) + 31];

		vfatSequenceIndex--;
		iterationCount++;
		nameLength += 13;
	}

	//
	// COPY CHARACTERS TO FINAL ARRAY AS 2-BYTE WIDE CHARACTERS.
	directoryEntry->name = (wchar_t*) calloc(nameLength + 2, sizeof(wchar_t));
	int nameIndex = 0;
	while (nameIndex < nameLength) {
		directoryEntry->name[nameIndex] = (wchar_t)
										  ((((uint16_t) name[(nameIndex*2) + 0]) << 0) |
										   (((uint16_t) name[(nameIndex*2) + 1]) << 8) );
		nameIndex++;
	}
	
}


void extractEntrytype(file_t* directoryEntry,
					   directory_entry_raw_t* directoryEntryRaw) {

	//
	// THE BYTE AT THIS INDEX CONTAINS THE type FLAG AT THE FIFTH LEAST-
	// SIGNIFICANT BIT.
	directoryEntry->type = (directoryEntryRaw->attributes[0] & 0b00010000) == 0 ? 0 : 1;
	
}


void extractEntryFirstCluster(file_t* directoryEntry,
							  directory_entry_raw_t* directoryEntryRaw,
							  boot_sect_t* bootSector,
							  uint32_t* fileAllocationTable) {
	uint32_t firstCluster;
	switch(getFatVersion(bootSector)) {

		case FAT12:	

			//
			// EXTRACTS THE 2-BYTE INTEGER FROM THE RAW DATA THAT REPRESENTS THE
			// NUMBER OF THE FIRST CLUSTER.
			firstCluster = (uint32_t)
			               (((uint32_t) directoryEntryRaw->firstCluster[1]) <<  8) |
			               (((uint32_t) directoryEntryRaw->firstCluster[0]) <<  0);
			break;

		case FAT32:

			//
			// EXTRACTS THE 4-BYTE INTEGER FROM THE RAW DATA THAT REPRESENTS THE
			// NUMBER OF THE FIRST CLUSTER.
			firstCluster = (uint32_t)
			               (((uint32_t) directoryEntryRaw->reserved[9]     & 0b00001111) << 24) |
			               (((uint32_t) directoryEntryRaw->reserved[8]     & 0b11111111) << 16) |
			               (((uint32_t) directoryEntryRaw->firstCluster[1] & 0b11111111) <<  8) |
			               (((uint32_t) directoryEntryRaw->firstCluster[0] & 0b11111111) <<  0) ;
			break;

	}

	//
	// GET THE CLUSTER SEQUENCE.
	directoryEntry->clusters = getClusterSequence(firstCluster,
	                                              bootSector,
												  fileAllocationTable,
												  &(directoryEntry->numClusters));

}


void extractEntrySize(file_t* directoryEntry,
					  directory_entry_raw_t* directoryEntryRaw) {

	//
	// EXTRACTS THE 4-BYTE INTEGER FROM THE RAW DATA THAT REPRESENTS THE
	// SIZE OF THE FILE (IN BYTES).
	directoryEntry->size = (uint32_t)
                           (((uint32_t) directoryEntryRaw->fileSize[3]) << 24) |
                           (((uint32_t) directoryEntryRaw->fileSize[2]) << 16) |
                           (((uint32_t) directoryEntryRaw->fileSize[1]) <<  8) |
                           (((uint32_t) directoryEntryRaw->fileSize[0]) <<  0);

}

