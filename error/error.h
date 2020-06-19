/******************************************************************************
 * This file contains functions and data structures that pertain to
 *                               ERROR HANDLING
 * for the program.
 *
 * By Daniel Huettner
 *****************************************************************************/

#ifndef ERROR_H_
#define ERROR_H_




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
// (NOTHING)

// STANDARD C LIBRARY
#include <wchar.h>




/*
 * Prints a nicely-formatted error message to the console, then terminates with
 * error code 1.
 *
 * @param functionName
 *     The name of the function which the error occurred.
 * @param description
 *     A message describing the particular error that occurred.
 */
void handleError(wchar_t* functionName, wchar_t* description);


#endif

