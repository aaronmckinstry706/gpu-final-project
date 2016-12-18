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
	
	if (argc != 4 && argc != 5) {
		printf("Usage: encode <source> <destination> <subBlockSize> <debug(optional)>\n");
		return 0;
	}
	
	FILE *sourceFile = fopen((const char*) argv[1], "r");
	
	if (argc == 5)
		printf("\nCounting character frequencies...\n");
	std::map<word_t, frequency_t> frequencies = getCharacterFrequencies(sourceFile);
	if (argc == 5) {
		printf("Finished counting.\n");
		printCharacterFrequencies(frequencies);
	}
	
	fclose(sourceFile);

	if (argc == 5)
		printf("\nCalculating code lengths...\n");
	std::map<word_t, size_t> codeLengths = getCodeLengths(frequencies, MAX_CODE_LENGTH+1);
	if (argc == 5) {
		printf("Finished calculating code lengths.\n");
		printCodeLengths(codeLengths);
	}

	if (argc == 5)
		printf("\nCalculating codes...\n");
	std::map<word_t, code_t> codes = getCodes(codeLengths);
	if (argc == 5) {
		printf("Finished calculating codes.\n");
		printCodes(codes);
	}
	
	if (argc == 5)
		printf("\nCreating decoding table...\n");
	std::vector<unsigned short> decodingTable = getDecodingTable(codes, codeLengths);
	if (argc == 5) {
		printf("Finished creating decoding table.\n");
		//printDecodingTable(decodingTable);
	}
	
	//get subblock size from command line
	size_t subBlockSize;
	sscanf(argv[3], "%llu", (unsigned long long *) &subBlockSize);
	if (subBlockSize == 0)
		subBlockSize = 1;
	if (argc == 5)
		printf("\nSub-block size: %llu\n", (unsigned long long)subBlockSize);

	//get sourceFileSize
	sourceFile = fopen(argv[1], "r");
	fseek(sourceFile, 0, SEEK_END);
	unsigned long long sourceFileSize = ftell(sourceFile);
	fseek(sourceFile, 0, SEEK_SET);
	if (argc == 5)
		printf("Source file size: %llu\n", (unsigned long long)sourceFileSize);

	//calculate number of bits in encoded data
	std::map<word_t, size_t>::iterator lengthsIter = codeLengths.begin();
	std::map<word_t, frequency_t>::iterator frequencyIter = frequencies.begin();
	unsigned long long encodedDataBitLength = 0;
	for (; lengthsIter != codeLengths.end(); lengthsIter++, frequencyIter++) {
		encodedDataBitLength += (unsigned long long)(lengthsIter->second)*(frequencyIter->second);
	}
	if (argc == 5)
		printf("Encoded data size: %llu\n", (unsigned long long)encodedDataBitLength);

	if (argc == 5)
		printf("\nEncoding file %s...\n", argv[1]);
	FILE *destinationFile = fopen(argv[2], "w");
	encodeFile(sourceFile, destinationFile, decodingTable, codes, codeLengths, subBlockSize, sourceFileSize, encodedDataBitLength);
	fclose(destinationFile);
	fclose(sourceFile);
	if (argc == 5) {
		printDestinationBits(argv[2]);
		printf("Finished encoding file.\n");
	}

	if (argc == 5) {
		printf("\nFinished. Enter 'q' to quit.\n");
		{ char c; fscanf(stdin, "%c", &c); }
	}
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

	size_t codeLengthLimit, subBlockSize, originalFileSize, sizeOfEncodedData;
	fread((void*)&codeLengthLimit, sizeof(size_t), 1, filePointer);
	fread((void*)&subBlockSize, sizeof(size_t), 1, filePointer);
	fread((void*)&originalFileSize, sizeof(size_t), 1, filePointer);
	fread((void*)&sizeOfEncodedData, sizeof(size_t), 1, filePointer);

	printf("Code Length Limit: %llu\n", (unsigned long long)codeLengthLimit);
	printf("Sub-Block Size: %llu\n", (unsigned long long)subBlockSize);
	printf("Original File Size: %llu\n", (unsigned long long)originalFileSize);
	printf("Size of Encoded Data: %llu\n", (unsigned long long)sizeOfEncodedData);

	size_t numSubBlocks = originalFileSize / subBlockSize + (originalFileSize % subBlockSize > 0);
	std::vector<size_t> subBlockOffsets(numSubBlocks, 0);
	fread((void*)&subBlockOffsets[0], sizeof(size_t), subBlockOffsets.size(), filePointer);
	for (size_t i = 0; i < subBlockOffsets.size(); ++i) {
		printf("block %llu: %llu\n", (unsigned long long)i, (unsigned long long)subBlockOffsets[i]);
	}

	size_t decodeTableSize = (unsigned)1 << codeLengthLimit;
	std::vector<unsigned short> decodeTable(decodeTableSize, 0);
	fread((void*)&decodeTable[0], sizeof(unsigned short), decodeTable.size(), filePointer);
	//printDecodingTable(decodeTable);

	unsigned char fileBuffer[FILE_BUFFER_SIZE];
	size_t numRead = fread((void*)fileBuffer, sizeof(word_t), FILE_BUFFER_SIZE, filePointer);
	while (numRead > 0) {
		for (size_t i = 0; i < numRead; i++) {
			printBits(fileBuffer[i]);
		}
		numRead = fread((void*)fileBuffer, sizeof(word_t), FILE_BUFFER_SIZE, filePointer);
	}
	printf("\n");
	
	fclose(filePointer);
}

template <typename T> void printBits(T code) {
	for (int i = 0; i < 8 * sizeof(T); ++i) {
		printf("%d", (code & (1 << i)) != (0 << 8 * sizeof(T)) ? 1 : 0);
	}
}

