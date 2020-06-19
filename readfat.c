/******************************************************************************
 *
 *          This file contains the main function of the program.
 *
 * By Daniel Huettner
 *****************************************************************************/




//
// INCLUDES
//

// LAYER 1: USER_INTERFACE
#include "print_header.h"
#include "print_directory.h"
#include "print_fs_info.h"
#include "user_interface_tools.h"

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
#include <locale.h>





int main(int argc, char** argv) {

	//
	// NEEDED FOR PRINTING UTF-16 CHARACTERS.
	setlocale(LC_ALL, "");
	
	//
	// PRINT A PROGRAM HEADER.
	printHeader();

	//
	// CHECK COMMAND ARGUMENTS.
	if (argc != 2)
		handleError(L"main", L"The Image Pathname Must Be Specified in the Command");

	//
	// GET FILENAME.
	char* fileName = argv[1];

	//
	// OPEN THE STORAGE DEVICE FILE.
	FILE* storageDevice = openStorageDevice(fileName);

	//
	// GET THE BOOT SECTOR.
	boot_sect_t* bootSector = getBootSector(storageDevice);

	//
	// GET THE FAT VERSION.
	uint8_t fatVersion = getFatVersion(bootSector);

	//
	// CHECK THE FAT VERSION.
	if (fatVersion == FAT16)
		handleError(L"main", L"FAT16 File Systems are not Supported");	

	//
	// PRINT THE FILE SYSTEM INFORMATION.
	printFileSystemInformation(fileName, bootSector);

	//
	// GET THE FILE ALLOCATION TABLE.
	uint32_t* fileAllocationTable = getFileAllocationTable(bootSector, storageDevice);

	//
	// GET THE DIRECTORY TREE.
	file_t* directoryTree = getDirectoryTree(bootSector, fileAllocationTable, storageDevice);

	//
	// PRINT THE DIRECTORY TREE.
	printDirectoryTreeHeader();
	printDirectory(directoryTree, 1, bootSector, fileAllocationTable);
	
	//
	// CLOSE THE STORAGE DEVICE FILE.
	closeStorageDevice(storageDevice);

	//
	// RETURN WITH SUCCESSFUL ERROR CODE (0).
	return 0;
	
}

