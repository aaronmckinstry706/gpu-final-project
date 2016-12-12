#include <cstdio>
#include <vector>

#include "encode_library.h"

// ---------------------------- MAIN FUNCTIONS ----------------------------------

size_t alphabetSize() {
	static size_t alphaSize = 1 << 8*sizeof(word_t);
	return alphaSize;
}

std::vector<long long> getCharacterFrequencies(FILE *filePointer) {
	std::vector<long long> frequencies(alphabetSize(), 0);
	
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

void printCharacterFrequencies(const std::vector<long long>& frequencies) {
	for (size_t i = 0; i < frequencies.size(); ++i)
		if (frequencies[i] > 0)
			printf("%d: %lld\n", i, frequencies[i]);
	printf("\n");
}


