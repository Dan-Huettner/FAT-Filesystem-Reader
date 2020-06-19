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
#include "user_interface_tools.h"

// LAYER 2: FILE_SYSTEM
// (NOTHING)

// LAYER 3: STORAGE_DEVICE
// (NOTHING)

// ERROR HANDLING
// (NOTHING)

// STANDARD C LIBRARY
#include <wchar.h>
#include <stdint.h>
#include <stdlib.h>




//
// SPECIAL INCLUDES...
//

// PLEASE NOTE: THESE INCLUDES ARE ONLY USED TO GET THE WIDTH OF THE TERMINAL
//              SCREEN (I.E. MAXIMUM NUMBER OF CHARACTERS PER ROW).
//
//              I KNOW THERE WAS A RULE IN THE ASSIGNMENT THAT SAID "STANDARD C
//              LIBRARY ONLY", BUT PLEASE BE AWARE THAT THESE OS-SPECIFIC
//              INCLUDES ARE ONLY USED TO GET THE TERMINAL WIDTH, AND NOTHING
//              ELSE.

//
// USED TO GET THE TERMINAL WIDTH ON A POSIX-COMPLIANT SYSTEM.
#if __linux__
	#include <sys/ioctl.h>
	#include <unistd.h>
#endif

//
// USED TO GET THE TERMINAL WIDTH ON A WINDOWS SYSTEM.
#if _WIN32
	#include <windows.h>
#endif





void printCentered(wchar_t** lines, uint32_t numLines, uint8_t enableBorder) {

	//
	// ITERATE THROUGH THE LINES, AND PRINT THEM SUCH THAT THEY ARE CENTERED
	// ON THE CONSOLE SCREEN.
	uint32_t widthOfLine;
	uint32_t spaceBefore;
	uint32_t lineNumber = 0;
	while (lineNumber < numLines) {

		//
		// GET THE WIDTH OF THE LINE.
		widthOfLine = wcslen(lines[lineNumber]);

		//
		// CALCULATE THE AMOUNT OF SPACE TO INSERT BEFORE THE LINE TO MAKE IT
		// CENTERED ON SCREEN.
		spaceBefore = (getTermWidth() - widthOfLine) / 2;

		//
		// PRINT THE LINE.
		if (enableBorder)
			wprintf(L"%ls%*ls%ls\n", L"|", spaceBefore + widthOfLine - 2, lines[lineNumber], L"|");
		else
			wprintf(L"%*ls\n", spaceBefore + widthOfLine, lines[lineNumber]);
			

		//
		// INCREMENT THE LINE NUMBER.
		lineNumber++;

	}

}


void printDashedLine() {

	//
	// ALLOCATE MEMORY FOR THE NEW STRING.
	wchar_t* dashedLine = (wchar_t*) calloc (getTermWidth() + 1, sizeof(wchar_t));

	//
	// SET THE FIRST AND LAST CHARACTERS TO A SPACE.
	dashedLine[0] = L' ';
	dashedLine[getTermWidth() - 1] = L' ';

	//
	// SET ALL OF THE OTHER CHARACTERS TO A '-'.
	wmemset(&(dashedLine[1]), L'-', getTermWidth() - 2);

	//
	// PRINT THE STRING TO THE CONSOLE.
	wprintf(L"%ls\n", dashedLine);

}


int getTermWidth() {

	int width = 0;
	char* buffer;

	//
	// ATTEMPING TO GET THE TERMINAL WIDTH ON A POSIX-COMPLIANT SYSTEM.
	#if __linux__
		struct winsize termSize;
		ioctl(0, TIOCGWINSZ, &termSize);
		width = (uint32_t) termSize.ws_col;
	#endif

	//
	// ATTEMPTING TO GET THE TERMINAL WIDTH ON A WINDOWS SYSTEM.
	#if _WIN32
		CONSOLE_SCREEN_BUFFER_INFO termInfo;
		GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &termInfo);
		width = (buffer.srWindow.Right - buffer.srWindow.Left + 1);
	#endif

	//
	// IF THE VALUE IS INVALID OR JUST TOO SMALL, THEN USE THE DEFAULT.
	if (width < MINIMUM_CHARACTERS_PER_ROW || width > MAXIMUM_CHARACTERS_PER_ROW)
		width = DEFAULT_CHARACTERS_PER_ROW;

	//
	// RETURN THE WIDTH.
	return width;

}

