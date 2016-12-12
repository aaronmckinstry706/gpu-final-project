#include <cstdio>
#include <vector>

#include "encode_library.h"

// ---------------------------- MAIN FUNCTIONS ----------------------------------

size_t alphabetSize() {
	static size_t alphaSize = 1 << 8*sizeof(word_t);
	return alphaSize;
}

std::map<word_t, frequency_t> getCharacterFrequencies(FILE *filePointer) {
	std::map<word_t, frequency_t> frequencies;
	
	static word_t fileBuffer[FILE_BUFFER_SIZE];
	
	size_t numRead = fread((void*)fileBuffer, sizeof(word_t), FILE_BUFFER_SIZE, filePointer);
	while (numRead > 0) {
		for (size_t i = 0; i < numRead; i++) {
			frequencies[fileBuffer[i]]++;
		}
		numRead = fread((void*)fileBuffer, sizeof(word_t), FILE_BUFFER_SIZE, filePointer);
	}
	return frequencies;
}

std::map<word_t, size_t> getWordCodeLengths(const std::map<word_t, frequency_t>& frequencies) {
	return std::map<word_t, size_t>();
}

// ---------------------------- DEBUG FUNCTIONS ---------------------------------

void printCharacterFrequencies(const std::map<word_t, frequency_t>& frequencies) {
	printf("size is %d\n", frequencies.size());
	for (std::map<word_t,frequency_t>::const_iterator i = frequencies.begin(); i != frequencies.end(); i++)
		if ((*i).second > 0)
			printf("%d: %lld\n", (*i).first, (*i).second);
	printf("\n");
}


