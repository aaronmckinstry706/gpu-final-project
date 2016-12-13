#define _CRT_SECURE_NO_WARNINGS

#include <cstdio>
#include <vector>

#include "encode_library.h"

void printCharacterFrequencies(const std::map<word_t, frequency_t>& frequencies);
void printCodeLengths(const std::map<word_t, size_t>& codeLengths);

int main(int argc, char** argv) {
	
	if (argc != 2) {
		printf("Usage: encode <filename>\n");
		return 0;
	}
	
	FILE *filePointer = fopen((const char*) argv[1], "r");
	
	printf("Counting character frequencies...\n");
	std::map<word_t, frequency_t> frequencies = getCharacterFrequencies(filePointer);
	printf("Finished counting.\n");
	//printCharacterFrequencies(frequencies);

	fclose(filePointer);

	printf("Calculating code lengths...\n");
	std::map<word_t, size_t> codeLengths = getCodeLengths(frequencies, 8*sizeof(code_t));
	printf("Finished calculating code lengths.\n");
	printCodeLengths(codeLengths);

	printf("\nFinished. Enter 'q' to quit.\n");
	{ char c; fscanf(stdin, "%c", &c); }
	return 0;
	
}

void printCharacterFrequencies(const std::map<word_t, frequency_t>& frequencies) {
	printf("size is %d\n", frequencies.size());
	for (std::map<word_t, frequency_t>::const_iterator i = frequencies.begin(); i != frequencies.end(); i++)
		if (i->second > 0)
			printf("%c: %lld\n", i->first, i->second);
	printf("\n");
}

void printCodeLengths(const std::map<word_t, size_t>& codeLengths) {
	for (std::map<word_t, size_t>::const_iterator i = codeLengths.begin();
	i != codeLengths.end();
		i++)
	{
		printf("%c: %d\n", i->first, i->second);
	}
}

