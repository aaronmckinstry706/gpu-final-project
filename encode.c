#include <stdio.h>

#define FILE_BUFFER_SIZE 2048
#define WORD_SIZE 1
#define CODE_LENGTH_LIMIT 2

void getCharacterFrequencies(FILE *filePointer, long long *frequencies) {
	static unsigned char fileBuffer[FILE_BUFFER_SIZE];
	
	size_t numRead = fread((void*)fileBuffer, 1, FILE_BUFFER_SIZE-1, filePointer);
	fileBuffer[numRead] = '\0';
	
	printf("%s", fileBuffer);
}

int main(int argc, char** argv) {
	
	if (argc != 2) {
		printf("Usage: encode <filename>\n");
		return 0;
	}
	
	FILE *filePointer = fopen((const char*) argv[1], "r");
	
	long long frequencies[(unsigned long long)1 << (8*WORD_SIZE)];
	
	getCharacterFrequencies(filePointer, frequencies);
	
	fclose(filePointer);
	
	return 0;
	
}


