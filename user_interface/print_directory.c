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
#include "print_directory.h"
#include "user_interface_tools.h"

// LAYER 2: FILE_SYSTEM
#include "boot_sector.h"
#include "file_allocation_table.h"
#include "directory.h"
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





/*
 * Used to print one entry from a directory.
 */
void printDirectoryEntry(file_t* directoryEntry,
						 boot_sect_t* bootSector,
						 uint32_t* fileAllocationTable);


/*
 * Used to print a path name to the console.
 * This function will split a long name over multiple lines.
 */
void printName(wchar_t* absolutePathName);


/*
 * Used to print a sequence of cluster numbers.
 * This function will split a long sequence of cluster numbers over multiple lines.
 */
void printClusterSequence(uint32_t* clusters,       uint32_t numClusters,
                          uint32_t  clustersPerRow, uint32_t clusterNumberLength);





//
// IMPLEMENTATION OF THE FUNCTIONS DEFINED IN THE HEADER (.h) FILE.
//


void printDirectory(file_t*      directory,
                    uint8_t      recursive,
                    boot_sect_t* bootSector,
                    uint32_t*    fileAllocationTable) {

	//
	// PARAMETER CHECK.
	if (directory == NULL)
		handleError(L"printDirectory", L"NULL 'directory' parameter");
	if (bootSector == NULL)
		handleError(L"printDirectory", L"NULL 'bootSector' parameter");
	if (fileAllocationTable == NULL)
		handleError(L"printDirectory", L"NULL 'fileAllocationTable' parameter");

	//
	// PRINTING THE FILES FIRST.
	uint32_t childNumber = 0;
	while (childNumber < directory->numChildren) {
		if (!(directory->children[childNumber].type))
			printDirectoryEntry(&(directory->children[childNumber]), bootSector, fileAllocationTable);
		childNumber++;
	}

	//
	// PRINTING THE DIRECTORIES SECOND.
	childNumber = 0;
	while (childNumber < directory->numChildren) {
		if (directory->children[childNumber].type) {
			printDirectoryEntry(&(directory->children[childNumber]), bootSector, fileAllocationTable);
			if (recursive) {
					printDirectory(&(directory->children[childNumber]),
					               recursive,
								   bootSector,
								   fileAllocationTable);
			}
		}
		childNumber++;
	}
}


void printDirectoryTreeHeader() {

	//
	// PRINT A BLANK LINE.
	wprintf(L"\n");

	//
	// PRINT THE CENTERED TITLE.
	wchar_t* title = L"DRIVE CONTENTS";
	wprintf(L"%*ls\n", ((getTermWidth() - wcslen(title)) / 2) + wcslen(title), title);

	//
	// PRINT A DASHED LINE.
	printDashedLine();

}




//
// IMPLEMENTATION OF THE HELPER FUNCTIONS AND DATA STRUCTURES DEFINED ABOVE.
//


void printDirectoryEntry(file_t* directoryEntry,
						 boot_sect_t* bootSector,
						 uint32_t* fileAllocationTable) {

	//
	// GET THE ABSOLUTE PATH NAME OF THE FILE/DIRECTORY.
	wchar_t* absolutePathName = getAbsolutePathName(directoryEntry);

	//
	// PRINT NAME TO CONSOLE.
	// THIS FUNCTION WILL SPLIT LONG NAMES OVER TWO MORE MORE LINES.
	printName(absolutePathName);

	//
	// FREE THE ABSOLUTE PATH NAME -- WE'RE DONE WITH IT.
	free (absolutePathName);

	//
	// PRINT TYPE TO CONSOLE.
	wchar_t* type = (directoryEntry->type) ? L"DIRECTORY" : L"FILE";
	wprintf(L"%ls%-*ls%ls\n", L"|  TYPE  |", CHARACTERS_PER_ROW_RIGHT_COLUMN, type, L"|");

	//
	// PRINT SIZE TO CONSOLE.
	wprintf(L"%ls%-*u%ls\n", L"|  SIZE  |", CHARACTERS_PER_ROW_RIGHT_COLUMN, directoryEntry->size, L"|");

	//
	// PRINT CLUSTERS TO CONSOLE.
	// USE UP TO 80 CHARACTERS PER LINE TO PRINT THE PATHNAME.
	// THIS CODE WILL SPLIT LONG NAMES OVER TWO MORE MORE LINES (WORD WRAP).
	switch (getFatVersion(bootSector)) {
		case FAT12:
			printClusterSequence(directoryEntry->clusters, directoryEntry->numClusters,
                                 CLUSTERS_PER_ROW_FAT12, CHARACTERS_PER_FAT12_CLUSTER_NUMBER);
			break;
		case FAT32:
			printClusterSequence(directoryEntry->clusters, directoryEntry->numClusters,
                                 CLUSTERS_PER_ROW_FAT32, CHARACTERS_PER_FAT32_CLUSTER_NUMBER);
			break;
	}

	//
	// PRINT THE BOTTOM PART OF THE "BOX" THAT EACH FILE/DIRECTORY APPEARS IN
	// WHEN PRINTED TO THE CONSOLE.
	printDashedLine();
	
}


