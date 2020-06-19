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
#include "print_header.h"
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
#include <stdlib.h>
#include <stdio.h>
#include <wchar.h>





void printHeader() {

	//
	// THE HEADER TEXT.
	uint32_t numLines = 8;
	wchar_t** lines = (wchar_t**) malloc(numLines * sizeof(wchar_t*));
	lines[0] = L"";
	lines[1] = L"";
	lines[2] = L"********************************";
	lines[3] = L"*  FAT FILESYSTEM READER V1.0  *";
	lines[4] = L"*                              *";
	lines[5] = L"*      BY DANIEL HUETTNER      *";
	lines[6] = L"********************************";
	lines[7] = L"";

	//
	// PRINT THE HEADER.
	printCentered(lines, numLines, 0);

}

