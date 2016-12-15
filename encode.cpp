#define _CRT_SECURE_NO_WARNINGS

#include <cstdio>
#include <vector>

#include "encode_library.h"

void printCharacterFrequencies(const std::map<word_t, frequency_t> &frequencies);
void printCodeLengths(const std::map<word_t, size_t> &codeLengths);
void printCodes(const std::map<word_t, code_t> &codes);
void printDecodingTable(const std::vector<unsigned short> &decodingTable);
void printDestinationBits(const char*);
template <typename T> void printBits(T code);

int main(int argc, char** argv) {
	
	if (argc != 3) {
		printf("Usage: encode <source> <destination>\n");
		return 0;
	}
	
	FILE *sourceFile = fopen((const char*) argv[1], "r");
	
	printf("Counting character frequencies...\n");
	std::map<word_t, frequency_t> frequencies = getCharacterFrequencies(sourceFile);
	printf("Finished counting.\n");
	printCharacterFrequencies(frequencies);

	fclose(sourceFile);

	printf("Calculating code lengths...\n");
	std::map<word_t, size_t> codeLengths = getCodeLengths(frequencies, MAX_CODE_LENGTH);
	printf("Finished calculating code lengths.\n");
	printCodeLengths(codeLengths);

	printf("Calculating codes...\n");
	std::map<word_t, code_t> codes = getCodes(codeLengths);
	printf("Finished calculating codes.\n");
	printCodes(codes);

	printf("Creating decoding table...\n");
	std::vector<unsigned short> decodingTable = getDecodingTable(codes, codeLengths);
	printf("Finished creating decoding table.\n");
	//printDecodingTable(decodingTable);

	printf("Encoding file %s...\n", argv[1]);
	sourceFile = fopen(argv[1], "r");
	FILE *destinationFile = fopen(argv[2], "w");
	encodeFile(sourceFile, destinationFile, decodingTable, codes, codeLengths);
	fclose(destinationFile);
	fclose(sourceFile);
	printDestinationBits(argv[2]);
	printf("Finished encoding file.\n");

	printf("\nFinished. Enter 'q' to quit.\n");
	{ char c; fscanf(stdin, "%c", &c); }
	return 0;
	
}

void printCharacterFrequencies(const std::map<word_t, frequency_t> &frequencies) {
	printf("size is %d\n", frequencies.size());
	for (std::map<word_t, frequency_t>::const_iterator i = frequencies.begin(); i != frequencies.end(); i++)
		if (i->second > 0)
			printf("%c: %lld\n", i->first, i->second);
	printf("\n");
}

void printCodeLengths(const std::map<word_t, size_t> &codeLengths) {
	for (std::map<word_t, size_t>::const_iterator i = codeLengths.begin();
	i != codeLengths.end();
		i++)
	{
		printf("%c: %d\n", i->first, i->second);
	}
}

void printCodes(const std::map<word_t, code_t> &codes) {
	for (std::map<word_t, code_t>::const_iterator it = codes.begin(); it != codes.end(); it++) {
		printf("%c: ", it->first);
		printBits(it->second);
		printf("\n");
	}
}

void printDecodingTable(const std::vector<unsigned short> &decodingTable) {
	for (size_t i = 0; i < decodingTable.size(); ++i) {
		printBits((code_t)i);
		printf(": %c ", (word_t)(decodingTable[i] & (unsigned short)0x00FF));
		printf("%u\n", (unsigned)(decodingTable[i] & (unsigned short)0xFF00) >> 8);
	}
}

void printDestinationBits(const char* filename) {
	FILE *filePointer = fopen(filename, "r");
	unsigned char fileBuffer[FILE_BUFFER_SIZE];
	size_t numRead = fread((void*)fileBuffer, sizeof(word_t), FILE_BUFFER_SIZE, filePointer);
	while (numRead > 0) {
		for (size_t i = 0; i < numRead; i++) {
			printBits(fileBuffer[i]);
		}
		numRead = fread((void*)fileBuffer, sizeof(word_t), FILE_BUFFER_SIZE, filePointer);
	}
	printf("\n");
}

template <typename T> void printBits(T code) {
	for (int i = 0; i < 8 * sizeof(T); ++i) {
		printf("%d", (code & (1 << i)) != (0 << 8 * sizeof(T)) ? 1 : 0);
	}
}