void printName(wchar_t* absolutePathName) {

	//
	// PRINT THE LEFT COLUMN TO THE CONSOLE.
	wprintf(L"%ls", L"|  NAME  |");

	//
	// USED TO TEMPORARILY STORE A CHARACTER FROM THE GIVEN NAME WHEN WE NEED
	// TO REPLACE THAT CHARACTER IN THE NAME WITH AN END-OF-STRING (NULL)
	// CHARACTER.
	// WE DO THIS REPLACEMENT WHEN WE NEED TO SPLIT A NAME OVER MULTIPLE LINES.
	//
	// THAT IS, IF WE WANT TO ONLY PRINT A PART OF A GIVEN STRING (AND STOP AT
	// AT A CERTAIN POINT) WE TEMPORARILY INSERT A NULL CHARACTER INTO THE STRING
	// AT THAT POINT.  THIS WILL CAUSE THE wprintf FUNCTION TO STOP PRINTING THE
	// STRING WHEN IT REACHES THAT POINT.
	//
	// HENCE, TO SPLIT A STRING OVER MULTIPLE LINES, WE USE THE NULL CHARACTER
	// TO FORCE THE wprintf FUNCTION TO ONLY PRINT PART OF A NAME IN EACH ROW.
	wchar_t temp;

	//
	// AS MENTIONED ABOVE, THE NULL CHARACTER IS TEMPORARILY INSERTED INTO A LONG
	// STRING TO TELL THE wprintf FUNCTION WHERE WE WANT IT TO STOP PRINTING.
	// THE FOLLOWING VARIABLE STORES A POINTER TO THE LOCATION IN THE STRING WHERE
	// WE WANT THE wprintf FUNCTION TO BEGIN FROM.
	//
	// HENCE, TO SPLIT A STRING OVER MULTIPLE LINES, WE SET THE POINTER TO THE
	// POINT WHERE WE STOPPED ON THE PREVIOUS LINE, AND INSERT THE NULL CHARACTER
	// AT THE POINT WHERE WE WANT TO STOP THIS TIME (AT 'x' NUMBER OF CHARACTERS
	// AFTER THE CURRENT POINTER, WHERE 'x' IS THE NUMBER OF CHARACTERS THAT CAN
	// FIT IN EACH ROW).
	wchar_t* startPositionPointer = absolutePathName;

	//
	// THE LOOP IS USED TO SPLIT A LONG NAME OVER MULTIPLE LINES.
	// THE LOOP CONDITION CHECKS THE LENGTH OF THE STRING, STARTING FROM THE POINT
	// WHERE WE'VE PRINTED UP TO SO FAR.  AS LONG AS THE REST OF THE STRING CONTAINS
	// AT LEAST ONE CHARACTER, THE LOOP WILL RE-ITERATE.
	while (wcslen(startPositionPointer) > 0) {

		//
		// IF THE REST OF THE STRING IS TOO LONG TO FIT IN ONE ROW, THEN WE MUST
		// SPLIT IT BY INSERTING THE NULL CHARACTER.
		if (wcslen(startPositionPointer) > CHARACTERS_PER_ROW_RIGHT_COLUMN) {

			//
			// TEMPORARILY STORE THE CHARACTER FROM THE STRING THAT WE WILL BE
			// REPLACING WITH NULL.
			temp = startPositionPointer[CHARACTERS_PER_ROW_RIGHT_COLUMN];

			//
			// REPLACE THE CHARACTER WITH NULL IN THE STRING.
			startPositionPointer[CHARACTERS_PER_ROW_RIGHT_COLUMN] = (wchar_t) 0;

			//
			// PRINT THE CURRENT PORTION OF THE STRING, FOLLOWED BY THE "|" AT THE
			// END OF THE LINE, FOLLOWED BY THE NEW LINE CHARACTER (\n), FOLLOWED BY
			// AN *EMPTY* LEFT COLUMN.
			wprintf(L"%ls%ls\n%ls", startPositionPointer, L"|", L"|        |");

			//
			// RESTORE THE CHARACTER THAT WAS TEMPORARILY REPLACED BY THE NULL
			// CHARACTER IN THE STRING.
			startPositionPointer[CHARACTERS_PER_ROW_RIGHT_COLUMN] = temp;

			//
			// SET THE START POSITION IN THE STRING FOR THE NEXT ITEREATION TO
			// THE POSITION WHERE WE LEFT OFF (THAT IS, THE POSITION OF THE
			// CHARACTER THAT WE TEMPORARILY REPLACED WITH NULL).
			startPositionPointer = startPositionPointer
			                     + CHARACTERS_PER_ROW_RIGHT_COLUMN;

		}

		//
		// IF THE REST OF THE STRING CAN FIT IN ONE ROW, THEN WE DON'T HAVE TO
		// SPLIT IT.
		else {

			//
			// PRINT THE STRING (PADDED WITH 0 OR MORE SPACES ON THE RIGHT TO FILL
			// UP THE CURRENT ROW), THEN PRINT THE "|" AT THE END OF THE CURRENT ROW,
			// THEN PRINT A NEW LINE CHARACTER (\n).
			wprintf(L"%-*ls%ls\n",
			        CHARACTERS_PER_ROW_RIGHT_COLUMN, startPositionPointer, L"|");

			//
			// SET THE START POSITION IN THE STRING TO THE END OF THE STRING.
			startPositionPointer = startPositionPointer
			                     + wcslen(startPositionPointer);

		}
	}
}


