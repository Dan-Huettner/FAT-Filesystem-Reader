/******************************************************************************
 * This file contains functions that pertain to the
 *                               USER INTERFACE
 * of the program.
 *
 * By Daniel Huettner
 *****************************************************************************/




//
// INCLUDES
//

// LAYER 1: USER_INTERFACE
#include "print_fs_info.h"
#include "user_interface_tools.h"

// LAYER 2: FILE_SYSTEM
#include "boot_sector.h"
#include "file_system_tools.h"

// LAYER 3: STORAGE_DEVICE
// (NOTHING)

// ERROR HANDLING
#include "error.h"

// STANDARD C LIBRARY
#include <stdint.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>





void printFileSystemInformation(char* deviceFileName,
                                boot_sect_t* bootSector) {

	//
	// PARAMETER CHECK.
	if (bootSector == NULL)
		handleError(L"printFileSystemInformation", L"NULL 'deviceFileName' parameter");
	if (bootSector == NULL)
		handleError(L"printFileSystemInformation", L"NULL 'bootSector' parameter");

	//
	// MAKE SURE THE FILENAME IS NOT TOO LONG.
	char temp = 0;
	if (strlen(deviceFileName) > RIGHT_COLUMN_WIDTH_FS) {
		temp = deviceFileName[RIGHT_COLUMN_WIDTH_FS];
		deviceFileName[RIGHT_COLUMN_WIDTH_FS] = 0;
	}

	//
	// TRANSLATE FILENAME INTO WIDE CHARACTERS.
	wchar_t* wideFileName = (wchar_t*) calloc(strlen(deviceFileName)+1, sizeof(wchar_t));
	int nameIndex = 0;
	while (nameIndex < strlen(deviceFileName)) {
		wideFileName[nameIndex] = (wchar_t) deviceFileName[nameIndex];
		nameIndex++;
	}

	//
	// GET VARIOUS VALUES TO BE PRINTED.
	uint32_t firstSector_FAT  = getSectorNumber_FileAllocationTable(bootSector);
	uint32_t firstSector_Root = getSectorNumber_RootDirectory(bootSector);
	uint32_t firstSector_Data =
			getSectorNumber_DataCluster(bootSector, 2);

	//
	// COMPUTE THE CAPACITY OF THE STORAGE DEVICE.
	uint32_t cap = bootSector->numSectors_FAT12 * bootSector->bytesPerSector;
	if (cap == 0)
		cap = bootSector->numSectors_FAT32 * bootSector->bytesPerSector;
	wchar_t* capUnit = L"B";
	if (cap >= 1000) {
		cap = cap / 1000;
		capUnit = L"KB";
	}
	if (cap >= 1000) {
		cap = cap / 1000;
		capUnit = L"MB";
	}
	if (cap >= 1000) {
		cap = cap / 1000;
		capUnit = L"GB";
	}
	uint8_t capLength = 1;
	if (cap >= 10)
		capLength = 2;
	if (cap >= 100)
		capLength = 3;
	if (cap >= 1000)
		capLength = 4;

	//
	// GET THE FAT VERSION.
	uint32_t fatVersion = getFatVersion(bootSector);

	//
	// PRINT THE TITLE.
	wchar_t* title = L"FILESYSTEM INFORMATION";
	wprintf(L"\n");
	wprintf(L"%*ls\n", ((getTermWidth() - wcslen(title)) / 2) + wcslen(title), title);

	//
	// PRINT THE INFORMATION.
	printDashedLine();
	wprintf(L"%ls%-*ls%ls\n",  L"|DEVICE FILE        |",    RIGHT_COLUMN_WIDTH_FS, wideFileName,                    L"|");
	wprintf(L"%ls%-*u%ls\n",   L"|FILE SYSTEM        |FAT", RIGHT_COLUMN_WIDTH_FS - 3, fatVersion,                  L"|");
	wprintf(L"%ls%u%-*ls%ls\n",L"|SIZE               |",    cap, RIGHT_COLUMN_WIDTH_FS - capLength, capUnit,        L"|");
	wprintf(L"%ls%-*u%ls\n",   L"|BYTES PER SECTOR   |",    RIGHT_COLUMN_WIDTH_FS, bootSector->bytesPerSector,      L"|");
	wprintf(L"%ls%-*u%ls\n",   L"|SECTORS PER CLUSTER|",    RIGHT_COLUMN_WIDTH_FS, bootSector->sectorsPerCluster,   L"|");
	if (fatVersion == FAT12)
	wprintf(L"%ls%-*u%ls\n",   L"|ROOT DIR ENTRIES   |",    RIGHT_COLUMN_WIDTH_FS, bootSector->numRootEntries_FAT12,L"|");
	if (fatVersion == FAT12)
	wprintf(L"%ls%-*u%ls\n",   L"|SECTORS PER FAT    |",    RIGHT_COLUMN_WIDTH_FS, bootSector->sectorsPerFAT_FAT12, L"|");
	if (fatVersion == FAT32)
	wprintf(L"%ls%-*u%ls\n",   L"|SECTORS PER FAT    |",    RIGHT_COLUMN_WIDTH_FS, bootSector->sectorsPerFAT_FAT32, L"|");
	wprintf(L"%ls%-*u%ls\n",   L"|RESERVED SECTORS   |",    RIGHT_COLUMN_WIDTH_FS, bootSector->numReservedSectors,  L"|");
	wprintf(L"%ls%-*u%ls\n",   L"|HIDDEN DISK SECTORS|",    RIGHT_COLUMN_WIDTH_FS, bootSector->numHiddenSectors,    L"|");
	wprintf(L"%ls%-*u%ls\n",   L"|FIRST FAT SECTOR   |",    RIGHT_COLUMN_WIDTH_FS, firstSector_FAT,                 L"|");
	wprintf(L"%ls%-*u%ls\n",   L"|FIRST ROOT SECTOR  |",    RIGHT_COLUMN_WIDTH_FS, firstSector_Root,                L"|");
	wprintf(L"%ls%-*u%ls\n",   L"|FIRST DATA SECTOR  |",    RIGHT_COLUMN_WIDTH_FS, firstSector_Data,                L"|");
	printDashedLine();

	//
	// MAKE SURE THE FILE NAME IS NOT TOO LONG (PART II).
	if (temp != 0)
		deviceFileName[RIGHT_COLUMN_WIDTH_FS] = temp;

}
