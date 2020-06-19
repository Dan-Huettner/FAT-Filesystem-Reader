/******************************************************************************
 * This file contains functions and data structures that pertain to
 *                               ERROR HANDLING
 * for the program.
 *
 * By Daniel Huettner
 *****************************************************************************/




//
// INCLUDES
//

// LAYER 1: USER_INTERFACE
#include "user_interface_tools.h"

// LAYER 2: FILE_SYSTEM
// (NOTHING)

// LAYER 3: STORAGE_DEVICE
// (NOTHING)

// ERROR HANDLING
#include "error.h"

// STANDARD C LIBRARY
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>




//
// IMPLEMENTATION OF THE FUNCTIONS DEFINED IN THE HEADER (.h) FILE.
//


void handleError(wchar_t* functionName, wchar_t* description) {

	//
	// PRINT A DASHED LINE.
	printDashedLine();

	//
	// PRINT CENTERED TITLE.
	wchar_t* title = L"AN ERROR HAS OCCURED:";
	wprintf(L"%ls%*ls%*ls\n",
			L"|",
			((getTermWidth() - wcslen(title)) / 2) + wcslen(title), title,
			((getTermWidth() - wcslen(title)) / 2), L"|");

	//
	// PRINT THE FUNCTION NAME.
	wchar_t* startName = L"|IN FUNCTION: ";
	wprintf(L"%ls%ls%*ls\n",
			startName, functionName, getTermWidth() - wcslen(functionName) - wcslen(startName), L"|");

	//
	// PRINT THE DESCRIPTION.
	wchar_t* startDesc = L"|DESCRIPTION: ";
	wprintf(L"%ls%ls%*ls\n",
			startDesc, description,  getTermWidth() - wcslen(description) - wcslen(startDesc), L"|");

	//
	// PRINT AN EMPTY LINE.
	wprintf(L"%ls%*ls\n",
			L"|", getTermWidth() - 1, L"|");

	//
	// PRINT TERMINATION WARNING.
	wchar_t* terminationWarning = L"|The Program Will Now Terminate";
	wprintf(L"%ls%*ls\n",
			terminationWarning, getTermWidth() - wcslen(terminationWarning), L"|");

	//
	// PRINT A DASHED LINE.
	printDashedLine();

	//
	// TERMINATE THE PROGRAM.
	exit(1);

}

