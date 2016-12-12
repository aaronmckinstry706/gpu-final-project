#include <cstdio>
#include <vector>
#include <map>
#include <algorithm>

#include "encode_library.h"

struct word_with_code_length {
	word_t word;
	size_t codeLength;
	
	word_with_code_length(word_t w, size_t c)
		: word(w), codeLength(c) {}
};

bool operator< (const word_with_code_length& wwcl1, const word_with_code_length& wwcl2) {
	return wwcl1.codeLength < wwcl2.codeLength
		|| (
			wwcl1.codeLength == wwcl2.codeLength
			&& wwcl1.word < wwcl2.word
		);
}

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

std::map<word_t, code_t> getWordCodeTable(const std::map<word_t, size_t>& wordCodeLengths) {
	std::vector<word_with_code_length> wordsWithCodeLengths(wordCodeLengths.size());
	for (wordCodeLengths::const_iterator i = wordCodeLengths.begin(); i != wordCodeLenghts.end(); i++)
		wordsWithCodeLengths.push_back(word_with_code_length(i->first, i->second));
	std::sort(wordsWithCodeLengths.begin(), wordsWithCodeLengths.end());
	
	std::map<word_t, code_t> wordCodes;
	for (size_t i = 0; i < wordsWithCodeLengths.size(); ++i) {
		code_t code = 0;
		for (size_t depth = 0; depth < wordsWithCodeLengths[i].codeLength; ++depth) {
			
		}
	}
}

// ---------------------------- DEBUG FUNCTIONS ---------------------------------

void printCharacterFrequencies(const std::map<word_t, frequency_t>& frequencies) {
	printf("size is %d\n", frequencies.size());
	for (std::map<word_t,frequency_t>::const_iterator i = frequencies.begin(); i != frequencies.end(); i++)
		if (i->second > 0)
			printf("%d: %lld\n", i->first, i->second);
	printf("\n");
}


