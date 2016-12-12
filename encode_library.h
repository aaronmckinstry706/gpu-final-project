#ifndef ENCODE_LIBRARY_H
#define ENCODE_LIBRARY_H

#include <cstdio>
#include <vector>

typedef char word_t;

#define FILE_BUFFER_SIZE 2048 //number of word_t items stored in file buffer when reading
#define CODE_LENGTH_LIMIT 2 //bytes; must be a power of 2

size_t alphabetSize();
std::vector<long long> getCharacterFrequencies(FILE *filePointer);

//debug functions
void printCharacterFrequencies(const std::vector<long long>& frequencies);

#endif
