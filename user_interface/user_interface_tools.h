/******************************************************************************
 * This file contains functions that pertain to the
 *                               USER INTERFACE
 * of the program.
 *
 * By Daniel Huettner
 *****************************************************************************/

#ifndef USER_INTERFACE_TOOLS_H_
#define USER_INTERFACE_TOOLS_H_




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
#include <wchar.h>
#include <stdint.h>




//
// CONSTANTS
//

// THE NUMBER OF CHARACTERS PER ROW FOR THE TERMINAL SCREEN.
// NOTE: THESE CAN BE CHANGED.
#define DEFAULT_CHARACTERS_PER_ROW 80
#define MINIMUM_CHARACTERS_PER_ROW 40
#define MAXIMUM_CHARACTERS_PER_ROW 240




/*
 * Prints the provided text such that it is centered on the console screen.
 */
void printCentered(wchar_t** lines, uint32_t numLines, uint8_t enableBorder);




/*
 * Prints a dashed line (series of dashes) to the console screen.
 */
void printDashedLine();




/*
 * Attempts to get the terminal width (i.e. the number of characters that can
 * fit on a line).
 * If it cannot get the width, the default width is returned (defined at the top
 * of this header file).
 */
int getTermWidth();




#endif