void printClusterSequence(uint32_t* clusters, uint32_t numClusters,
                          uint32_t  clustersPerRow, uint32_t clusterNumberLength) {

	//
	// PRINT THE LEFT COLUMN TO THE CONSOLE.
	wprintf(L"%ls", L"|CLUSTERS|");

	//
	// FIRST, LET'S HANDLE EMPTY FILES (FILES WITHOUT A CLUSTER SEQUENCE).
	if (numClusters == 0){
		wprintf(L"%ls%*ls\n", L"(EMPTY FILE)", CHARACTERS_PER_ROW_RIGHT_COLUMN - 11, L"|");
		return;
	}

	//
	// PRINTS ALL THE CLUSTERS IN THE SEQUENCE, ONE AT A TIME.
	uint32_t clusterIndex = 0;
	uint32_t spaceAfterLastCluster = 0;
	while (clusterIndex < numClusters) {

		//
		// PRINT THE CURRENT CLUSTER NUMBER IN THE SEQUENCE.
		wprintf(L"%#0*x", clusterNumberLength, clusters[clusterIndex]);

		//
		// IF THERE IS SPACE IN THIS ROW FOR ANOTHER CLUSTER...
		if (clusterIndex == 0 || (clusterIndex != 0 && (((clusterIndex+1) % clustersPerRow) != 0))) {

			//
			// IF THERE ARE STILL ONE OR MORE CLUSTERS TO PRINT, THEN
			// PRINT A SPACE TO SEPARATE THE CLUSTER WE JUST PRINTED
			// FROM THE NEXT CLUSTER TO BE PRINTED.
			if (clusterIndex < numClusters - 1)
				wprintf(L" ");

			//
			// IF THERE ARE *NOT* ANY MORE CLUSTERS LEFT TO PRINT, THEN PRINT THE
			// "|" AT THE END OF THE CURRENT ROW (PADDED WITH AS MANY SPACES ON
			//  THE LEFT AS NECESSARY), THEN A NEW LINE CHARACTER (\n).
			else {

				//
				// CALCULATING THE NUMBER OF CHARACTERS LEFT IN THIS ROW AFTER THE LAST CLUSTER.
				spaceAfterLastCluster =
										// THE TOTAL NUMBER OF CHARACTERS IN THE RIGHT COLUMN.
										CHARACTERS_PER_ROW_RIGHT_COLUMN
										// MINUS THE SUM OF THE WIDTHS OF THE CLUSTER NUMBERS PRINTED.
										- ((numClusters % clustersPerRow) * clusterNumberLength)
										// THE SUM OF THE SPACES BETWEEN CLUSTER NUMBERS.
										- ((numClusters % clustersPerRow) - 1)
										// PLUS 1 FOR THE "|" AT THE END OF THE ROW.
										+ 1;

				//
				// PRINTING THE END OF THE ROW.
				wprintf(L"%*ls\n", spaceAfterLastCluster, L"|");

			}

		}

		// 
		// IF THERE IS *NOT* SPACE FOR ANOTHER CLUSTER IN THIS ROW, THEN
		// WE START A NEW ROW.
		else {

			//
			// IF THERE ARE STILL ONE OR MORE CLUSTERS TO PRINT, THEN PRINT THE
			// "|" AT THE END OF THE CURRENT ROW (PADDED WITH SPACES ON THE LEFT,
			// IF NECESSARY), THEN A NEW LINE CHARACTER (\n), THEN AN *EMPTY*
			// LEFT COLUMN.
			if (clusterIndex < numClusters - 1) {

				//
				// CALCULATING THE NUMBER OF CHARACTERS LEFT IN THIS ROW AFTER THE LAST CLUSTER.
				spaceAfterLastCluster =
										// THE TOTAL NUMBER OF CHARACTERS IN THE RIGHT COLUMN.
										CHARACTERS_PER_ROW_RIGHT_COLUMN
										// MINUS THE SUM OF THE WIDTHS OF THE CLUSTER NUMBERS PRINTED.
										- (clustersPerRow * clusterNumberLength)
										// THE SUM OF THE SPACES BETWEEN CLUSTER NUMBERS.
										- (clustersPerRow - 1)
										// PLUS 1 FOR THE "|" AT THE END OF THE ROW.
										+ 1;

				//
				// PRINTING THE END OF THE ROW, FOLLOWED BY AN EMPTY LEFT COLUMN.
				wprintf(L"%*ls\n%ls", spaceAfterLastCluster, L"|", L"|        |");

			}

			//
			// IF THERE ARE *NOT ANY CLUSTERS LEFT TO PRINT, THEN PRINT THE
			// "|" AT THE END OF THE CURRENT ROW (PADDED WITH SPACES ON THE LEFT,
			// IF NECESSARY), THEN A NEW LINE CHARACTER (\n).
			else {

				//
				// CALCULATING THE NUMBER OF CHARACTERS LEFT IN THE ROW AFTER THE LAST CLUSTER.
				spaceAfterLastCluster =
										// THE TOTAL NUMBER OF CHARACTERS IN THE RIGHT COLUMN.
										CHARACTERS_PER_ROW_RIGHT_COLUMN
										// MINUS THE SUM OF THE WIDTHS OF THE CLUSTER NUMBERS PRINTED.
										- (clustersPerRow * clusterNumberLength)
										// THE SUM OF THE SPACES BETWEEN CLUSTER NUMBERS.
										- (clustersPerRow - 1)
										// PLUS 1 FOR THE "|" AT THE END OF THE ROW.
										+ 1;

				//
				// PRINTING THE END OF THE ROW.
				wprintf(L"%*ls\n", spaceAfterLastCluster, L"|");

			}

		}

		//
		// INCREMENT THE CLUSTER INDEX.
		clusterIndex++;

	}

}

