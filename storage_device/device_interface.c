/******************************************************************************
 * This file contains functions and data structures that operate on the
 *                               STORAGE DEVICE
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
// (NOTHING)

// LAYER 3: STORAGE_DEVICE
#include "device_interface.h"

// ERROR HANDLING
#include "error.h"

// STANDARD C LIBRARY
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <wchar.h>





uint8_t* readSectors(uint8_t*  buffer,
                     uint32_t* sectorLocations,
                     uint32_t  numLocations,
					 uint32_t  bytesPerSector,
					 uint32_t  sectorsPerLocation,
					 FILE*     storageDevice) {

	//
	// READ IN THE RAW DATA, ONE SECTOR AT A TIME.
	uint32_t counter = 0;
	uint32_t byteOffset = 0;
	uint32_t result = 0;
	while (counter < numLocations) {

		//
		// CALCULATE THE STARTING BYTE ADDRESS.
		byteOffset = bytesPerSector * sectorLocations[counter];
		
		//
		// SET THE STARTING POSITION IN THE STORAGE DEVICE.
		fseek(storageDevice, byteOffset, SEEK_SET);
		
		//
		// READ IN THE CURRENT SECTOR OR GROUP OF CONTIGUOUS SECTORS.
		result = fread(buffer + (bytesPerSector * sectorsPerLocation * counter),
		               bytesPerSector,
					   sectorsPerLocation,
					   storageDevice);

		//
		// VERIFY THE SECTORS WERE READ PROPERLY.
		if (result != sectorsPerLocation)
			handleError(L"readSectors",
			            L"Unable to read in requested sectors");

		//
		// INCREMENT THE COUNTER.
		counter++;
	}
	
	return buffer;
	
}




FILE* openStorageDevice(char* deviceFileName) {

	//
	// PARAMETER CHECK.
	if (deviceFileName == NULL)
		handleError(L"openDevice",
		            L"NULL 'deviceFileName' parameter");

	//
	// OPEN THE DEVICE.
	FILE* storageDevice = fopen(deviceFileName, "r");

	//
	// VERIFY THE DEVICE WAS OPENED.
	if (storageDevice == NULL)
		handleError(L"openDevice",
		            L"The storage device could not be opened");

	//
	// RETURN THE HANDLE TO THE DEVICE.
	return storageDevice;

}




void closeStorageDevice(FILE* storageDevice) {

	fclose(storageDevice);

}

