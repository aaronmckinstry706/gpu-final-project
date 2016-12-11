#include <stdio.h>

#include "encode_library.h"

// ---------------------------- MAIN FUNCTIONS ----------------------------------

void getCharacterFrequencies(FILE *filePointer, long long *frequencies) {
	
	for (size_t i = 0; i < FREQUENCY_TABLE_SIZE; ++i)
		frequencies[i] = 0;
	
	static word_t fileBuffer[FILE_BUFFER_SIZE];
	
	size_t numRead = fread((void*)fileBuffer, sizeof(word_t), FILE_BUFFER_SIZE, filePointer);
	while (numRead > 0) {
		for (size_t i = 0; i < numRead; i++) {
			frequencies[fileBuffer[i]]++;
		}
		numRead = fread((void*)fileBuffer, sizeof(word_t), FILE_BUFFER_SIZE, filePointer);
	}
	
}

// ---------------------------- DEBUG FUNCTIONS ---------------------------------

void printCharacterFrequencies(long long *frequencies) {
	for (size_t i = 0; i < FREQUENCY_TABLE_SIZE; ++i)
		if (frequencies[i] > 0)
		printf("%d: %lld\n", i, frequencies[i]);
	printf("\n");
}


