#include <cstdio>
#include <vector>

#include "encode_library.h"

int main(int argc, char** argv) {
	
	if (argc != 2) {
		printf("Usage: encode <filename>\n");
		return 0;
	}
	
	FILE *filePointer = fopen((const char*) argv[1], "r");
	
	printf("Counting character frequencies...\n");
	std::vector<long long> frequencies = getCharacterFrequencies(filePointer);
	printf("Finished counting.\n");
	printCharacterFrequencies(frequencies);
	
	printf("Running package-merge...\n");
	//run package merge
	printf("Finished package-merge.\n");
	
	fclose(filePointer);
	
	printf("\nFinished. Enter 'q' to quit.\n");
	{ char c; fscanf(stdin, "%c", &c); }
	return 0;
	
}


