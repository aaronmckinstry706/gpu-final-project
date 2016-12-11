#ifndef ENCODE_LIBRARY_H
#define ENCODE_LIBRARY_H

#include <stdio.h>

typedef char word_t;

#define FREQUENCY_TABLE_SIZE (1 << 8*sizeof(word_t))
#define FILE_BUFFER_SIZE 2048 //number of word_t items stored in file buffer when reading
#define CODE_LENGTH_LIMIT 2 //bytes; must be a power of 2

void getCharacterFrequencies(FILE *filePointer, long long *frequencies);

//debug functions
void printCharacterFrequencies(long long *frequencies);

#endif
