#include <stdio.h>

#include "encode_library.h"

int main(int argc, char** argv) {
	
	if (argc != 2) {
		printf("Usage: encode <filename>\n");
		return 0;
	}
	
	FILE *filePointer = fopen((const char*) argv[1], "r");
	
	long long frequencies[FREQUENCY_TABLE_SIZE];
	for (size_t i = 0; i < FREQUENCY_TABLE_SIZE; ++i) {
		frequencies[i] = 0;
	}
	
	printf("Counting character frequencies...\n");
	getCharacterFrequencies(filePointer, frequencies);
	printf("Finished counting.\n");
	printCharacterFrequencies(frequencies);
	
	fclose(filePointer);
	
	printf("\nFinished. Enter 'q' to quit.\n");
	{ char c; fscanf(stdin, "%c", &c); }
	return 0;
	
}



